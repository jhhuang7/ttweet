#include "ttweet.h"

/**
 * Client request processing functions...
 */

/**
 * Function for connection Client to given port and IP.
 * Params: Port number, IP string, username string.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port, char* ip, char* username) {
    struct sockaddr_in servAddr;
    char buffer[BUFFERSIZE] = {0}; 

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(port); 
    
    if (inet_pton(AF_INET, ip, &servAddr.sin_addr) <= 0) { 
        printf("%s", INVSERIP); 
        return 0;
    } 
   
    // Check if given username is already connected.

    // Use while loop to connect user and continue to taken in their requests
    // from stdin, break if "exit". For each request, send message according
    // to protocol to server and directly print out message back from server.
    // A function will be needed to check each user request, then function(s)
    // to process the request.

    while (1) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) { 
            printf("%s", CONER); 
            return 0;
        }

        if (sock < 0) { 
            printf("%s", CONER); 
            return 0;
        } else {
            scanf("%s", buffer);
            send(sock, buffer, strlen(buffer), 0);
            fflush(stdin);
            memset(buffer, 0, sizeof(buffer));
        }
    }

    fflush(stdout);

    return 1;
}

/**
 * Checks whether the given command lines arguemnts are valid.
 * Params: argc and argv from the main function.
 * Returns 1 if ok and 0 otherwise.
 */
int check_args(int argc, char** argv) {
    // Check valid number of arguments
    if (argc != 4) {
        printf("%s", CARGE);
        return 0;
    }

    // Get the IP (IP is checked when trying to create connection)
    char* ip = argv[1];

    // Check if valid port
    char* port = argv[2];
    char* porterr;
    int portnum = strtol(port, &porterr, 10);

    if (portnum < MIN ||portnum > MAXPORT || strcmp(porterr, "") != 0) {
        printf("%s", INVSERPORT);
        return 0;
    }

    // Check if valid username (username existence is checked when connecting)
    char* username;
    username = argv[3];
    for (int i = 0; i < strlen(username); i++) {
        if (!isalnum((int)username[i])) {
            printf("%s", INVUSER);
            return 0;
        }
    }

    return network_connection(portnum, ip, username);
}

/**
 * Main function of program.
 */
int main(int argc, char** argv) {
    int args = check_args(argc, argv);
    if (!args) {
        return 0;
    }

    return 1;   
}
