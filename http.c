#include <string.h>
#include "http.h"
#include <stdlib.h>
#include <stdio.h>


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

StatusLine* parse_status_line(char* status_line){
    char** parts = str_split(status_line, ' ');
    
    for (int i = 0; i < 2; i++)
    {

        printf("Part: %s\n", parts[i]);
        
    }

    StatusLine* status = (StatusLine*)malloc(sizeof(StatusLine));
    // Turn char version to enum one 
    char* method_str = parts[0];
    HttpMethod method;
    if(strcmp(method_str, "GET") == 0){
        method = GET;
    }else if(strcmp(method_str, "POST") == 0){
        method = POST;
    }else if(strcmp(method_str, "PUT") == 0){
        method = PUT;
    }else if(strcmp(method_str, "DELETE") == 0){
        method = DELETE;
    }

    status-> version = parts[2];
    status-> path = parts[1];
    status-> method = method;

    return status;
    

}

Request* parse_request_buffer(char* buffer) {
    if (buffer == NULL) {
        return NULL;
    }

    Request* request = (Request*)malloc(sizeof(Request));
    if (request == NULL) {
        return NULL;
    }
    char* version;
    HttpMethod method;
    char* path;
    
    char** lines = str_split(buffer,"\r\n");
    char* status_line = lines[0];
    parse_status_line(status_line);

    free(lines);
    free(status_line);

    return request;
}