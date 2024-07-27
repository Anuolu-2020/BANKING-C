#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sqlite3.h"
#include "uuid/uuid4.h"

struct Customer {
  char customer_id[37];
  char name[50];
  char address[50];
  char contact[50];
};

// For clearing input buffer
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

void clear_screen() {
#if defined(_WIN32) || defined(_WIN64)
  system("cls");
#else
  system("clear");
#endif

  // for (int i = 0; i < 100; i++) printf("\n");
}

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

int execute_sql(sqlite3 *db, char *sql) {
  char *err_msg = NULL;
  int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    return rc;
  }

  return SQLITE_OK;
}

static int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  fprintf(stderr, "%s: ", (const char *)data);

  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

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

int create_customers_table(sqlite3 *db) {
  char *sql;

  sql = "CREATE TABLE IF NOT EXISTS customers ("
        "customer_id TEXT PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "address TEXT, "
        "contact TEXT);";

  int rc = execute_sql(db, sql);

  if (rc != SQLITE_OK) {
    return rc;
  }

  return SQLITE_OK;
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

int create_accounts_table(sqlite3 *db) {
  char *sql;

  sql = "CREATE TABLE IF NOT EXISTS accounts ("
        "account_number INTEGER PRIMARY KEY, "
        "customer_id TEXT, "
        "account_type TEXT, balance REAL, "
        "FOREIGN KEY(customer_id) REFERENCES customers(customer_id));";

  // Execute sql
  int rc = execute_sql(db, sql);

  if (rc != SQLITE_OK) {
    return rc;
  }

  return SQLITE_OK;
}

// Generate uuids
int generate_uuid(struct Customer *customer) {
  UUID4_STATE_T state;
  UUID4_T uuid;

  uuid4_seed(&state);
  uuid4_gen(&state, &uuid);

  char buffer[UUID4_STR_BUFFER_SIZE];
  if (!uuid4_to_s(uuid, customer->customer_id, sizeof(buffer))) {
    return 0;
  }

  return 1;
}

int insert_customer(sqlite3 *db, struct Customer *customer) {
  sqlite3_stmt *stmt;

  const char *sql = "INSERT INTO customers (customer_id, name, address, "
                    "contact) VALUES (?, ?, ?, ?);";

  // Prepare the SQL statement
  int rc = sqlite3_prepare_v3(db, sql, -1, 0, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  // Bind parameters to the statement
  sqlite3_bind_text(stmt, 1, customer->customer_id, -1, NULL);
  sqlite3_bind_text(stmt, 2, customer->name, -1, NULL);
  sqlite3_bind_text(stmt, 3, customer->address, -1, NULL);
  sqlite3_bind_text(stmt, 4, customer->contact, -1, NULL);

  // Execute the statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  } else {
    printf("Customer inserted successfully\n");
  }

  // Finalize the statement
  sqlite3_finalize(stmt);

  return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int get_customer_details(sqlite3 *db, const char *customer_id) {
  sqlite3_stmt *stmt;
  int rc;
  char *zErrMsg = 0;
  // const char *data = "Callback function called";

  char *sql = "SELECT customer_id, name, address, contact FROM customers WHERE "
              "customer_id = ?;";

  /* Execute SQL statement */
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  // Bind parameters to the statement
  sqlite3_bind_text(stmt, 1, customer_id, -1, SQLITE_STATIC);

  int customer_count = 0;

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    customer_count++;
    const unsigned char *customer_id = sqlite3_column_text(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    const unsigned char *address = sqlite3_column_text(stmt, 2);
    const unsigned char *contact = sqlite3_column_text(stmt, 3);

    printf("Customer Details\n");
    printf("-----------------\n");
    printf("Customer ID: %s\n", customer_id);
    printf("Name: %s\n", name);
    printf("Address: %s\n", address);
    printf("Contact: %s\n", contact);
    printf("\n");
  }

  if (customer_count == 0) {
    printf("Customer does not exist.\n");
    return 2;
  }

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  }

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int select_customers_details(sqlite3 *db) {
  sqlite3_stmt *stmt;
  int rc;
  char *zErrMsg = 0;
  // const char *data = "Callback function called";

  char *sql = "SELECT * FROM customers;";

  /* Execute SQL statement */
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  int customer_count = 0;

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    customer_count++;
    const unsigned char *customer_id = sqlite3_column_text(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    const unsigned char *address = sqlite3_column_text(stmt, 2);
    const unsigned char *contact = sqlite3_column_text(stmt, 3);

    printf("Customers Details\n");
    printf("-----------------\n");
    printf("Customer ID: %s\n", customer_id);
    printf("Name: %s\n", name);
    printf("Address: %s\n", address);
    printf("Contact: %s\n", contact);
    printf("\n");
  }

  if (customer_count == 0) {
    printf("No Customers found.\n");
    return 0;
  } else {
    printf("Total Customers: %d\n", customer_count);
  }

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int update_customer_details(sqlite3 *db, const char *customer_id,
                            struct Customer *customer) {
  sqlite3_stmt *stmt = NULL;
  int rc;

  // Check if customer exists
  const char *check_sql =
      "SELECT COUNT(*) FROM customers WHERE customer_id = ?;";
  rc = sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare check statement: %s\n",
            sqlite3_errmsg(db));
    return rc;
  }

  sqlite3_bind_text(stmt, 1, customer_id, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    stmt = NULL;

    if (count == 0) {
      printf("Customer with ID %s does not exist.\n", customer_id);
      return SQLITE_OK; // Not an error, just customer doesn't exist
    }
  } else {
    fprintf(stderr, "Failed to check customer existence: %s\n",
            sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
  }

  // Customer exists, proceed with update
  const char *sql = "UPDATE customers SET name = ?, address = ?, contact = ? "
                    "WHERE customer_id = ?;";
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare update statement: %s\n",
            sqlite3_errmsg(db));
    return rc;
  }

  // Bind parameters to the statement
  sqlite3_bind_text(stmt, 1, customer->name, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, customer->address, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, customer->contact, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, customer_id, -1, SQLITE_STATIC);

  // Execute the statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  } else {
    printf("Customer updated successfully\n");
  }

  sqlite3_finalize(stmt);
  return (rc == SQLITE_DONE) ? SQLITE_OK : rc;
}

