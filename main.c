#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <bits/socket.h>
#include <unistd.h>
#include <string.h>
#include <cmark.h>
#include <dirent.h>
#include "db.h"
#include "http.h"
#include <dotenv.h>

#define PORT 4000
#define QUEUE_SIZE 32
#define NOTES_DIR = "./notes"
#define INITIAL_REQUEST_BUFFER_SIZE 256 
#define REQUEST_BUFFER_INCREMENT_AMOUNT 512

// Function takes a body string and returns a raw HTTP 1.1 response buffer
char* compile_response(const char* body) {
    int body_len = strlen(body);

    // Prepare the header with placeholder for Content-Length
    char header_template[] = 
        "HTTP/1.1 200 OK\r\n"
        "Server: ihatemylife/1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n";

    // Calculate header length with the actual content length
    int header_len = snprintf(NULL, 0, header_template, body_len);
    char* header = malloc(header_len + 1); // +1 for null terminator
    if (!header) {
        perror("Failed to allocate memory for header");
        return NULL;
    }

    // Write the header with the actual content length
    snprintf(header, header_len + 1, header_template, body_len);

    // Calculate total response length
    int total_len = header_len + body_len;

    // Allocate memory for the complete response
    char* response = malloc(total_len + 1); // +1 for null terminator
    if (!response) {
        perror("Failed to allocate memory for response");
        free(header);
        return NULL;
    }

    // Concatenate header and body into the response buffer
    strcpy(response, header);
    strcat(response, body);

    // Free header memory and return the complete response
    free(header);
    return response;
}

// Reads whole file to a buffer
char* read_file_to_buffer(const char* file_path) {
    // Open the file in binary mode for reading
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Error: Unable to open file");
        return NULL; // Return NULL if the file doesn't exist or can't be opened
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file); // Go back to the beginning of the file

    // Allocate buffer to hold the file content
    char *buffer = malloc(file_size + 1); // +1 for null terminator
    if (buffer == NULL) {
        perror("Error: Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        perror("Error: Failed to read the entire file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0'; // Null-terminate the buffer

    fclose(file);
    return buffer; // Return the buffer with the file content
}


char* render_home(){
    char* str = "<h1>I hate my life</h1>";
    char* buffer = malloc(strlen(str));
    strcpy(buffer, str);
    return buffer;
}

char *compose_strings(const char *str1, const char *str2) {
    int size = snprintf(NULL, 0, "%s, %s!", str1, str2) + 1;   // Calculate required size
    char *result = malloc(size);                               // Allocate memory

    if (result != NULL) {
        snprintf(result, size, "%s, %s!", str1, str2);         // Format the strings
    }
    return result;                                             // Return the result
}


/// Dynamicly reads request by chunks and returns the allocated buffer 
char* allocate_request_buffer(int stream) {
    // Initial buffer setup
    int buffer_size = INITIAL_REQUEST_BUFFER_SIZE;
    int total_bytes = 0;
    char* request_buffer = malloc(sizeof(char) * buffer_size);
    if (!request_buffer) {
        return NULL;
    }
    
    // Read loop
    while (1) {
        // Calculate remaining space in buffer
        int remaining_space = buffer_size - total_bytes;
        
        // Read into current position in buffer
        int bytes_read = recv(stream, request_buffer + total_bytes, remaining_space, 0);
        
        // Handle errors and connection closure
        if (bytes_read <= 0) {
            if (bytes_read == 0) { // Connection closed
                break;
            } else { // Error occurred
                free(request_buffer);
                return NULL;
            }
        }
        
        total_bytes += bytes_read;
        
        // Check if we need more space
        if (total_bytes >= buffer_size) {
            buffer_size += REQUEST_BUFFER_INCREMENT_AMOUNT;
            char* new_buffer = realloc(request_buffer, buffer_size);
            if (!new_buffer) {
                free(request_buffer);
                return NULL;
            }
            request_buffer = new_buffer;
        }
        
        // Check for end of HTTP headers
        if (total_bytes >= 4 && 
            memcmp(request_buffer + total_bytes - 4, "\r\n\r\n", 4) == 0) {
            break;
        }
    }
    
    // Ensure null termination
    if (total_bytes < buffer_size) {
        request_buffer[total_bytes] = '\0';
    } else {
        char* new_buffer = realloc(request_buffer, buffer_size + 1);
        if (new_buffer) {
            request_buffer = new_buffer;
            request_buffer[total_bytes] = '\0';
        }
    }
    
    return request_buffer;
}
//  I USED CHATGPT HERE BC I'M A LOSER

void router(Request* request){

}


int main(){
    // Load dotenv
    env_load(".", false);
    // Connect to db 
    db_connect();
    char* json = db_get_all_notes(0);
    printf("all notes: %s\n", json);


    int socket_fd;
    int opt = 1;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("ERR: socket error\n");
        return 1;
    }
    
    // Set the SO_REUSEADDR option to allow reuse of the port
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("ERR: setsockopt error");
        close(socket_fd);
        return 1;
    }
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    addr.sin_family = AF_INET;
    
    if(bind(socket_fd, (struct sockaddr* )&addr, addr_len)< 0){
        perror("ERR: bind error\n");
        return 1;
    }

    if(listen(socket_fd, QUEUE_SIZE) < 0){
        perror("ERR: listen error\n");
        return 1;
    }

    while(1){

        int stream = accept(socket_fd, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
        if(stream < 0){
            perror("ERR: accept error\n");
            return 1;
        }

        char* request_buffer = allocate_request_buffer(stream);
        
        Request *request = parse_request_buffer(request_buffer);
        
        
    

        // Render home    
        if(strcmp(request->status->path, "/") == 0){
            printf("Home\n");
        }
        else{
            printf("Not home\n");
        }
        char* body = render_home();
        char* response = compile_response(body);
        char* headers_str = "Content-Type: text/html;\n";
        char* headers = (char*) malloc(strlen(headers_str)+1);
        strcpy(headers, headers_str);
        char* body_new = "<h1> I hate my life from repsonse object </h1>";
        Response* response_new = (Response*) malloc(sizeof(Response));
        response_new->status_code= 200;
        response_new->status_message= "OK";
        response_new->headers = headers;
        response_new->body = body_new;
        

        char* response_str = response_to_str(response_new);
        int bytes_written = write(stream, response_str, strlen(response_str));

        printf("Repsonse str:\n%s\n", response_str);
        free(response_str);

        free_request(request);
        free(response);
        if(bytes_written < 0){
            perror("ERR: write error\n");
            return 1;

        }
        else if(bytes_written == 0){
            printf("WARNING: Connection closed\n");
        }else{
            printf("%d bytes written\n", bytes_written);
        }
        

        // Cleanup 
        
        free(request_buffer);
        close(stream);

        printf("Connection closed\n");

    }

    close(socket_fd);
    return 0;
}