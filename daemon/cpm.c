/*
 * cpm.c
 * Cheap Power Meter
 * Wait for interrupt from Raspberry Pi GPIO board
 * and insert row into sqlite database.
 *
 * Used with light-dependent resistor connected to the Pi,
 * to measure power usage from fuse box. One blink equals 1 watt hour
 *
 * Read more: http://www.hyggeit.dk/2013/04/super-cheap-web-enabled-power.html
 *
 *
 * Copyright (C) 2013 Lasse Mammen <lasse.mammen@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#ifndef WITHOUT_WIRINGPI
#include <wiringPi.h>
#endif
#include <time.h>
#include <pthread.h>

int verbose_flag;
#include "cpm.h"

static short int pin = 1;
static short int sensitivity = 20;
static short int keepRunning = 1;
static volatile int interruptcounter = 0;
static int wattcounter = 0;

void intHandler() {
   keepRunning = 0;
}

void interrupt_handler(void) {
   ++interruptcounter;
   /* When diode lights up to 2000 interrupt are fired, try to throttle a little */
   usleep(100);
}

int main(int argc, char **argv) {
   int c;
   time_t timer;
   char buffer[25];
   char *database = NULL;
   struct tm* tm_info;
   pthread_t webapi;
   api_thread auth;

   auth.apikey = NULL;
   auth.systemid = NULL;
   
   while (1) {
      static struct option long_options[] ={
         {"verbose", no_argument, &verbose_flag, 1},
         {"help", no_argument, 0, 'h'},
         {"database", required_argument, 0, 'd'},
         {"pin", required_argument, 0, 'p'},
         {"sensitivity", required_argument, 0, 's'},
	 {"apikey", required_argument, 0, 'k'},
	 {"systemid", required_argument, 0, 'i'},
         {0, 0, 0, 0}
      };

      int option_index = 0;

      c = getopt_long(argc, argv, "vhd:p:s:k:i:",
              long_options, &option_index);

      if (c == -1)
         break;

      switch (c) {
         case 0:
            if (long_options[option_index].flag != 0)
               break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
               printf(" with arg %s", optarg);
            printf("\n");
            break;
         case 'h':
            printf("cpm - CheapPowerMeter (2013 Dec 11)\n\n");
            printf("Arguments:\n");
            printf("   -d <file>\t\t\tSqlite3 database file *required\n");
            printf("   --database\n");
            printf("   -p <number>\t\t\tOverride default pin (18) on Raspberry Pi's GPIO\n");
            printf("   --pin\n");
            printf("   -s <number>\t\t\tHow many interrupts for one valid reading or Wh (default: 20)\n");
            printf("   --sensitivity\n");
            printf("   -k <apikey>\t\t\tpvoutput.org APIKey\n");
            printf("   --apikey\n");
            printf("   -i <systemid>\t\t\tpvoutput.org SystemId\n");
            printf("   --systemid\n");
            printf("   --verbose\t\t\t Increase verbosity\n");
            printf("   -h  --help\t\t\tPrint Help (this message) and exit\n\n");

            exit(0);
            break;
         case 'd':
            database = allocate_str(optarg);
            break;
         case 'p':
            if (sscanf (optarg, "%hd", &pin) != 1) {
               fprintf(stderr, "Invalid value for argument for --pin / -p \n");
               exit(1);
            }
            break;
         case 's':
            if (sscanf(optarg, "%hd", &sensitivity) != 1) {
               fprintf(stderr, "Invalid value for argument for --sensitivity / -s \n");
               exit(1);
            }
            break;
         case 'k':
            auth.apikey = allocate_str(optarg);
            break;
         case 'i':
            auth.systemid = allocate_str(optarg);
            break;

         default:
            exit(1);
      }
   }

   if (database == NULL) {
      fprintf(stderr, "cpm: --database/-d is required\n");
      exit(1);
   }
   
   if ((auth.apikey != NULL  && auth.systemid == NULL) ||
       (auth.apikey == NULL  && auth.systemid != NULL)) {
      fprintf(stderr, "cpm: both --apikey / -a and --systemid / -i is required when either is specified \n");
      exit(1);
   }

   signal(SIGINT, intHandler);

   initialize_db(database);

   if (auth.apikey != NULL && auth.systemid != NULL) {
      auth.database_file = database;
      if (pthread_create(&webapi, NULL, &thread_webapi, &auth)) {
         fprintf(stderr, "cpm: webapi thread could not be created (exit)\n");
         exit(1);
      }
   }
   
#ifndef WITHOUT_WIRINGPI
   /* set priority to maximum and die if fail */
   if (piHiPri(99) != 0) {
      fprintf(stderr, "cpm: Setting priorty of process failed\n");
      exit(1);
   }

   if (wiringPiSetup() == -1) {
      fprintf(stderr, "cpm: WiringPI setup failed. You use sudo to run this program\n");
      exit(1);
   }
   pinMode(pin, INPUT);
   pullUpDnControl(pin, PUD_DOWN);

   if (wiringPiISR(pin, INT_EDGE_RISING, &interrupt_handler) < 0) {
      fprintf(stderr, "cpm: Interrupt failed.\n");
      exit(1);
   }
#endif
   
   if (verbose_flag) {
      fprintf(stderr, "cpm: Starting main loop...\n");
   }

   while (keepRunning) {
      if (interruptcounter / sensitivity) {
         if (verbose_flag) {
            time(&timer);
            tm_info = localtime(&timer);

            strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
            fprintf(stderr, "%s Detected light level %d\n", buffer, interruptcounter);
         }
         ++wattcounter;
         /* Wait before resetting counter so all remaining "single blink" 
            interrupt are cleared */
         usleep(500000);
         interruptcounter = 0;
      } else {
         usleep(500000);
      }
      
      /* Only insert every 5 minutes to save system resources when getting data */
      if (wattcounter > 0 && (int)time(NULL) % 300 == 0) {
         insert_data(wattcounter);
         wattcounter = 0;
      }
   }
   
   if (wattcounter > 0) {
      insert_data(wattcounter);
   }
   
   close_db();
   return 0;
}