#include "ttweet.h"

/**
 * Client request processing functions...
 */
int check_alpha_num(char);
int parse_client_input(char*, int);
int handle_client_request(char*, char*, char*, int);

/**
 * Function for connection Client to given port and IP.
 * Params: Port number, IP string, username string.
 * Returns 0 if there's a problem with the connection, 1 on success.
 */
int network_connection(int port, char* ip, char* username) {
    struct sockaddr_in servAddr;
    char buffer[BUFFERSIZE] = {0};
    //memset(buffer, 0, sizeof(buffer));


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

            //TODO: add handling for messages over the buffer size
            //scanf("%s", buffer);

            fgets(buffer, BUFFERSIZE, stdin);

            parse_client_input(buffer, sock);

            send(sock, buffer, strlen(buffer), 0);
            fflush(stdin);
            memset(buffer, 0, sizeof(buffer));
        }
    }

    fflush(stdout);

    return 1;
}

/*  
    Take the clients inputted text and parse it into components delimited by spaces
    and ending with a new line character. Calls handle_client_request with the socket
    and the message components.

    Note: this function validates too many words and/or if the second word has more
    than 150 characters. It also checks hashtag structure and length.
    All other error checking is done in handle_client_request so
    that error handling specific to each command can be performed.
*/
int parse_client_input(char* buffer, int sock)
{
    int status = -1;

    int word_count = 0;

    char* command;
    int first_word_char_count = 0;

    char* second_word;
    int second_word_char_count = 0;
    
    char* third_word;
    int third_word_char_count;

    for (int i = 0; i < BUFFERSIZE; i++)
    {
        // Every time a space is reached, the word count and character counts get updated.
        if (buffer[i] == ' ')
        {
            ++word_count;
            if (word_count == 1)
            {
                first_word_char_count = i;
                //printf("1st word char count: %d\n", first_word_char_count);
            }
            else if (word_count == 2)
            {
                second_word_char_count = i - first_word_char_count;
                //printf("2nd word char count: %d\n", second_word_char_count);
            }
        }


        /* The newline character at the end of the user's inputted command is handled here.
           The character counts for the first, second, and third words are updated here as applicable.
           If this condition is reached, the for loop is exited. */
        if (buffer[i] == 0x0A && word_count <= 3)
        {
            ++word_count;
            if (word_count == 1)
            {
                first_word_char_count = i;
                //printf("1st word char count: %d\n", first_word_char_count);
            }
            else if (word_count == 2)
            {
                second_word_char_count = i - (first_word_char_count + 1);
                //printf("2nd word char count: %d\n", second_word_char_count);
            }
            else if (word_count == 3)
            {
                third_word_char_count = i - (first_word_char_count + second_word_char_count + 1);
                //printf("End line cond char count: %d\n", third_word_char_count);
            }
            else
            {
                printf("%s", WRONGPARAMS);
                return status;
            }
            break;
        }

        /* For tweets, the message is wrapped in quotes. This statement and for loop will
        search for matching quote at the end of the message. If not found, the status is returned
        as failed after the illegal message length error is returned to the console.*/
        if (buffer[i] == '"')
        {
            int start_quote_index = ++i;
            
            for (int j = start_quote_index; j < 152; j++)
            {
                if (j == 151)
                {
                    printf("%s", ILLMSGLEN);
                    return status;
                }

                if (buffer[j] == '"')
                {
                    i = j;
                    break;
                }
            }
        }

        // last ditch effort to catch when too many arguements are inputted
        if (word_count > 3)
        {
            printf("%s", WRONGPARAMS);
            return status;
        }
    }

    command = malloc((first_word_char_count + 1) * sizeof(char));
    if (command == NULL)
    {
        printf("%s\n", "Error allocating memory.");
    }

    // Extract the command string
    for (int j = 0; j < first_word_char_count; j++)
    {
        command[j] = buffer[j];
        //printf("Command[j] %c\n", command[j]);
    }

    if (word_count > 1)
    {       
        second_word = malloc(second_word_char_count * sizeof(char));
        if (second_word == NULL)
        {
            printf("%s\n", "Error allocating memory.");
        }

        int char_index_into_buffer = (first_word_char_count + 1);
        for (int k = 0; k < second_word_char_count; k++)
        {
            second_word[k] = buffer[char_index_into_buffer];
            // printf("second_word[k] %c\n", second_word[k]);
            // printf("buffer[char_count] %c\n", buffer[char_count]);
            char_index_into_buffer++;
        }

        if (word_count == 3)
        {
            if (third_word_char_count < 2 || third_word_char_count > MAXHASHLEN)
            {
                printf("%s\n", ILLHASH);
                free(command);
                free(second_word);
                return status;
            }

            third_word = malloc(third_word_char_count * sizeof(char));
            if (third_word == NULL)
            {
                printf("%s\n", "Error allocating memory.");
            }

            char_index_into_buffer = (second_word_char_count + first_word_char_count + 1);
            for (int m = 0; m < third_word_char_count; m++)
            {
                third_word[m] = buffer[char_index_into_buffer];
                // printf("third_word[m] %c\n", third_word[m]);
                // printf("buffer[char_count] %c\n", buffer[char_index_into_buffer]);
                char_index_into_buffer++;
            }

            if (third_word[0] != '#')
            {
                printf("%s\n", ILLHASH);
                free(command);
                free(second_word);
                free(third_word);
                return status;
            }

            for (int n = 0; n < third_word_char_count - 1; n++)
            {
                if (third_word[n] == '#')
                {
                    int p = n + 1;
                    if (!(check_alpha_num(third_word[p])))
                    {   
                        printf("%s\n", ILLHASH);
                        free(command);
                        free(second_word);
                        free(third_word);
                        return status;
                    }

                    while (p < third_word_char_count - 1)
                    {
                        if (third_word[p] == '#')
                        {
                            // exit this loop early since another hashtag unit has been detected
                            break;
                        }
                        if (!(check_alpha_num(third_word[p])))
                        {   
                            printf("%s\n", ILLHASH);
                            free(command);
                            free(second_word);
                            free(third_word);
                            return status;
                        }
                        p++;
                    }
                }
            }


        }
    }
    status = handle_client_request(command, second_word, third_word, sock);

    //TODO: free memory!
    return status;
}

int check_alpha_num(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return 1;
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        return 1;
    }
    else if (ch >= '0' && ch <= '9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int handle_client_request(char* command, char* second_word, char* third_word, int sock)
{
    int status = -1;

    if (strcmp(command, "tweet") == 0) // For some reason, #define tweet doesn't work here. Perhaps it's confusion with the stuct Tweet type
    {
        printf("tweet\n");            
    }
    else if (strcmp(command, SUBS) == 0)
    {
        printf("subscribe\n");
    }
    else if (strcmp(command, UNSUBS) == 0)
    {
        printf("unsubscribe\n");
    }
    else if (strcmp(command, TIMELINE) == 0)
    {
        printf("timeline\n");
    }
    else if (strcmp(command, GETUSERS) == 0)
    {
        printf("getusers\n");
    }
    else if (strcmp(command, GETTWEETS) == 0)
    {
        printf("gettweets\n");
    }
    else if (strcmp(command, EXIT) == 0)
    {
        printf("exit\n");
    }
    else
    {
        printf("%s", ILLHASH);
        return status;
    }
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
