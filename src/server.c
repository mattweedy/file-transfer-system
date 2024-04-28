#include <pwd.h>
#include <grp.h>
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
void *connection_handler(void *args);

// define constants
#define SERVER_PORT 8082
#define BUF_SIZE 1024
#define INFO_SIZE 1024

// connection handler
void *connection_handler(void *sock_desc) {
    int sock = *(int*)sock_desc;
    ssize_t READ_SIZE;
    char buffer[BUF_SIZE];
    char user_info[INFO_SIZE];
    char filepath[600];
    int file;

    memset(user_info, 0, INFO_SIZE);

    // read user info from client
    if ((READ_SIZE = recv(sock, user_info, sizeof(user_info), 0)) < 0) {
        perror("Failed to receive user info\n");
        return NULL;
    }
    user_info[READ_SIZE] = '\0';

    // extract user and group from user_info
    char *user = strtok(user_info, ":");
    char *group = strtok(NULL, ":");

    if (user == NULL || group == NULL) {
        perror("Error parsing user info");
        return NULL;
    }

    printf("Received FILE from USER:[%s], GROUP:[%s]\n", user, group);

    // use group name to determine directory
    snprintf(filepath, sizeof(filepath), "./server/%s/%s_file", group, user);

    // create directory if it doesn't exist
    char dirpath[1024];
    snprintf(dirpath, sizeof(dirpath), "./server/%s", group);
    struct stat st = {0};

    // check if directory exists
    if (stat(dirpath, &st) == -1) {
        mkdir(dirpath, 0700);
    }

    // open or create file to write data to
    file = open(filepath, O_WRONLY | O_CREAT, 0666);
    if (file < 0) {
        perror("Failed to open file");
        close(sock);
        return NULL;
    }

    // read data from client and write to file
    while ((READ_SIZE = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        write(file, buffer, READ_SIZE);
    }

    // check if data was received
    if (READ_SIZE == -1) {
        perror("Failed to receive data\n");
    }

    printf("File received and saved to %s\n", filepath);

    // close the file and socket
    close(file);
    close(sock);
    free(sock_desc);
    return NULL;
}

// main function
int main(void) {
    int sock_desc, client_sock, CONN_SIZE, *new_sock;
    struct sockaddr_in server, client;

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
    printf("Server listening on port %d\n", SERVER_PORT);

    // accept incoming connections
    CONN_SIZE = sizeof(struct sockaddr_in);
    while ((client_sock = accept(sock_desc, (struct sockaddr *)&client, (socklen_t*)&CONN_SIZE))) {
        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&thread_id, NULL, connection_handler, (void*) new_sock) < 0) {
            perror("Failed to create thread\n");
            return 1;
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