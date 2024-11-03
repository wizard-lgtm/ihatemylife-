#ifndef HTTP_H
#define HTTP_H

typedef struct{
    HttpMethod method;
    char* path;
    char* version;

} StatusLine;

typedef enum{
    GET,
    POST,
    PUT,
    DELETE
} HttpMethod;

typedef struct {
    char* version ;
    HttpMethod method;
    char* path;
} Request; 

Request* parse_request_buffer(char* buffer);

#endif