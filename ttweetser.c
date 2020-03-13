#include "ttweet.h"

/**
 * Functions for Processing client requests ...
 * (store stuff, send back messages, etc.)
 */

/**
 * Function for connection Server to given port forever.
 * Params: Port number.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port) {
    int serverfd; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    char buffer[BUFFERSIZE] = {0}; 
   
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == 0) {
        return 0;
    } 
    
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | 
            SO_REUSEPORT, &opt, sizeof(opt))) {
        return 0; 
    }
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port);
    
    if (bind(serverfd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
        return 0; 
    }

    if (listen(serverfd, 3) < 0) { 
        return 0; 
    }

    while (1) {
        int socket = accept(serverfd, (struct sockaddr*)&address, 
            (socklen_t*)&addrlen);

        if (socket >= 0) { 
            read(socket, buffer, BUFFERSIZE);

            printf("%s\n", buffer);

            fflush(stdout);
            memset(buffer, 0, sizeof(buffer));
        }

        // Use multi-threading to turn this into handling up to 5 clients.
        // For each client read their request and send this for processing.
        // May need locking.

    }

    return 1; 
}

/**
 * Checks whether the given command lines arguemnts are valid.
 * Params: argc and argv from the main function.
 * Returns 1 if ok and 0 otherwise.
 */
int check_args(int argc, char** argv) {
    // Check valid number of arguments
    if (argc != 2) {
        printf("%s", SARGE);
        return 0;
    }

    // Check if valid port
    char* port = argv[1];
    char* porterr;
    int portnum = strtol(port, &porterr, 10);

    if (portnum < MIN ||portnum > MAXPORT || strcmp(porterr, "") != 0) {
        printf("%s", INVSERPORT);
        return 0;
    }

    // Server running forever
    return network_connection(portnum);
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
