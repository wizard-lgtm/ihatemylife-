#ifndef HTTP_H
#define HTTP_H



typedef enum{
    GET,
    POST,
    PUT,
    DELETE
} HttpMethod;
typedef struct{
    HttpMethod method;
    char* path;
    char* version;

} StatusLine;
typedef struct {
    StatusLine* status;
    char* body;

} Request; 

typedef struct{
    char* status_message; // not heap
    int status_code; // int  
    char* headers; // heap 
    char* body; // heap 
} Response;

Request* parse_request_buffer(char* buffer);
void free_request(Request* request);
// turns a list of headers to str 
char* response_to_str(Response* response);
void free_response(Response* response);

#endif