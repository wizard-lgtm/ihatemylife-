
#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#define DATABASE_NAME "website"
#define COLLECTION_NAME "notes"


mongoc_client_t *client;
mongoc_collection_t *collection;
mongoc_database_t *database;

void db_cleanup() {
    mongoc_database_destroy(database);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();
}


void db_connect() {
    bson_t *command = NULL;
    bson_t reply = BSON_INITIALIZER;
    bson_error_t error = {0};
    bool ok;

    mongoc_init();
    const char* mongodb_uri = getenv("MONGODB_URI");
    printf("Mongodb uri: %s\n");
    if (mongodb_uri == NULL) {
        fprintf(stderr, "Error: MONGODB_URI environment variable not set\n");
        exit(EXIT_FAILURE);
    }

    client = mongoc_client_new(mongodb_uri);
    database = mongoc_client_get_database(client, DATABASE_NAME);
    collection = mongoc_client_get_collection(client, DATABASE_NAME, COLLECTION_NAME);

    // Ping to database

    command = BCON_NEW("ping", BCON_INT32(1));
    ok = mongoc_database_command_simple(database, command, NULL, &reply, &error);
    if(!ok){
        fprintf(stderr, "Database ping err: %s", error.message);
        db_cleanup();
        exit(EXIT_FAILURE);
    }
    else{
        printf("Connected to database!\n");
    }
    bson_destroy(command);
    
    bson_destroy(&reply);

}
void db_get_all_notes(){
   
}
void db_create_note(Note *note){
    bson_t *document;
    document = bson_new();
    bson_error_t error = {0};
    bson_t reply;

    BSON_APPEND_UTF8(document, "title", note->title);
    BSON_APPEND_UTF8(document, "content", note->content);

    mongoc_collection_insert_one(collection, document, NULL, &reply, &error);

    printf("Note created successfully!\n");

    // Cleaning 
    bson_destroy(document);
    bson_destroy(&reply);
}

void db_get_note(){}
void db_update_note(){}
void db_delete_note(){}