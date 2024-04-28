#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// function signatures
void *connection_handler(void *socket_desc);

// define constants
#define SERVER_PORT 8082
#define BUF_SIZE 1024

// connection handler
void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    int READ_SIZE;
    char client_message[BUF_SIZE];
    char user_id[100];
    char filepath[600];

    // receive user id
    if ((READ_SIZE = recv(sock, user_id, sizeof(user_id), 0)) < 0) {
        user_id[READ_SIZE] = '\0';
    } else {
        perror("Failed to receive user id\n");
        close(sock);
        free(socket_desc);
        return NULL;
    }

    printf("User ID : %s\n", user_id);

    // construct filepath
    snprintf(filepath, sizeof(filepath), "./server/%s_file", user_id);

    // open file to write data
    int file = open(filepath, O_WRONLY | O_CREAT, 0666);
    if (file < 0) {
        perror("Failed to open file");
        close(sock);
        free(socket_desc);
        return NULL;
    }

    // receive data from client
    while ((READ_SIZE = recv(sock, client_message, BUF_SIZE, 0)) > 0) {
        write(file, client_message, READ_SIZE);
    }

    if (READ_SIZE == -1) {
        perror("Failed to receive data\n");
    }
    
    printf("File received and saved to %s\n", filepath);

    // close the file and socket
    close(file);
    close(sock);
    free(socket_desc);
    return NULL;
}

// main function
int main(void) {
    int sock_desc, client_sock, CONN_SIZE;
    struct sockaddr_in server, client;
    pthread_t thread_id;

    // create a socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        perror("Couldn't create socket\n");
        return 1;
    } else {
        printf("Socket created\n");
    }

    // set socket variables
    server.sin_port = htons(SERVER_PORT); // port number
    server.sin_family = AF_INET;          // IPv4
    server.sin_addr.s_addr = INADDR_ANY;  // localhost

    // bind
    if (bind(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed\n");
        return 1;
    } else {
        printf("Bind successful\n");
    }

    // listen
    listen(sock_desc, 3);

    // accept incoming connections
    printf("Server listening on port %d\n", SERVER_PORT);
    CONN_SIZE = sizeof(struct sockaddr_in);
    while ((client_sock = accept(sock_desc, (struct sockaddr *)&client, (socklen_t*)&CONN_SIZE))) {
        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            perror("Failed to allocate memory\n");
            continue;
        }
        *new_sock = client_sock;

        // create a new thread for each connection
        if (pthread_create(&thread_id, NULL, connection_handler, (void*)new_sock) < 0) {
            perror("Failed to create thread\n");
            free(new_sock);
            continue;
        }
    }

    // check if accept failed
    if (client_sock < 0) {
        perror("Accept failed\n");
        return 1;
    }

    // close the socket
    close(sock_desc);
    return 0;
}