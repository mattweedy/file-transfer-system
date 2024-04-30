#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// function signatures
int main(int argc, char *argv[]);

// define constants
#define SERVER_PORT 8082
#define BUF_SIZE 1024
#define INFO_SIZE 100

// main function
int main(int argc, char *argv[]) {
    int sock_desc;
    struct sockaddr_in server;
    char *filepath;
    char buffer[BUF_SIZE] = {0};
    ssize_t READ_SIZE;
    struct passwd *pw;
    struct group *grp;
    uid_t user_id;
    gid_t group_id;
    char user_info[INFO_SIZE];

    // check if the user has provided a filepath
    if (argc < 2) {
        printf("Usage: %s <filepath>\n", argv[0]);
        return 1;
    }

    // get the id and filepath from the user
    filepath = argv[1];
    user_id = getuid();
    group_id = getgid();
    pw = getpwuid(user_id);
    grp = getgrgid(group_id);

    if (!pw || !grp) {
        perror("FAILED to get user or group\n");
        return 1;
    }

    // check if user should be allowed to send the file
    char real_path[200];
    if (realpath(filepath, real_path) == NULL) {
        perror("FAILED to resolve real path");
        return 1;
    }

    // check if the file belongs to the user's group
    char expected_prefix[200];
    char absolute_expected_prefix[200];
    snprintf(expected_prefix, sizeof(expected_prefix), "../files/%s", grp->gr_name);
    if (realpath(expected_prefix, absolute_expected_prefix) == NULL) {
        perror("FAILED to resolve absolute path for expected prefix");
        return 1;
    }

    // Check if the real path starts with the expected prefix
    if (strncmp(real_path, absolute_expected_prefix, strlen(absolute_expected_prefix)) != 0) {
        fprintf(stderr, "ERROR: Users can only transfer files owned by their group.\n");
        return 1;
    }

    // prepare user info to send
    snprintf(user_info, sizeof(user_info), "%s:%s", pw->pw_name, grp->gr_name);

    // create a socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        perror("FAILED to create socket\n");
        return 1;
    } else {
        printf("CREATED socket\n");
    }

    // set socket variables
    server.sin_port = htons(SERVER_PORT);                   // port number
    server.sin_family = AF_INET;                     // IPv4
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    // connect to the server
    if (connect(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("FAILED to connect\n");
        return 1;
    }

    printf("SUCCESS Connected to server\n");

    // send the user id to the server
    if (send(sock_desc, user_info, strlen(user_info), 0) < 0) {
        perror("FAILED to send user id\n");
        return 1;
    }

    // open the file
    int file = open(filepath, O_RDONLY);
    if (file < 0) {
        perror("FAILED to open file");
        return 1;
    }

    // send file contents
    while ((READ_SIZE = read(file, buffer, sizeof(buffer))) > 0) {
        if (send(sock_desc, buffer, READ_SIZE, 0) < 0) {
            perror("FAILED to send file\n");
            close(file);
            return 1;
        }
    }

    // check if the file was read successfully
    if (READ_SIZE < 0) {
        perror("FAILED to read file\n");
        close(file);
        return 1;
    }

    printf("SENT file [%s] from transfer owner [%s] successfully\n", filepath, pw->pw_name);

    // close the file and socket
    close(file);
    close(sock_desc);
    return 0;
}