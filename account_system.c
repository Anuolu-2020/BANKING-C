#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "account_system.h"
#include "sqlite3.h"
#include "utils_functions.h"

// Display account management menu
void display_account_menu() {
  printf("   1 Create New Account\n");
  printf("   2 View Account Details\n");
  printf("   3 Update Customer Information\n");
  printf("   4 Delete Account\n");
}

// Create accounts table
int create_accounts_table(sqlite3 *db) {
  char *sql;

  sql = "CREATE TABLE IF NOT EXISTS accounts ("
        "account_number TEXT PRIMARY KEY, "
        "customer_id TEXT, "
        "account_type TEXT CHECK(account_type IN ('savings', 'current')), "
        "balance REAL, "
        "FOREIGN KEY(customer_id) REFERENCES customers(customer_id));";

  // Execute sql
  int rc = execute_sql(db, sql);

  if (rc != SQLITE_OK) {
    return rc;
  }

  return SQLITE_OK;
}

// Insert new accounts into table
int insert_account(sqlite3 *db, struct Account *account) {
  sqlite3_stmt *stmt;
  int rc;
  // Validate account_type
  if (strcmp(account->account_type, "savings") != 0 &&
      strcmp(account->account_type, "current") != 0) {
    printf("Invalid account type. Must be 'savings' or 'current'.\n");
    return SQLITE_ERROR;
  }

  // If user exists, execute command
  const char *sql =
      "INSERT INTO accounts (account_number, customer_id, account_type, "
      "balance) VALUES (?, ?, ?, ?);";

  // Prepare the SQL statement
  rc = sqlite3_prepare_v3(db, sql, -1, 0, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  // Bind parameters to the statement
  sqlite3_bind_text(stmt, 1, account->account_number, -1, NULL);
  sqlite3_bind_text(stmt, 2, account->customer_id, -1, NULL);
  sqlite3_bind_text(stmt, 3, account->account_type, -1, NULL);
  sqlite3_bind_double(stmt, 4, account->balance);

  // Execute the statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  } else {
    printf("Your account number is: %s", account->account_number);
    printf("Customer inserted successfully\n");
  }

  // Finalize the statement
  sqlite3_finalize(stmt);

  return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

// Account management menu logic
void print_account_management_system(sqlite3 *db) {
  clear_screen();
  display_account_menu();
  int choice;

  struct Account account;

  printf("Your choice? ");
  scanf("%d", &choice);
  clear_input_buffer();

  switch (choice) {
  case 1:
    clear_screen();
    srand(time(NULL)); // Initialize random seed

    printf("Customer Id? ");
    if (scanf("%38s", account.customer_id) != 1) { // Assuming max length of 36
      printf("Invalid input for Customer Id.\n");
      break;
    }
    clear_input_buffer();

    printf("Account Type (savings or current)? ");
    if (scanf("%9s", account.account_type) != 1) { // Limiting to 9 characters
      printf("Invalid input for Account Type.\n");
      break;
    }
    clear_input_buffer();

    printf("Initial Balance? ");
    if (scanf("%lf", &account.balance) != 1) {
      printf("Invalid input for Initial Balance.\n");
      break;
    }
    clear_input_buffer();

    printf("\n");

    generate_account_number(db, account.account_number);
    insert_account(db, &account);
    printf("Press Enter to return to main menu...");
    getchar(); // Wait for user to press Enter
    break;
  case 4:
    exit(0);
    break;
  }
}
