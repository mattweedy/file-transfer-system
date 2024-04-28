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

// main function
int main(int argc, char *argv[]) {
    int sock_desc;
    struct sockaddr_in server;
    char *filepath;
    char buffer[1024] = {0};
    ssize_t READ_SIZE;
    struct passwd *pw;
    struct group *grp;
    uid_t user_id;
    gid_t group_id;
    char user_info[512];

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
        perror("Failed to get user or group\n");
        return 1;
    }

    // prepare user info to send
    snprintf(user_info, sizeof(user_info), "%s:%s", pw->pw_name, grp->gr_name);

    // create a socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        perror("Couldn't create socket\n");
        return 1;
    } else {
        printf("Socket created\n");
    }

    // set socket variables
    server.sin_port = htons(8082);                   // port number
    server.sin_family = AF_INET;                     // IPv4
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    // connect to the server
    if (connect(sock_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    // send the user id to the server
    if (send(sock_desc, user_info, strlen(user_info), 0) < 0) {
        perror("Failed to send user id\n");
        return 1;
    }

    // open the file
    int file = open(filepath, O_RDONLY);
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

    printf("File [%s] from transfer owner [%s] sent successfully\n", filepath, pw->pw_name);

    // close the file and socket
    close(file);
    close(sock_desc);
    return 0;
}