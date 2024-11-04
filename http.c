#include <string.h>
#include "http.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

    // Find the body separator (\r\n\r\n)
    char* body_start = strstr(buffer, "\r\n\r\n");
    char* body = NULL;
    
    if (body_start != NULL) {
        // Move pointer past the separator
        body_start += 4;
        
        // Calculate body length
        size_t body_length = strlen(body_start);
        
        // Allocate and copy body if it exists
        if (body_length > 0) {
            body = (char*)malloc(body_length + 1);
            if (body != NULL) {
                strcpy(body, body_start);
            }
        }
        
        // Temporarily null-terminate headers section
        *body_start = '\0';
    }
    /// DERIVED FROM CHATGPT BC I'M A LOSER

    
    char** lines = str_split(buffer,'\r');
    char* status_line_str = lines[0];
    printf("Request buffer is %s\n", buffer);
    printf("Status line str: %s\n", status_line_str);

    StatusLine* status_line = parse_status_line(status_line_str);
    request->status = status_line;
    request->body = body;
    
    

    free(lines);
    free(status_line_str);

    return request;
}
void free_request(Request* request){
    free(request->body);
    free(request->status);
    free(request);
}

// Turns response object to raw http string 
char* response_to_str(Response* response){

    // Turn status code to str 
    char* http_version = "HTTP/1.1";
    int http_version_len = strlen(http_version);
    int status_message_len = strlen(response->status_message);
    int status_code_len = 3;
    int status_line_len = http_version_len + status_message_len + status_code_len + 1 + 3 + 1; // one for null terminator 3 for spaces

    char* status_line_str = (char*)malloc(status_line_len);

    snprintf(status_line_str, status_line_len, "%s %d %s\n", http_version, response->status_code, response->status_message);

    // Header 
    int headers_len = strlen(response->headers);

    // Add content length to headers
    char* content_length_header_line_str_format = "Content-Length: %d;\r\n\r\n";
    int content_length_header_line_len = strlen("Content-Length: ") + 20; // 20 len of int who can do much more 
    char* content_length_header_line_str = (char*)malloc(content_length_header_line_len);
    int body_len = strlen(response->body);

    snprintf(content_length_header_line_str, content_length_header_line_len, content_length_header_line_str_format, body_len);

    // Add content length header to headers
    int new_headers_len = headers_len + content_length_header_line_len;
    char* new_headers = realloc(response->headers, new_headers_len);
    
    
    strcat(new_headers, content_length_header_line_str);

    response->headers = new_headers;

    // Body 
    int response_str_len;
    response_str_len += status_message_len;
    response_str_len += headers_len;
    response_str_len += body_len;
    char* response_str = (char*)malloc(response_str_len);
    snprintf(response_str, response_str_len, "%s%s%s", status_line_str, response->headers, response->body);

    

    return response_str;
    

} 