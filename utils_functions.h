#ifndef UTILS_FUNCTIONS_H
#define UTILS_FUNCTIONS_H

#include "customer_system.h"
#include "sqlite3.h"

void clear_input_buffer();
void clear_screen();
int execute_sql(sqlite3 *db, char *sql);
int generate_uuid(struct Customer *customer);

#endif
