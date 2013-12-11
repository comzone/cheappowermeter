/*
 * misc.c
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

#include "cpm.h"

char *allocate_str(char *input) {
   char *output;
   
   if ((output = malloc(strlen(input) + 1)) == NULL) {
      fprintf(stderr, "misc: Invalid value for argument for --database / -d \n");
      exit(1);
   }
   strcpy(output, input);
   
   return output;
}

char *allocate_concat_str(char *str1, char *str2) {
   char *output;

   if ((output = malloc(strlen(str1) + strlen(str2) + 1)) == NULL) {
      fprintf(stderr, "misc: Malloc error, will exit \n");
      exit(1);
   }

   strcat(output, str1);
   strcat(output, str2);
           
   return output;
}