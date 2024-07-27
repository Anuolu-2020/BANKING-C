
#ifndef GEN_ACCOUNT_NUMBER_H
#define GEN_ACCOUNT_NUMBER_H

#include "sqlite3.h"
#include <time.h>

// Define the length of the account number
#define ACCOUNT_NUMBER_LENGTH 10

// Function prototype for generating an account number
void generate_account_number(sqlite3 *db, char *account_number);

#endif
