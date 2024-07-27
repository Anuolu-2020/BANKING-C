
#ifndef CUSTOMER_SYSTEM_H
#define CUSTOMER_SYSTEM_H

#include "sqlite3.h"

struct Customer {
  char customer_id[38];
  char name[50];
  char address[50];
  char contact[50];
};

int insert_customer(sqlite3 *db, struct Customer *customer);
int get_customer_details(sqlite3 *db, const char *customer_id);
int select_customers_details(sqlite3 *db);
int update_customer_details(sqlite3 *db, const char *customer_id,
                            struct Customer *customer);
int delete_customer(sqlite3 *db, const char *customer_id);
void display_customer_menu();
void update_customer_menu(sqlite3 *db, char *customer_id,
                          struct Customer *customer);
void print_customer_management_system(sqlite3 *db);

#endif
