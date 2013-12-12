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

#include <curl/curl.h>
#include "cpm.h"

#define PV_API_URL "http://pvoutput.org/service/r2/addstatus.jsp"

char *prepare_data_str(sql_data *data) {
   char *string; //20111201,t=10:00,v1=1000";
   char watt[15];
   snprintf(watt, 10, "%d", data->watt);

   string = allocate_concat_str("d=", data->date);
   string = allocate_concat_str(string, ",t=");
   string = allocate_concat_str(string, data->time);
   string = allocate_concat_str(string, ",v1=");
   string = allocate_concat_str(string, watt);

   return string;
}

int api_connect(char *apikey, char *systemid, sql_data *data) {
   CURL *curl;
   CURLcode res;
   struct curl_slist *chunk = NULL;
   char *dataStr;

   curl_global_init(CURL_GLOBAL_ALL);

   /* Read about PV API here: http://pvoutput.org/help.html#api */
   curl = curl_easy_init();

   if (curl) {

      chunk = curl_slist_append(chunk, allocate_concat_str("X-Pvoutput-Apikey: ", apikey));
      chunk = curl_slist_append(chunk, allocate_concat_str("X-Pvoutput-SystemId: ", systemid));

      curl_easy_setopt(curl, CURLOPT_URL, PV_API_URL);

      dataStr = prepare_data_str(data);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataStr);
		
      res = curl_easy_perform(curl);
      /* Check for errors */
      if (res != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      }

      curl_easy_cleanup(curl);
   }
   return 0;
}

void *thread_webapi(void *credentials) {
   api_thread *auth = (api_thread*) credentials;


   if (verbose_flag) {
      fprintf(stderr, "Starting webapi loop with apikey: %s and systemid: %s\n", auth->apikey, auth->systemid);
   }
   for (;;) {
      sql_data data;
      sleep(300);
      if (get_last_data(5, auth->database_file, &data) > 0) {
         if (verbose_flag) {
            fprintf(stderr, "webapi: good data send to api: %s\n", prepare_data_str(&data));
         }
         api_connect(auth->apikey, auth->systemid, &data);

      } else {
         if (verbose_flag) {
            fprintf(stderr, "webapi: ignore stale or bad data\n");
         }
      }
   }

   curl_global_cleanup();
}