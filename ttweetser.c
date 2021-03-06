#include "ttweet.h"
    
/**
 * Stores a user's tweet and sends tweet to all other users who have subscribed
 * to the attached hashtag.
 * Params: lists of users, specific user index, tweet string, list of hashtags,
 * number of hashtags.
 * Returns 1.
 */
int handle_tweet(int sock, User* users, int id, char* tweet, 
        char** hashtag, int hashes) {
    // Increase size of message lists if limit is reached
    if (users[id].numtwts > 0 && users[id].numtwts % BUFFERSIZE == 0) {
        users[id].tweets = (char**)
            realloc(users[id].tweets, sizeof(users[id].tweets) * BUFFERSIZE);
    }
    if (numtweets > 0 && numtweets % BUFFERSIZE == 0) {
        tweets = (Tweet*)realloc(tweets, sizeof(tweets) * BUFFERSIZE);
    }
    
    // Store the tweet with user and globally
    strcpy(users[id].tweets[users[id].numtwts], tweet);
    strcat(users[id].tweets[users[id].numtwts], " ");
    for (int i = 0; i < hashes; i++) {
        strcat(users[id].tweets[users[id].numtwts], HASHTAG);
        strcat(users[id].tweets[users[id].numtwts], hashtag[i]);
    }
    strcpy(tweets[numtweets].username, users[id].username);
    strcpy(tweets[numtweets].message, users[id].tweets[users[id].numtwts]);
    users[id].numtwts += 1;
    numtweets += 1;
    send(sock, NOFEEDBACK, strlen(NOFEEDBACK), 0);

    // Deliver to all the clients who are subscribed to the hashtag
    char response[BUFFERSIZE];
    strcpy(response, users[id].username);
    strcat(response, " ");
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
                }
            }
        }
    }

    return 1;
}

/**
 * Stores a hashtag subscription for a user.
 * Params: lists of users, specific user index, hashtag string.
 * Returns 1 or success and 0 on failure.
 */
int handle_subscribe(int sock, User* users, int id, char* hashtag) {
    // Max hashtag limit reached
    char response[BUFFERSIZE];
    strcpy(response, "");
    strcat(response, "operation failed: sub ");
    strcat(response, hashtag);
    strcat(response, " failed, already exists or exceeds 3 limitation\n");
    
    if (users[id].numsusbs >= MAXHASH) {
        send(sock, response, strlen(response), 0);
        return 0;
    }

    // Store hashtag with user
    int space;
    for (int i = 0; i < MAXHASH; i++) {
        if (strcmp(users[id].subscriptions[i], "") == 0) {
            space = i;
        } else if (strcmp(users[id].subscriptions[i], hashtag) == 0) {
            send(sock, response, strlen(response), 0);
            return 0;
        }
    }
    strcpy(users[id].subscriptions[space], hashtag);
    users[id].numsusbs += 1;
    send(sock, SUCCOP, strlen(SUCCOP), 0);
    return 1;
}

/**
 * Removes a hashtag subscription for a user.
 * Params: lists of users, specific user index, hashtag string.
 * Returns 1 on success and 0 on failure.
 */
int handle_unsubscribe(int sock, User* users, int id, char* hashtag) {
    // If the hashtag is #ALL then remove all subscriptions
    if (strcmp(hashtag, ALL) == 0) {
        for (int i = 0; i < MAXHASH; i++) {
            strcpy(users[id].subscriptions[i], "");
        }
        users[id].numsusbs = 0;
        send(sock, SUCCOP, strlen(SUCCOP), 0);
        return 1;
    }

    // Remove specific hashtag
    for (int i = 0; i < MAXHASH; i++) {
        if (strcmp(users[id].subscriptions[i], hashtag) == 0) {
            strcpy(users[id].subscriptions[i], "");
            users[id].numsusbs -= 1;
            send(sock, SUCCOP, strlen(SUCCOP), 0);
            return 1;
        }
    }

    // Can unsubscribe from a non-existing hashtag
    send(sock, SUCCOP, strlen(SUCCOP), 0);
    return 0;
}

/**
 * Prints out all stored tweets for the given user.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_timeline(int sock, User* users, int id) {
    char response[BUFFERSIZE];
    strcpy(response, "");

    // Build up all tweets
    for (int i = 0; i < numtweets; i++) {
        if (strcmp(tweets[i].username, "") != 0) {
            strcat(response, tweets[i].username);
            strcat(response, ": ");
            strcat(response, tweets[i].message);
            strcat(response, "\n");
        }
    }
    
    // Send response to client
    send(sock, response, strlen(response), 0);
    return 1;
}

/**
 * Prints out all currently connected usernames.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_getusers(int sock, User* users, int id) {
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
    send(sock, response, strlen(response), 0);
    return 1;
}

/**
 * Prints out all stored tweets for the desired user.
 * Params: lists of users, specific user index.
 * Returns 1 on success and 0 on failure.
 */
int handle_gettweets(int sock, User* users, int id, char* username) {
    char response[BUFFERSIZE];
    strcpy(response, "");

    for (int i = 0; i < MAXCONNS; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // If username exists then build message to send to client
            for (int j = 0; j < users[i].numtwts; j++) {
                strcat(response, username);
                strcat(response, ": ");
                strcat(response, users[i].tweets[j]);
                strcat(response, "\n");
            }
            // Send response to client
            send(sock, response, strlen(response), 0);
            return 1;
        }
    }

    // No user error message
    strcat(response, "no user ");
    strcat(response, username);
    strcat(response, " in the system\n");
    send(sock, response, strlen(response), 0);
    return 0;
}

/**
 * Allows the user to exit the server, cleans up all its memory.
 * Params: lists of users, specific user index.
 * Returns 1.
 */
