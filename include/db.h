#ifndef DB_H
#define DB_H
#include "types.h"

void db_connect();
void db_cleanup();
void db_get_all_notes();
void db_create_note(Note *note);
void db_get_note();
void db_update_note();
void db_delete_note();
#endif