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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <wiringPi.h>
#include <time.h>

static short int pin = 1;
static short int sensitivity = 20;
static short int keepRunning = 1;
static short int verbose_flag;
static volatile int lightcounter = 0;

sqlite3 *initialize_db(char *dbfile) {
   sqlite3 *pDb;
   int rc;
   char *zErrMsg = 0;
   char *sql;

   rc = sqlite3_open_v2(dbfile, &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

   if (rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(pDb));
      sqlite3_close(pDb);
      exit(1);
   }

   sql = "CREATE TABLE IF NOT EXISTS watthours ( \
			id INTEGER PRIMARY KEY AUTOINCREMENT, \
			datetime DATETIME DEFAULT CURRENT_TIMESTAMP, \
			lightvalue INTEGER NOT NULL)";

   rc = sqlite3_exec(pDb, sql, NULL, 0, &zErrMsg);

   if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      sqlite3_close(pDb);
      exit(1);
   }

   return pDb;
}

sqlite3_stmt *prepare_statement(sqlite3 *pDb) {
   sqlite3_stmt *pStmt;
   const char *sql = "INSERT INTO watthours (lightvalue) VALUES (:usage)";

   sqlite3_prepare_v2(pDb, sql, strlen(sql), &pStmt, NULL);

   return pStmt;
}

void insert_data(int reading, sqlite3_stmt *pStmt) {
   const int index = sqlite3_bind_parameter_index(pStmt, ":usage");
   sqlite3_bind_int(pStmt, index, reading);

   if (sqlite3_step(pStmt) != SQLITE_DONE) {
      fprintf(stderr, "SQL error!\n");
   }
   sqlite3_reset(pStmt);
}

void intHandler() {
   keepRunning = 0;
}

void interrupt_handler(void) {
   ++lightcounter;
   /* When diode lights up to 2000 interrupt are fired, try to throttle a little */
   usleep(100);
}

int main(int argc, char **argv) {
   int c;
   sqlite3 *pDb;
   sqlite3_stmt *pStmt;
   time_t timer;
   char buffer[25];
   char *database;
   struct tm* tm_info;

   while (1) {
      static struct option long_options[] ={
         {"verbose", no_argument, &verbose_flag, 1},
         {"help", no_argument, 0, 'h'},
         {"database", required_argument, 0, 'd'},
         {"pin", required_argument, 0, 'p'},
         {"sensitivity", required_argument, 0, 's'},
         {0, 0, 0, 0}
      };

      int option_index = 0;

      c = getopt_long(argc, argv, "vhd:p:s:",
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
            printf("option -h with value ");
            break;
         case 'd':
            if ((database = malloc(strlen(optarg)+1)) == NULL) {
               fprintf(stderr, "Invalid value for argument for --database / -d \n");
               exit(1);
            }
            strcpy(database, optarg);
            break;
         case 'p':
            if (sscanf (optarg, "%i", &pin) != 1) {
               fprintf(stderr, "Invalid value for argument for --pin / -p \n");
               exit(1);
            }
            break;
         case 's':
            if (sscanf (optarg, "%i", &sensitivity) != 1) {
               fprintf(stderr, "Invalid value for argument for --sensitivity / -s \n");
               exit(1);
            }
            break;

         default:
            exit(1);
      }
   }

   if (database == NULL) {
      fprintf(stderr, "cpm: --database/-d is required\n");
      exit(1);
   }

   signal(SIGINT, intHandler);

   pDb = initialize_db(database);

   /* set priority to maximum and die if fail */
   if (piHiPri(99) != 0) {
      fprintf(stderr, "Setting priorty of process failed\n");
      exit(1);
   }

   if (wiringPiSetup() == -1) {
      fprintf(stderr, "WiringPI setup failed. You use sudo to run this program\n");
      exit(1);
   }
   pinMode(pin, INPUT);
   pullUpDnControl(pin, PUD_DOWN);

   if (wiringPiISR(pin, INT_EDGE_RISING, &interrupt_handler) < 0) {
      fprintf(stderr, "Interrupt failed.\n");
      exit(1);
   }

   pStmt = prepare_statement(pDb);

   if (verbose_flag) {
        fprintf(stderr, "Starting main loop...\n");
   }

   while (keepRunning) {
      if (verbose_flag) {
         fprintf(stderr, ".");
      }
      if (lightcounter / sensitivity) {
         if (verbose_flag) {
            time(&timer);
            tm_info = localtime(&timer);

            strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);
            fprintf(stderr, "\n%s Detected light level %d\n", buffer, lightcounter);
         }
         insert_data(lightcounter, pStmt);

         /* Wait before resetting counter so all remaining "single blink" 
            interrupt are cleared */
         delay(500);
         lightcounter = 0;
         
      } else {
         delay(500);
      }
   }

   sqlite3_finalize(pStmt);
   sqlite3_close(pDb);
   return 0;
}