
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
#include <mongoc/mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* db_get_all_notes(int page) {
    int page_size = 10;
    int skip = page * page_size;

    bson_t *query = bson_new();
    bson_t *opts = bson_new();
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    int json_len = 2;  // "[]"
    char *json = malloc(sizeof(char) * json_len);
    strcpy(json, "[");  // Start JSON array
    int first = 1;

    bson_t projection;
    bson_init(&projection);
    // Get only id and title
    BSON_APPEND_INT32(&projection, "_id", 1);
    BSON_APPEND_INT32(&projection, "title", 1);
    BSON_APPEND_DOCUMENT(opts, "projection", &projection);

    BSON_APPEND_INT32(opts, "skip", skip);
    BSON_APPEND_INT32(opts, "limit", page_size);

    cursor = mongoc_collection_find_with_opts(collection, query, opts, NULL);
    if (!cursor) {
        fprintf(stderr, "Failed to initialize cursor\n");
        bson_destroy(query);
        bson_destroy(opts);
        bson_destroy(&projection);
        return NULL;
    }

    while (mongoc_cursor_next(cursor, &doc)) {
        char *str = bson_as_json(doc, NULL);
        int str_len = strlen(str);

        // Adjust size 
        json_len += str_len + (first ? 0 : 1);
        json = realloc(json, json_len);

        if (!first) {
            strcat(json, ",");
        }

        strcat(json, str);
        first = 0;

        bson_free(str);
    }

    // Check for cursor errors
    if (mongoc_cursor_error(cursor, NULL)) {
        fprintf(stderr, "Cursor error\n");
        bson_destroy(query);
        bson_destroy(opts);
        bson_destroy(&projection);
        mongoc_cursor_destroy(cursor);
        free(json);
        return NULL;
    }

    strcat(json, "]");

    // Cleanup
    bson_destroy(query);
    bson_destroy(opts);
    bson_destroy(&projection);
    mongoc_cursor_destroy(cursor);

    return json;
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

char* db_get_note_by_id_json(char* id){
    Note *note;
    uint32_t limit = 1; // Find one 
    bson_t *query;
    bson_t *field;
    bson_oid_t oid;
    mongoc_cursor_t *cursor; 
    const bson_t *doc;
    char *str;
    bool fail = true;
    
    field = bson_new();
    query = bson_new();

    bson_oid_init_from_string(&oid, id);
    BSON_APPEND_OID(query, "_id", &oid);

    cursor = mongoc_collection_find(collection, 0, 0, 0, 0, query, field, NULL);
    if(mongoc_cursor_next(cursor, &doc)){
        str = bson_as_json (doc, NULL);
        fail = false;
    }

    // Cleaning 
    bson_destroy(query);

    if(fail){
        return NULL;
    }
    else{
        return str;
    }
}
void db_update_note(char* id, Note *new_note){
    bson_t *selector;
    bson_t *update;
    selector = bson_new();
    update = bson_new();

    BSON_APPEND_UTF8(selector, "title", new_note->title);
    BSON_APPEND_UTF8(selector, "content", new_note->content);

    mongoc_collection_update_one(collection, selector, update, 0, 0, 0);
    bson_destroy(selector);
    bson_destroy(update);
}
void db_delete_note(char* id){
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, id);

    bson_t *selector;
    selector = bson_new();
    BSON_APPEND_OID(selector, "_id", &oid);

    mongoc_collection_delete_one(collection, selector, 0, 0, 0);
    printf("Object deleted successfully");
    bson_destroy(selector);
}