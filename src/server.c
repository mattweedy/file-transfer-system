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
#define INFO_SIZE 100

// connection handler
void *connection_handler(void *sock_desc) {
    int sock = *(int*)sock_desc;
    ssize_t READ_SIZE;
    char buffer[BUF_SIZE];
    char user_info[INFO_SIZE];
    char dirpath[200];
    int file;

    memset(user_info, 0, INFO_SIZE);

    // read user info from client
    if ((READ_SIZE = recv(sock, user_info, sizeof(user_info), 0)) < 0) {
        perror("FAILED to receive USER INFO\n");
        return NULL;
    }
    user_info[READ_SIZE] = '\0';

    // extract user and group from user_info
    char *user = strtok(user_info, ":");
    char *group = strtok(NULL, ":");

    if (user == NULL || group == NULL) {
        perror("ERROR parsing USER INFO");
        return NULL;
    }

    printf("RECEIVED file from USER:[%s], GROUP:[%s]\n", user, group);

    // create directory if it doesn't exist
    snprintf(dirpath, sizeof(dirpath), "../server/%s", group);
    struct stat st = {0};

    // check if directory exists
    if (stat(dirpath, &st) == -1) {
        // create directory and set permissions
        mkdir(dirpath, 0770);
        chown(dirpath, *user, *group);
    }

    // calculate the maximum size needed for filepath
    size_t filepath_size = strlen(dirpath) + strlen(user) + strlen("_file.txt") + 2;

    // declare filepath
    char filepath[filepath_size];

    // construct the file path
    snprintf(filepath, filepath_size, "%s/%s_file.txt", dirpath, user);

    // open or create file to write data to
    file = open(filepath, O_WRONLY | O_CREAT, 0660);
    if (file < 0) {
        perror("FAILED to open FILE");
        close(sock);
        return NULL;
    }

    // set file permissions
    fchmod(file, 0660);
    fchown(file, *user, *group);

    // read data from client and write to file
    while ((READ_SIZE = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        write(file, buffer, READ_SIZE);
    }

    // check if data was received
    if (READ_SIZE == -1) {
        perror("FAILED to receive DATA\n");
    }

    printf("SAVED file to [%s]\n", filepath);

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
        perror("FAILED to create socket\n");
        return 1;
    } else {
        printf("CREATED socket\n");
    }

    // set socket variables
    server.sin_port = htons(SERVER_PORT); // port number
    server.sin_family = AF_INET;          // IPv4
    server.sin_addr.s_addr = INADDR_ANY;  // localhost

    // bind
    if (bind(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("FAILED Bind\n");
        return 1;
    } else {
        printf("SUCCESS Bind\n");
    }

    // listen
    listen(sock_desc, 3);
    printf("SERVER LISTENING on port [%d]...\n", SERVER_PORT);

    // accept incoming connections
    CONN_SIZE = sizeof(struct sockaddr_in);
    while ((client_sock = accept(sock_desc, (struct sockaddr *)&client, (socklen_t*)&CONN_SIZE))) {
        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&thread_id, NULL, connection_handler, (void*) new_sock) < 0) {
            perror("FAILED to create thread\n");
            return 1;
        }
    }

    // check if accept failed
    if (client_sock < 0) {
        perror("FAILED accept\n");
        return 1;
    }

    // close the socket
    close(sock_desc);
    return 0;
}