int delete_customer(sqlite3 *db, const char *customer_id) {
  sqlite3_stmt *check_stmt;
  const char *check_sql =
      "SELECT COUNT(*) FROM customers WHERE customer_id = ?;";
  int rc = sqlite3_prepare_v2(db, check_sql, -1, &check_stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare check statement: %s\n",
            sqlite3_errmsg(db));
    return rc;
  }

  sqlite3_bind_text(check_stmt, 1, customer_id, -1, SQLITE_STATIC);

  if (sqlite3_step(check_stmt) == SQLITE_ROW) {
    int count = sqlite3_column_int(check_stmt, 0);
    sqlite3_finalize(check_stmt);

    if (count == 0) {
      printf("Customer with ID %s does not exist.\n", customer_id);
      return SQLITE_OK; // Not an error, just customer doesn't exist
    }
  } else {
    fprintf(stderr, "Failed to check customer existence: %s\n",
            sqlite3_errmsg(db));
    sqlite3_finalize(check_stmt);
    return SQLITE_ERROR;
  }

  // If we're here, the customer exists. Proceed with deletion.
  sqlite3_stmt *stmt;
  const char *sql = "DELETE FROM customers WHERE customer_id = ?;";
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare delete statement: %s\n",
            sqlite3_errmsg(db));
    return rc;
  }

  sqlite3_bind_text(stmt, 1, customer_id, -1,
                    SQLITE_STATIC); // Corrected: bind to parameter 1, not 4

  // Execute the statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  } else {
    printf("Customer deleted successfully\n");
  }

  // Finalize the statement
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

void print_main_menu() {
  printf("Bank Management System\n");
  printf("----------------------\n");
  printf("1. Customer Management\n");
  printf("2. Account Management\n");
  printf("3. Transaction Management\n");
  printf("4. Exit\n");
}

void display_customer_menu() {
  printf("   1 Add New Customer\n");
  printf("   2 View Customers Details\n");
  printf("   3 Update Customer Information\n");
  printf("   4 Delete Customer\n");
}

void update_customer_menu(sqlite3 *db, char *customer_id,
                          struct Customer *customer) {
  printf("Update Customer detail\n");
  printf("Name? ");
  fgets(customer->name, sizeof(customer->name), stdin);
  customer->name[strcspn(customer->name, "\n")] =
      '\0'; // Remove newline character

  printf("Address? ");
  fgets(customer->address, sizeof(customer->address), stdin);
  customer->address[strcspn(customer->address, "\n")] = '\0';

  printf("Contact? ");
  fgets(customer->contact, sizeof(customer->contact), stdin);
  customer->contact[strcspn(customer->contact, "\n")] = '\0';

  printf("\n");

  update_customer_details(db, customer_id, customer);
}

void print_customer_management_system(sqlite3 *db) {
  clear_screen();
  display_customer_menu();

  int choice;

  struct Customer customer;

  printf("Your choice? ");
  scanf("%d", &choice);
  clear_input_buffer();

  switch (choice) {
  case 1:
    clear_screen();
    printf("Name? ");
    fgets(customer.name, sizeof(customer.name), stdin);
    customer.name[strcspn(customer.name, "\n")] =
        '\0'; // Remove newline character

    printf("Address? ");
    fgets(customer.address, sizeof(customer.address), stdin);
    customer.address[strcspn(customer.address, "\n")] =
        '\0'; // Remove newline character

    printf("Contact? ");
    fgets(customer.contact, sizeof(customer.contact), stdin);
    customer.contact[strcspn(customer.contact, "\n")] =
        '\0'; // Remove newline character

    printf("\n");

    generate_uuid(&customer);
    insert_customer(db, &customer);
    printf("Press Enter to return to main menu...");
    getchar(); // Wait for user to press Enter
    break;
  case 2:
    clear_screen();
    select_customers_details(db);
    printf("Press Enter to return to main menu...");
    getchar(); // Wait for user to press Enter
    break;
  case 3:
    clear_screen();
    char customer_id[UUID4_STR_BUFFER_SIZE];
    printf("Customer ID? ");
    scanf("%36s", customer_id);

    int result = get_customer_details(db, customer_id);

    if (result == 2) {
      clear_input_buffer();
      printf("Press Enter to return to main menu...");
      getchar();
    } else {
      clear_input_buffer();
      update_customer_menu(db, customer_id, &customer);
    }
    break;
  case 4:
    clear_screen();
    printf("Customer ID? ");
    scanf("%36s", customer_id);
    clear_input_buffer();
    delete_customer(db, customer_id);
    printf("Press Enter to return to main menu...");
    getchar(); // Wait for user to press Enter
    break;
  }
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

  return 0;
}
