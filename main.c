#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "account_system.h"
#include "customer_system.h"
#include "gen_account_number.h"
#include "sqlite3.h"
#include "utils_functions.h"

/** function prototypes**/
// Initialize database
int initialize_database(sqlite3 **db);
// Create customer table
int create_customers_table(sqlite3 *db);
// Create accounts table
int create_accounts_table(sqlite3 *db);
// Create transactions table
int create_transactions_table(sqlite3 *db);
// Cli main event loop
int cli_event_loop(sqlite3 *db);

int main(int argv, char **argc) {
  sqlite3 *db;
  // char *zErrMsg = 0;

  initialize_database(&db);
  clear_screen();
  cli_event_loop(db);

  return 0;
}

int initialize_database(sqlite3 **db) {
  int rc;

  rc = sqlite3_open("bank.db", db);

  if (rc) {
    fprintf(stderr, "Can't open db: %s\n", sqlite3_errmsg(*db));
    return rc;
  } else {
    fprintf(stdout, "Opened database successfully\n");
  }

  // Create the customers table
  rc = create_customers_table(*db);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create customers table\n");
    sqlite3_close(*db);
    return rc;
  }

  printf("Customers table created successfully\n");

  // Create the accounts table
  rc = create_accounts_table(*db);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create accounts table\n");
    sqlite3_close(*db);
    return rc;
  }

  printf("Accounts table created successfully\n");

  // Create the transactions table
  rc = create_transactions_table(*db);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create transactions table\n");
    sqlite3_close(*db);
    return rc;
  }

  printf("Transactions table created successfully\n");
}

int create_transactions_table(sqlite3 *db) {
  char *sql;

  sql = "CREATE TABLE IF NOT EXISTS transactions ("
        "transaction_id TEXT PRIMARY KEY, "
        "account_number INTEGER, "
        "date TEXT, "
        "amount REAL, "
        "type TEXT, "
        "FOREIGN KEY(account_number) REFERENCES accounts(account_number)); ";

  int rc = execute_sql(db, sql);

  if (rc != SQLITE_OK) {
    return rc;
  }

  return SQLITE_OK;
}

void print_main_menu() {
  printf("Bank Management System\n");
  printf("----------------------\n");
  printf("1. Customer Management\n");
  printf("2. Account Management\n");
  printf("3. Transaction Management\n");
  printf("4. Exit\n");
}

int cli_event_loop(sqlite3 *db) {
  int choice;
  char input[10];

  do {
    clear_screen();
    print_main_menu();

    printf("Your choice?: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
      // Remove trailing newline character if present
      input[strcspn(input, "\n")] = 0;

      // Check if the input is a valid integer
      if (sscanf(input, "%d", &choice) != 1) {
        printf("Invalid input! Please enter a number.\n");
        continue;
      }

      switch (choice) {
      case 1:
        clear_screen();
        print_customer_management_system(db);
        break;
      case 2:
        clear_screen();
        print_account_management_system(db);
        break;
      case 4:
        exit(0);
      default:
        printf("Invalid choice!\n");
      }

    } else {
      // Handle error in reading input
      printf("Error reading input!\n");
      clearerr(stdin); // Clear the error flag on stdin
    }

  } while (choice != 4);
}
