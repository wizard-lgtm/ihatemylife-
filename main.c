#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <bits/socket.h>
#include <unistd.h>
#include <string.h>

#define PORT 4000
#define QUEUE_SIZE 32

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


int main(){
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


        // Alloc buffer for reading request

        int buffer_size = 1024;
        char* request_buffer = malloc(sizeof(char) * buffer_size);
        
        int bytes_read = recv(stream, request_buffer, buffer_size, 0);
        
        printf("%d bytes read\nRequest: %s\n", bytes_read, request_buffer);

        char* body = "<h1>I hate my life</h1><h2>But i hate c++ more</h2>"; 
        char* response = compile_response(body);


        int bytes_written = write(stream, response, strlen(response));

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
        free(response);
        free(request_buffer);
        close(stream);

        printf("Connection closed\n");

    }
    close(socket_fd);
    return 0;
}