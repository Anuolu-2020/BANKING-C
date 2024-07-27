#include "gen_account_number.h"
#include "sqlite3.h"
#include "string.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>

#define ACCOUNT_NUMBER_LENGTH 10

// Generate account numbers
void generate_account_number(sqlite3 *db, char *account_number) {
  time_t t;
  struct tm *tm_info;
  char timestamp[7];
  char random_part[5];
  int is_unique = 0;

  while (!is_unique) {
    // Get current time
    time(&t);
    tm_info = localtime(&t);

    // Format timestamp (YYMMDD)
    strftime(timestamp, sizeof(timestamp), "%y%m%d", tm_info);

    // Generate random part (0000-9999)
    snprintf(random_part, sizeof(random_part), "%04d", rand() % 10000);

    // Combine timestamp and random part
    snprintf(account_number, ACCOUNT_NUMBER_LENGTH + 1, "%s%s", timestamp,
             random_part);

    // Check if the account number already exists in the database
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM accounts WHERE account_number = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
      fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      return;
    }

    sqlite3_bind_text(stmt, 1, account_number, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      int count = sqlite3_column_int(stmt, 0);
      is_unique = (count == 0);
    }

    sqlite3_finalize(stmt);
  }
}
