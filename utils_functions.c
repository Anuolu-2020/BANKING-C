#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>

#include "utils_functions.h"
#include "uuid/uuid4.h"

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
}

// Utility function for executing some sql commands
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
