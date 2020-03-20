#include "ttweet.h"

/**
 * Stores a user's tweet and sends tweet to all other users who have subscribed
 * to the attached hashtag.
 * Params: lists of users, specific user index, tweet string, list of hashtags,
 * number of hashtags.
 * Returns 1.
 */
int handle_tweet(User* users, int id, char* tweet, char** hashtag, int hashes) {
    // Store the tweet
    strcpy(users[id].tweets[users[id].numtwts].message, tweet);
    strcpy(users[id].tweets[users[id].numtwts].hashtag, "");
    for (int i = 0; i < hashes; i++) {
        strcat(users[id].tweets[users[id].numtwts].hashtag, hashtag[i]);
    }
    users[id].numtwts += 1;
    // For tweet operation, no feedback needed

    // Deliver to all the clients who are subscribed to the hashtag
    char response[BUFFERSIZE];
    strcpy(response, "");
    strcat(response, "\"");
    strcat(response, tweet);
    strcat(response, "\" ");
    for (int i = 0; i < hashes; i++) {
        strcat(response, hashtag[i]);
    }
    strcat(response, "\n");

    for (int i = 0; i < MAXCONNS; i++) {
        for (int j = 0; j < MAXHASH; j++) {
            for (int k = 0; k < hashes; k++) {
                if (strcmp(users[i].subscriptions[j], hashtag[k]) == 0 
                        || strcmp(users[i].subscriptions[j], ALL) == 0) {
                    send(users[i].socket, response, strlen(response), 0);
                    break;
                }
            }
            break;
        }
    }

    return 1;
}

/**
 * Stores a hashtag subscription for a user.
 * Params: lists of users, specific user index, hashtag string.
 * Returns 1 or success and 0 on failure.
 */
int handle_subscribe(User* users, int id, char* hashtag) {
    // Max hashtag limit reached
    if (users[id].numsusbs >= MAXHASH) {
        char response[BUFFERSIZE];
        strcpy(response, "");
        strcat(response, "operation failed: sub ");
        strcat(response, hashtag);
        strcat(response, " failed, already exists or exceeds 3 limitation\n");
        send(users[id].socket, response, strlen(response), 0);
        return 0;
    }

    // Store hashtag with user
    for (int i = 0; i < MAXHASH; i++) {
        if (strcmp(users[id].subscriptions[i], "") == 0) {
            strcpy(users[id].subscriptions[i], hashtag);
            break;
        }
    }
    users[id].numsusbs += 1;
    send(users[id].socket, SUCCOP, strlen(SUCCOP), 0);
    return 1;
}

/**
 * Removes a hashtag subscription for a user.
 * Params: lists of users, specific user index, hashtag string.
 * Returns 1 on success and 0 on failure.
 */
int handle_unsubscribe(User* users, int id, char* hashtag) {
    // If the hashtag is #ALL then remove all subscriptions
    if (strcmp(hashtag, ALL) == 0) {
        for (int i = 0; i < MAXHASH; i++) {
            strcpy(users[id].subscriptions[i], "\0");
        }
        users[id].numsusbs = 0;
        send(users[id].socket, SUCCOP, strlen(SUCCOP), 0);
        return 1;
    }

    // Remove specific hashtag
    for (int i = 0; i < MAXHASH; i++) {
        if (strcmp(users[id].subscriptions[i], hashtag) == 0) {
            strcpy(users[id].subscriptions[i], "\0");
            users[id].numsusbs -= 1;
            send(users[id].socket, SUCCOP, strlen(SUCCOP), 0);
            return 1;
        }
    }

    // Can unsubscribe from a non-existing hashtag
    send(users[id].socket, SUCCOP, strlen(SUCCOP), 0);
    return 0;
}

/**
 * Prints out all stored tweets for the given user.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_timeline(User* users, int id) {
    char response[BUFFERSIZE];
    strcpy(response, "");

    // Build up all tweets
    for (int i = 0; i < numusers; i++) {
        for (int j = 0; j < users[i].numtwts; j++) {
            strcat(response, users[i].username);
            strcat(response, ": \"");
            strcat(response, users[i].tweets[j].message);
            strcat(response, "\" ");
            strcat(response, users[i].tweets[j].hashtag);
            strcat(response, "\n");
        }
    }
    
    // Send response to client
    send(users[id].socket, response, strlen(response), 0);
    return 1;
}

/**
 * Prints out all currently connected usernames.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_getusers(User* users, int id) {
    char response[BUFFERSIZE];
    strcpy(response, "");

    // Build up all usernames
    for (int i = 0; i < numusers; i++) {
        strcat(response, users[i].username);

        if (strcmp(users[i].username, "") != 0) {
            strcat(response, "\n");
        }
    }

    // Send response to client
    send(users[id].socket, response, strlen(response), 0);
    return 1;
}

/**
 * Prints out all stored tweets for the desired user.
 * Params: lists of users, specific user index.
 * Returns 1 on success and 0 on failure.
 */
int handle_gettweets(User* users, int id, char* username) {
    char response[BUFFERSIZE];
    strcpy(response, "");

    for (int i = 0; i < numusers; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // If username exists then build message to send to client
            for (int j = 0; j < users[i].numtwts; j++) {
                strcat(response, username);
                strcat(response, ": \"");
                strcat(response, users[i].tweets[j].message);
                strcat(response, "\" ");
                strcat(response, users[i].tweets[j].hashtag);
                strcat(response, "\n");
            }
            // Send response to client
            send(users[id].socket, response, strlen(response), 0);
            return 1;
        }
    }

    // No user error message
    strcat(response, "no user");
    strcat(response, username);
    strcat(response, "in the system\n\0");
    send(users[id].socket, response, strlen(response), 0);
    return 0;
}

/**
 * Allows the user to exit the server, cleans up all its memory.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_exit(User* users, int id) {
    // Send client "bye bye"
    send(users[id].socket, BYE, strlen(BYE), 0);

    // Clean up
    users[id].numsusbs = 0;
    users[id].numtwts = 0;
    users[id].socket = 0;
    for (int i = 0; i < MAXHASH; i++) {
        strcpy(users[id].subscriptions[i], "\0");
    }
    free(users[id].tweets);
    strcpy(users[id].username, "\0");
    numusers -= 1;
    return 1;
}

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
        // For each client read their request and send this along with 
        // relevant info for processing.
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
