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
        strcat(users[id].tweets[users[id].numtwts].hashtag, HASHTAG);
        strcat(users[id].tweets[users[id].numtwts].hashtag, hashtag[i]);
    }
    users[id].numtwts += 1;
    send(users[id].socket, NOFEEDBACK, strlen(NOFEEDBACK), 0);

    // Deliver to all the clients who are subscribed to the hashtag
    char response[BUFFERSIZE];
    strcpy(response, "");
    strcat(response, tweet);
    strcat(response, " ");
    for (int i = 0; i < hashes; i++) {
        strcat(response, HASHTAG);
        strcat(response, hashtag[i]);
    }
    strcat(response, "\n");

    for (int i = 0; i < MAXCONNS; i++) {
        for (int j = 0; j < MAXHASH; j++) {
            for (int k = 0; k < hashes; k++) {
                char tag[BUFFERSIZE];
                strcpy(tag, HASHTAG);
                strcat(tag, hashtag[k]);
                if (strcmp(users[i].subscriptions[j], tag) == 0 ||
                        strcmp(users[i].subscriptions[j], ALL) == 0) {
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
    char response[BUFFERSIZE];
    strcpy(response, "");
    strcat(response, "operation failed: sub ");
    strcat(response, hashtag);
    strcat(response, " failed, already exists or exceeds 3 limitation\n");
    
    if (users[id].numsusbs >= MAXHASH) {
        send(users[id].socket, response, strlen(response), 0);
        return 0;
    }

    // Store hashtag with user
    for (int i = 0; i < MAXHASH; i++) {
        if (strcmp(users[id].subscriptions[i], "") == 0) {
            strcpy(users[id].subscriptions[i], hashtag);
            break;
        } else if (strcmp(users[id].subscriptions[i], hashtag) == 0) {
            send(users[id].socket, response, strlen(response), 0);
            return 0;
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
            strcat(response, ": ");
            strcat(response, users[i].tweets[j].message);
            strcat(response, " ");
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
    for (int i = 0; i < MAXCONNS; i++) {
        if (strcmp(users[i].username, "") != 0) {
            strcat(response, users[i].username);
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

    for (int i = 0; i < MAXCONNS; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // If username exists then build message to send to client
            for (int j = 0; j < users[i].numtwts; j++) {
                strcat(response, username);
                strcat(response, ": ");
                strcat(response, users[i].tweets[j].message);
                strcat(response, " ");
                strcat(response, users[i].tweets[j].hashtag);
                strcat(response, "\n");
            }
            // Send response to client
            send(users[id].socket, response, strlen(response), 0);
            return 1;
        }
    }

    // No user error message
    strcat(response, "no user ");
    strcat(response, username);
    strcat(response, " in the system\n");
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
    users[id].tweets = malloc(sizeof(Tweet) * BUFFERSIZE);
    strcpy(users[id].username, "\0");
    numusers -= 1;
    return 1;
}

/**
 * Gets the id position of the user in the users list given username.
 * This function can also be used to check if a given user is connected or not.
 * Params: users list, username string.
 * Return the int position of user in list.
 */
int get_user_id(User* users, char* username) {
    int id = -1;

    for (int i = 0; i < MAXCONNS; i++) {
        if (strcmp(username, users[i].username) == 0) {
            id = i;
            break;
        }
    }

    return id;
}

/**
 * Handles initial user connection.
 * Params: users list, username to be change, buffer socket info of client.
 * Returns 1 on success and 0 on failure.
 */
int user_connection(User* users, char* username, char* buffer, int socket) {
    char name[BUFFERSIZE];
    strncpy(name, buffer + 2, strlen(buffer) - 2);

    int status = get_user_id(users, name);
    int id;
    if (status == -1 && numusers <= MAXCONNS) {
        strcpy(username, name);

        for (int i = 0; i < MAXCONNS; i++) {
            if (strcmp(users[i].username, "") == 0) {
                users[i].socket = socket;
                strcpy(users[i].username, username);
                id = i;
                numusers += 1;
                break;
            }
        }

        send(users[id].socket, LOGIN, strlen(LOGIN), 0);
    } else {
        send(socket, LOGGEDIN, strlen(LOGGEDIN), 0);
        return 0;
    }

    return 1;
}

/**
 * Completes the given client requests by calling relevant functions based on 
 * given command.
 * Params: users list, username string, command code string, 
 * first and second strings follwoing command code.
 * Returns 1 on success and 0 on failure.
 */
int complete_request(User* users, char* username, char* command, 
        char* first, char** second, int hashes, int sock) {
    int id = get_user_id(users, username);
    users[id].socket = sock;

    if (strcmp(command, TWTCODE) == 0) {
        return handle_tweet(users, id, first, second, hashes);
    } else if (strcmp(command, SUBSCODE) == 0) {
        return handle_subscribe(users, id, first);
    } else if (strcmp(command, UNSCODE) == 0) {
        return handle_unsubscribe(users, id, first);
    } else if (strcmp(command, TIMECODE) == 0) {
        return handle_timeline(users, id);
    } else if (strcmp(command, GTUSRCODE) == 0) {
        return handle_getusers(users, id);
    } else if (strcmp(command, GTTWTCODE) == 0) {
        return handle_gettweets(users, id, first);
    } else if (strcmp(command, EXITCODE) == 0) {
        return handle_exit(users, id);
    }

    return 0;
}

/** 
 * Parses the read buffer from client.
 * Params users list, string buffer, socket fd.
 * Returns 0 if there's a problem, 1 on success.
 */
int parse_buffer(User* users, char* buffer, char* username, int sock) {
    char cmd[BUFFERSIZE];
    strncpy(cmd, buffer, 2);

    char first[BUFFERSIZE];
    int len = 0;
    int index = 2;
    char** second = malloc(sizeof(char*) * BUFFERSIZE);
    int hashes = 0;
    if (strcmp(cmd, TWTCODE) != 0) {
        int i;
        for (i = 2; i < strlen(buffer); i++) {
            first[i - 2] = buffer[i];
        }
        first[i - 2] = '\0';
    } else {
        first[len++] = buffer[index++];
        while (1) {
            if (buffer[index] == '"') {
                first[len++] = buffer[index++];
                break;
            }

            first[len++] = buffer[index++];
        }
        first[len] = '\0';

        char rest[BUFFERSIZE] = {0};
        strncpy(rest, buffer + 2 + len, strlen(buffer) - 2 - len);
        strcat(rest, "\0");

        char* hash = strtok(rest, HASHTAG);
        while (hash != NULL) {
            second[hashes++] = hash;
            hash = strtok(NULL, HASHTAG);
        }
    }

    return complete_request(users, username, cmd, first, second, hashes, sock);
}

/**
 * Function for connection Server to given port forever.
 * Params: Port number.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port, User* users) {
    int serverfd; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    int sock = 0;
    char buffer[BUFFERSIZE] = {0}; 
    char response[BUFFERSIZE] = {0};
    char username[BUFFERSIZE] = {0}; 
   
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

    printf("%s", SERCON); // connected

    while (1) {
        sock = accept(serverfd, (struct sockaddr*)&address, 
            (socklen_t*)&addrlen);

        if (sock >= 0) {
            read(sock, buffer, BUFFERSIZE);
            printf("Client requested: %s\n", buffer);

            if (strncmp(buffer, CONNCECTCODE, 2) == 0 ) {
                user_connection(users, username, buffer, sock);
            } else if (strcmp(buffer, MSGNONE) == 0) {
                strcpy(response, MSGNONE);
                send(sock, response, strlen(response), 0);
            } else {
                parse_buffer(users, buffer, username, sock);
            }

            fflush(stdout);
            memset(buffer, 0, sizeof(buffer));
            memset(response, 0, sizeof(response));
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

    // Initialize list of users for storage
    User* users = malloc(sizeof(User) * MAXCONNS);
    for (int i = 0; i < MAXCONNS; i++) {
        users[i].socket = 0;
        users[i].username = malloc(sizeof(char) * BUFFERSIZE);
        users[i].numsusbs = 0;
        for (int j = 0; j < MAXHASH; j++) {
            users[i].subscriptions[j] = malloc(sizeof(char) * BUFFERSIZE);
        }
        users[i].numtwts = 0;
        users[i].tweets = malloc(sizeof(Tweet) * BUFFERSIZE);
    }

    // Server running forever
    return network_connection(portnum, users);
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
