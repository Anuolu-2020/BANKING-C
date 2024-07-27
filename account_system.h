
#ifndef ACCOUNT_SYSTEM_H
#define ACCOUNT_SYSTEM_H

#include "gen_account_number.h"
#include "sqlite3.h"

struct Account {
  char account_number[ACCOUNT_NUMBER_LENGTH];
  char customer_id[37];
  char account_type[8];
  double balance;
};

int create_accounts_table(sqlite3 *db);
int insert_account(sqlite3 *db, struct Account *account);
void print_account_management_system(sqlite3 *db);

#endif
