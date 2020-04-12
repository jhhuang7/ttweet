#include "ttweet.h"

/**
 * Client request processing function declarations.
 */
int check_alpha_num(char);
int check_hashtag(char*, int);
int parse_client_input(char*, int);
int handle_client_request(char*, char*, char*, int, int, int, int);

/**
 * Estalish initial connection to server checking if username is ok.
 * Params: socket stuff to connect, username string to be checked.
 * Returns 0 if there's a problem, 1 on success.
 */
int check_user(int sock, char* username) {
    // Send message to server
    char connectmsg[BUFFERSIZE];
    strcpy(connectmsg, CONNCECTCODE);
    strcat(connectmsg, username);
    send(sock, connectmsg, strlen(connectmsg), 0);

    // Check if username is indeed valid
    char response[BUFFERSIZE] = {0};
    int status = INVALID;
    read(sock, response, BUFFERSIZE);
    if (strcmp(response, LOGIN) == 0) {
        status = VALID;
    }
    printf("%s", response);

    return status;
}

/**
 * Loops forever and prints out anything from the server.
 * This function is used for pthread_create.
 * Parameterss: a void pointer of arguments.
 * Returns: a void pointer (NULL).
 */
void* handle_response(void* arg) {
    Response* rsp = (Response*)arg;
    int sock = rsp->socket;
    char response[BUFFERSIZE] = {0};
    
    while (1) {
        if (read(sock, response, BUFFERSIZE) > 0)
        {
            printf("%s", response);
        }
        memset(response, 0, sizeof(response));
        
        // this sleep call causes the responses to be delayed significantly
        //sleep(2);
    }
    
    pthread_exit(NULL);
    return NULL; 
}

/**
 * Function for connection Client to given port and IP.
 * Params: Port number, IP string, username string.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port, char* ip, char* username) {
	struct sockaddr_in serverAddr;
    char buffer[BUFFERSIZE] = {0}; 
    int sock;
    int con;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("%s", CONER); 
        return 0;
	}

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ip);
    if (serverAddr.sin_addr.s_addr <= 0) { 
        printf("%s", INVSERIP); 
        return 0;
    } 

	con = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (con < 0){
		printf("%s", CONER);
        return 0;
	}

    // Check if given username is already connected.
    if (!check_user(sock, username)) {
        return 0;
    }

    //Handle forever printing out server messages
    pthread_t threadId;
    Response rsp;
    rsp.socket = sock;
    pthread_create(&threadId, NULL, handle_response, &rsp);

    //this sleep call causes the server connection to close for some reason
    //sleep(2);

	while (1) {


        fgets(buffer, BUFFERSIZE, stdin);
        
        // this sleep call causes the server connection to close for some reason
        //sleep(1);

        if (parse_client_input(buffer, sock) == VALID + VALID) {
            // Exit program is client wants to exit
            close(sock);
            break;
        }

        memset(buffer, 0, sizeof(buffer));
        fflush(stdin);
        fflush(stdout);

        //this sleep call changes the client output order but not in any useful way
        //sleep(1);
	}

    return 1;
}

/**  
 * Parse_client_input.
 * Take the clients inputted text and parse it into components delimited by 
 * spaces and ending with a new line character. Calls handle_client_request 
 * with the socket and the message components.
 *
 * Note: this function validates too many words and/or if the second word has 
 * more than 150 characters. It also checks hashtag structure and length.
 * All other error checking is done in handle_client_request so
 * that error handling specific to each command can be performed.
 */
int parse_client_input(char* buffer, int sock) {
    int word_count = 0;

    char* command;
    int first_word_char_count = 0;

    char second_word[BUFFERSIZE];
    int second_word_char_count = 0;
    
    char* third_word;
    int third_word_char_count = 0;

    for (int i = 0; i < BUFFERSIZE; i++) {
        // Every time a space is reached, the word character counts get updated.
        if (buffer[i] == ' ') {
            ++word_count;
            if (word_count == 1) {
                first_word_char_count = i;
            } else if (word_count == 2) {
                second_word_char_count = i - (first_word_char_count + 1);
            }
        }

        // The newline character at the end of the user's inputted command is 
        // handled here. The character counts for the first, second, and third 
        // words are updated here as applicable.
        // If this condition is reached, the for loop is exited.
        if (buffer[i] == 0x0A && word_count <= 3) {
            ++word_count;
            if (word_count == 1) {
                first_word_char_count = i;
            } else if (word_count == 2) {
                second_word_char_count = i - (first_word_char_count + 1);
            } else if (word_count == 3) {
                third_word_char_count = i - 
                    (first_word_char_count + second_word_char_count + 2);
            } else {
                printf("%s", WRONGPARAMS);
                return INVALID;
            }
            break;
        }

        // For tweets, the message is wrapped in quotes. 
        // This statement and for loop will search for matching quote at the 
        // end of the message. If not found, the status is returned
        // as failed after the illegal message length error is returned.
        if (buffer[i] == '"') {
            int start_quote_index = ++i;
            
            for (int j = start_quote_index; j < strlen(buffer); j++) {
                if (buffer[j] == '"') {
                    i = j;
                    break;
                }
            }
        }

        // last ditch effort to catch when too many arguements are inputted
        if (word_count > 3) {
            printf("%s", WRONGPARAMS);
            return INVALID;
        }
    }

    command = calloc((first_word_char_count + 1), sizeof(char));
    if (command == NULL) {
        printf("%s\n", WRONGPARAMS);
    }

    // Extract the command string
    for (int j = 0; j < first_word_char_count; j++) {
        command[j] = buffer[j];
    }

    // Extract the second (and third if needed) argument provided by the user
    if (word_count > 1) {       
        int char_index_into_buffer = (first_word_char_count + 1);
        int k;
        for (k = 0; k < second_word_char_count; k++) {
            second_word[k] = buffer[char_index_into_buffer];
            char_index_into_buffer++;
        }
        second_word[k] = '\0';

        if (strlen(second_word) > MSGMAX + 2) {
            printf("%s", ILLMSGLEN);
            return INVALID;
        }

        if (word_count == 3) {
            third_word = calloc(third_word_char_count, sizeof(char));
            if (third_word == NULL) {
                printf("%s\n", WRONGPARAMS);
            }

            char_index_into_buffer = 
                (second_word_char_count + first_word_char_count + 2);

            for (int m = 0; m < third_word_char_count; m++) {
                third_word[m] = buffer[char_index_into_buffer];
                char_index_into_buffer++;
            }
        }
    }

    int status = handle_client_request(command, second_word, third_word, 
        second_word_char_count, third_word_char_count, sock, word_count);

    // char response[BUFFERSIZE] = {0};
    // memset(response, 0, sizeof(BUFFERSIZE));
    // if (read(sock, response, BUFFERSIZE) > 0)
    // {
    //     printf("%s", response);
    // }
    fflush(stdout);

    switch (word_count) {
        case 1:
            free(command);
            break;
        case 2:
            free(command);
            break;
        case 3:
            free(command);
            free(third_word);
            break;
    }

    return status;
}

