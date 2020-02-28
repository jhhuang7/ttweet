#include "ttweet.h"

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
    char message[BUFFERSIZE];
    strcpy(message, "EMPTYMSG"); 
   
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
            
            if (strncmp(buffer, "-u", 2) == 0) {
                memset(message, 0, sizeof(message));
                strncpy(message, buffer + 2, MSGMAX);
                send(socket, SUCCOP, strlen(SUCCOP), 0);
                printf("%s\n", "-u");
            } else if (strncmp(buffer, "-d", 2) == 0) {
                send(socket, message, strlen(message), 0);
                printf("%s\n", "-d");
            } 
    
            fflush(stdout);
            memset(buffer, 0, sizeof(buffer));
        }
    }

    return 1; 
}

/**
 * Main function of program.
 */
int main(int argc, char** argv) {
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
