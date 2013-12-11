/*
 * sqlite.c
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
#include <sqlite3.h>

#include "cpm.h"

sqlite3 *pDb;
sqlite3_stmt *pStmt;
char *database_file;

void initialize_db(char *dbfile) {
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

   //prepare_statement();
}

void close_db() {
   sqlite3_close(pDb);
}

void prepare_statement() {
   sqlite3_stmt *pStmt;
   const char *sql = "INSERT INTO watthours (lightvalue) VALUES (:usage)";

   sqlite3_prepare_v2(pDb, sql, strlen(sql), &pStmt, NULL);
}

void insert_data(int reading) {
   const int index = sqlite3_bind_parameter_index(pStmt, ":usage");
   sqlite3_bind_int(pStmt, index, reading);

   if (sqlite3_step(pStmt) != SQLITE_DONE) {
      fprintf(stderr, "sqlite: SQL error!\n");
   }
   sqlite3_reset(pStmt);
}

int get_last_data(int minutes, char *dbfile, sql_data *data) {
   char *sql;
   int rc;
   int index;
   sqlite3 *lDb;
   sqlite3_stmt *lStmt;

   rc = sqlite3_open_v2(dbfile, &lDb, SQLITE_OPEN_READONLY, NULL);

   if (rc) {
      fprintf(stderr, "sqlite: Can't open database: %s\n", sqlite3_errmsg(lDb));
      sqlite3_close(pDb);
      exit(1);
   }

   sql = "select strftime('%Y-%m-%d', datetime('now', 'utc')) as date, \
          strftime('%H:%M', datetime('now', 'utc')) as time, count(*) as watt from watthours \
          where datetime > datetime('now', 'utc', '-5 minutes');";

   rc = sqlite3_prepare_v2(lDb, sql, strlen(sql), &lStmt, NULL);

   //index = sqlite3_bind_parameter_index(lStmt, ":minutes");
   //rc = sqlite3_bind_int(lStmt, index, minutes); // Using parameters ("?") is not

   if (rc != SQLITE_OK) {
      fprintf(stderr, "sqlite: %s\n", sqlite3_errmsg(lDb));
      sqlite3_finalize(lStmt);
      sqlite3_close(lDb);
      return 0;
   }

   rc = sqlite3_step(lStmt);
   if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
      fprintf(stderr, "sqlite: %s\n", sqlite3_errmsg(lDb));
      sqlite3_finalize(lStmt);
      sqlite3_close(lDb);
      return 0;
   }
   if (rc == SQLITE_DONE) {
      sqlite3_finalize(lStmt);
      if (verbose_flag) {
         fprintf(stderr, "sqlite: No data in query\n");
      }
      sqlite3_close(lDb);
      return 0;
   }

   if (sqlite3_column_int(lStmt, 2) == 0) {
      sqlite3_finalize(lStmt);
      if (verbose_flag) {
         fprintf(stderr, "sqlite: Stale data (err: 1)\n");
      }
      sqlite3_close(lDb);
      return 0;
   }

   data->date = allocate_str((char*) sqlite3_column_text(lStmt, 0));
   data->time = allocate_str((char*) sqlite3_column_text(lStmt, 1));
   data->watt = sqlite3_column_int(lStmt, 2);

   sqlite3_finalize(lStmt);
   sqlite3_close(lDb);
   return 1;
}