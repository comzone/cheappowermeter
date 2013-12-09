/*
 * cpm.h
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
#include <unistd.h>

extern int verbose_flag;

typedef struct api_thread {
   char *database_file;
   char *apikey;
   char *systemid;
} api_thread;

typedef struct sql_data {
   char *date;
   char *time;
   int watt;
} sql_data;

/* sqlite.c */
void initialize_db(char *dbfile);
void insert_data(int reading);
void close_db();
int get_last_data(int minutes, char *dbfile, sql_data *data);

/* webapi.c */
void *thread_webapi(void *credentials);

/* misc.c */
char *allocate_str(char *input);
char *allocate_concat_str(char *str1, char *str2);