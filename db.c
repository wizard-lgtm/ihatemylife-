
#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <stdio.h>
#include <stdlib.h>

#define DATABASE_NAME "note_database"
#define COLLECTION_NAME "notes"


mongoc_client_t *client;
mongoc_collection_t *collection;

void initialize_mongodb() {
    mongoc_init();
    const char* mongodb_uri = getenv("MONGODB_URI");
    if (mongodb_uri == NULL) {
        fprintf(stderr, "Error: MONGODB_URI environment variable not set\n");
        exit(EXIT_FAILURE);
    }

    client = mongoc_client_new(mongodb_uri);
    collection = mongoc_client_get_collection(client, DATABASE_NAME, COLLECTION_NAME);
}

void cleanup_mongodb() {
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}

void db_get_all_notes(){}
void db_create_note(){}
void db_get_note(){}
void db_update_note(){}
void db_delete_note(){}