/** 
 * Checks the client command and args to ensure they are valid. 
 * Returns 1 on valid and 0 on invalid.
 * Calling method should free the memory.
 */
int handle_client_request(char* command, char* second_word, char* third_word, 
        int second_word_size, int third_word_size, int sock, int word_count) {
    char send_msg[BUFFERSIZE] = {0};
    memset(send_msg, 0, sizeof(send_msg));

    if (strcmp(command, "tweet") == 0) { // "#define TWT" doesn't work here
        if (word_count == 3 && strcmp(second_word, "\"\"") != 0 
                && check_hashtag(third_word, third_word_size)) {
            strcpy(send_msg, TWTCODE);
            strcat(send_msg, second_word);
            strcat(send_msg, third_word);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, SUBS) == 0) {
        if (word_count == 2 && check_hashtag(second_word, second_word_size)) {
            strcpy(send_msg, SUBSCODE);
            strcat(send_msg, second_word);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, UNSUBS) == 0) {
        if (word_count == 2 && check_hashtag(second_word, second_word_size)) {
            strcpy(send_msg, UNSCODE);
            strcat(send_msg, second_word);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, TIMELINE) == 0) {
        if (word_count == 1) {
            strcpy(send_msg, TIMECODE);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, GETUSERS) == 0) {
        if (word_count == 1) {
            strcpy(send_msg, GTUSRCODE);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, GETTWEETS) == 0) {
        if (word_count == 2) {
            strcpy(send_msg, GTTWTCODE);
            strcat(send_msg, second_word);

            send(sock, send_msg, strlen(send_msg), 0);
            return VALID;
        }
    } else if (strcmp(command, EXIT) == 0) {
        if (word_count == 1) {
            strcpy(send_msg, EXITCODE);
            
            send(sock, send_msg, strlen(send_msg), 0);
            printf("%s", BYE); // Need this read for exit message
            return VALID + VALID;
        }
    }
    // else { Command invalid }
    strcpy(send_msg, MSGNONE);
    send(sock, send_msg, strlen(send_msg), 0);
    return INVALID;
}

/** 
 * Check_alpha_num.
 * Checks if character is A-Z, a-z, or 0-9.
 * Returns 1 on valid and 0 on invalid.
 */
int check_alpha_num(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return VALID;
    } else if (ch >= 'a' && ch <= 'z') {
        return VALID;
    } else if (ch >= '0' && ch <= '9') {
        return VALID;
    } else {
        return INVALID;
    }
}

/** 
 * Check_hashtag.
 * Checks the validity of the hashtag. Returns 1 on valid and 0 on invalid.
 * Calling method should free the memory.
 */
int check_hashtag(char* word, int size) {
    if (size < 2 || size > MAXHASHLEN) {
        printf("%s", ILLHASH);
        return INVALID;
    }

    if (word[0] != '#' || (!check_alpha_num(word[size - 1]))) {
        printf("%s", ILLHASH);
        return INVALID;
    }

    for (int n = 0; n < size - 1; n++) {
        if (word[n] == '#') {
            int p = n + 1;
            if (!(check_alpha_num(word[p]))) {   
                printf("%s", ILLHASH);
                return INVALID;
            }

            while (p < size - 1) {
                if (word[p] == '#') {
                    // exit inner loop early as another hashtag has been found
                    break;
                }

                if (!(check_alpha_num(word[p]))) {   
                    printf("%s", ILLHASH);
                    return INVALID;
                }

                p++;
            }
        }
    }

    return VALID;
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

    // Get and check range of the IP (IP is checked again when connecting)
    char* ip = argv[1];
    if (ip[0] - '2' >= 0) {
        printf("%s", INVSERIP);
        return 0;
    }

    // Check if valid port
    char* port = argv[2];
    char* porterr;
    int portnum = strtol(port, &porterr, 10);

    if (portnum <= MIN + 80 ||portnum > MAXPORT || strcmp(porterr, "") != 0) {
        printf("%s", INVSERPORT);sleep(0.5);
        return 0;
    }

    // Check if valid username (username existence is checked when connecting)
    char* username;
    username = argv[3];
    if (strcmp(username, "") == 0) {
        printf("%s", INVUSER);
        return 0;
    }
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
    signal(SIGPIPE, SIG_IGN);

    int args = check_args(argc, argv);
    if (!args) {
        return 0;
    }

    return 1;   
}
