#include "ttweet.h"

/**
 * Function for connection Client to given port and IP.
 * Params: Port number, IP string, Message string, Client mode string.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port, char* ip, char* message, char* mode) {
    int sock = 0; 
    struct sockaddr_in servAddr; 
    char buffer[BUFFERSIZE] = {0}; 

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        printf("%s", CONER); 
        return 0;
    } 

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(port); 
    
    if (inet_pton(AF_INET, ip, &servAddr.sin_addr) <= 0) { 
        printf("%s", INVSERIP); 
        return 0;
    } 
   
    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) { 
        printf("%s", CONER); 
        return 0;
    }

    if (strcmp(mode, "-u") == 0) {
        // Build message to send to Server
        char info[BUFFERSIZE];
        strcpy(info, "-u");
        strcat(info, message);
        strcat(info, "\0");

        send(sock, info, strlen(info), 0);
        read(sock, buffer, BUFFERSIZE); 
        printf("%s", buffer);
    } else if (strcmp(mode, "-d") == 0) {
        send(sock, "-d", strlen("-d"), 0);
        read(sock, buffer, BUFFERSIZE); 
        printf("%s\n", buffer);
    }

    fflush(stdout);

    return 1;
}

/**
 * Main function of program.
 */
int main(int argc, char** argv) {
    // Check valid number of arguments
    if (argc != 4 && argc != 5) {
        printf("%s", CARGE);
        return 0;
    }

    // Check if valid mode
    char* mode = argv[1];
    if (argc == 5 && strcmp(mode, "-u") != 0) {
        printf("%s", CARGE);
        return 0;
    }
    if (argc == 4 && strcmp(mode, "-d") != 0) {
        printf("%s", CARGE);
        return 0;
    }

    // Get the IP
    char* ip = argv[2];

    // Check if valid port
    char* port = argv[3];
    char* porterr;
    int portnum = strtol(port, &porterr, 10);

    if (portnum < MIN ||portnum > MAXPORT || strcmp(porterr, "") != 0) {
        printf("%s", INVSERPORT);
        return 0;
    }

    // Check if valid message
    char* message ;
    if (argc == 5) {
        message = argv[4];

        if (strlen(message) > MSGMAX) {
            printf("%s", ILLMSGLEN);
            return 0;
        }
    }

    return network_connection(portnum, ip, message, mode);
}
