#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// function signatures
int main(int argc, char *argv[]);


// main function
int main(int argc, char *argv[]) {
    int sock_desc;
    struct sockaddr_in server;
    char *filepath;
    char buffer[1024] = {0};
    int file;
    ssize_t READ_SIZE;
    char *user_id = "123";

    // check if the user has provided a filepath
    if (argc < 2) {
        printf("Usage: %s <filepath>\n", argv[0]);
        return 1;
    }

    // get the filepath from the user
    filepath = argv[1];

    // create a socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        perror("Couldn't create socket\n");
        return 1;
    } else {
        printf("Socket created\n");
    }

    // set socket variables
    server.sin_port = htons(8082);       // port number
    server.sin_family = AF_INET;         // IPv4
    server.sin_addr.s_addr = INADDR_ANY; // localhost

    // connect to the server
    if (connect(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    // send the user id to the server
    if (send(sock_desc, user_id, strlen(user_id), 0) < 0) {
        perror("Failed to send user id\n");
        return 1;
    }

    // open the file
    file = open(filepath, O_RDONLY);
    if (file < 0) {
        perror("Failed to open file");
        return 1;
    }

    // send file contents
    while ((READ_SIZE = read(file, buffer, sizeof(buffer))) > 0) {
        if (send(sock_desc, buffer, READ_SIZE, 0) < 0) {
            perror("Failed to send file\n");
            close(file);
            return 1;
        }
    }

    // check if the file was read successfully
    if (READ_SIZE < 0) {
        perror("Failed to read file\n");
        close(file);
        return 1;
    }

    printf("File sent successfully\n");

    // close the file and socket
    close(file);
    close(sock_desc);
    return 0;
}