int handle_exit(int sock, User* users, int id) {
    // Client will automatically print "bye bye"

    // Clean up on server side
    for (int j = 0; j < numtweets; j++) {
        if (strcmp(users[id].username, tweets[j].username) == 0) {
            strcpy(tweets[j].username, "");
            strcpy(tweets[j].message, "");
        }
    }

    for (int i = 0; i < MAXHASH; i++) {
        strcpy(users[id].subscriptions[i], "");
    }
    for (int k = 0; k < BUFFERSIZE; k++) {
        strcpy(users[id].tweets[k], "");
    }
    strcpy(users[id].username, "");
    users[id].numsusbs = 0;
    users[id].numtwts = 0;
    users[id].socket = 0;

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
    if (status == -1 && numusers < MAXCONNS) {
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
        return handle_tweet(sock, users, id, first, second, hashes);
    } else if (strcmp(command, SUBSCODE) == 0) {
        return handle_subscribe(sock, users, id, first);
    } else if (strcmp(command, UNSCODE) == 0) {
        return handle_unsubscribe(sock, users, id, first);
    } else if (strcmp(command, TIMECODE) == 0) {
        return handle_timeline(sock, users, id);
    } else if (strcmp(command, GTUSRCODE) == 0) {
        return handle_getusers(sock, users, id);
    } else if (strcmp(command, GTTWTCODE) == 0) {
        return handle_gettweets(sock, users, id, first);
    } else if (strcmp(command, EXITCODE) == 0) {
        return handle_exit(sock, users, id);
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
 * Function for handling multiple connections in threads.
 * Parameters: void pointer arguments.
 * Returns: a void pointer.
 */
void* new_connection(void* arg) {
    Connectinfo* connectinfo = (Connectinfo*)arg;
    int sock = connectinfo->socket;

    char buffer[BUFFERSIZE]; 
    char response[BUFFERSIZE];
    char username[BUFFERSIZE];

    while (recv(sock, buffer, BUFFERSIZE - 1, 0) > 0) {
        if (strlen(buffer) != 0) {
            printf("Client %s requested: %s\n", username, buffer);
        }

        pthread_mutex_lock(&lock);
        if (strncmp(buffer, CONNCECTCODE, 2) == 0 ) {
            if (!user_connection(connectinfo->users, username, buffer, sock)) {
                memset(username, 0, sizeof(username));
            }
        } else if (strcmp(buffer, MSGNONE) == 0) {
            strcpy(response, MSGNONE);
            send(sock, response, strlen(response), 0);
        } else {
            parse_buffer(connectinfo->users, buffer, username, sock);
        }
        pthread_mutex_unlock(&lock);

        fflush(stdout);
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));
    }

    pthread_exit(NULL);
    return NULL;
}

/**
 * Function for connection Server to given port forever.
 * Params: Port number.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port) {    
	struct sockaddr_in serverAddr;
    int serverfd;
    int con;
	struct sockaddr_in newAddr;
    socklen_t addr_size;
    int sock;

	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd < 0) {
        printf("%s", CONER);
		return 0;
	}

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	con = bind(serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (con < 0) {
		printf("%s", CONER);
		return 0;
	}

	if (listen(serverfd, 100) != 0) {
		printf("%s", CONER);
		return 0;
	}

    // Initialize list of users for storage once inputs are valid
	User* users = (User*)malloc(sizeof(User) * MAXCONNS);
	for (int i = 0; i < MAXCONNS; i++) {
		users[i].socket = 0;
		users[i].username = malloc(sizeof(char) * BUFFERSIZE);
		users[i].numsusbs = 0;
		for (int j = 0; j < MAXHASH; j++) {
			users[i].subscriptions[j] = malloc(sizeof(char) * BUFFERSIZE);
		}
		users[i].numtwts = 0;
		users[i].tweets = malloc(sizeof(char*) * BUFFERSIZE);
		for (int k = 0; k < BUFFERSIZE; k++) {
			users[i].tweets[k] = malloc(sizeof(char) * BUFFERSIZE);
		}
	}
	tweets = (Tweet*)malloc(sizeof(Tweet) * BUFFERSIZE);
	for (int a = 0; a < BUFFERSIZE; a++) {
		tweets[a].username = malloc(sizeof(char) * BUFFERSIZE);
		tweets[a].message = malloc(sizeof(char) * BUFFERSIZE);
    }

    // Connected
    printf("%s", SERCON); 
    pthread_t pid;
    Connectinfo connectinfo;

	while ((sock = accept(serverfd, (struct sockaddr*)&newAddr, &addr_size))) {
		connectinfo.socket = sock;
        connectinfo.users = users;
        
		if (sock < 0) {
			printf("%s", CONER);
		    return 0;
		} else {
            // Use threads to handle multiple clients
            pthread_create(&pid, NULL, new_connection, &connectinfo);
        }
	}

	// Free memory if an exit happens
	for (int i = 0; i < MAXCONNS; i++) {
		free(users[i].username);
		for (int j = 0; j < MAXHASH; j++) {
			free(users[i].subscriptions[j]);
        }
		for (int k = 0; k < BUFFERSIZE; k++) {
			free(users[i].tweets[k]);
        }
		free(users[i].tweets);
	}
	free(users);
	for (int x = 0; x < BUFFERSIZE; x++) {
		free(tweets[x].username);
		free(tweets[x].message);
	}
	free(tweets);
    
	// Close up socket
	close(serverfd);
    close(sock);
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
    signal(SIGPIPE, SIG_IGN);

    int args = check_args(argc, argv);
    if (!args) {
        return 0;
    }

    return 1;
}
