#ifndef DB_H
#define DB_H
#include "types.h"

void db_connect();
void db_cleanup();
char* db_get_all_notes(int page);
void db_create_note(Note *note);
char* db_get_note_by_id_json(char* );
void db_update_note();
void db_delete_note(char *id);
#endif