#include "ttweet.h"

/**
 * Client request processing functions...
 */
int handle_client_requests(char*, int);
int parse_client_input(char*);

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

            //handle_client_requests(buffer, sock);
            parse_client_input(buffer);

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
    and ending with a new line character. 
*/
int parse_client_input(char* buffer)
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
        //printf("Buffer[i] %c\n", buffer[i]);
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
                second_word_char_count = i - first_word_char_count;
                //printf("2nd word char count: %d\n", second_word_char_count);
            }
            else if (word_count == 3)
            {
                third_word_char_count = i - second_word_char_count;
                //printf("End line cond char count: %d\n", third_word_char_count);
            }
            else
            {
                printf("%s", WRONGPARAMS);
                return status;
            }
            break;
        }

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

    return status;
}

int handle_client_requests(char* buffer, int sock)
{
    int command_size = 0;
    // get the inital command which should be no longer then 12 characters
    for (int i = 0; i < 12; i++)
    {
        // if buffer[i] is a space or carriage return (Enter key), stop copy
        if (buffer[i] == ' ' || buffer[i] == 0x0A)
        {
            printf("Break condition: %d", command_size);
            command_size++;
            break;
        }
        command_size++;
    }

    if (command_size == 0)
    {
        printf("Command size is zero%s", ILLHASH);
    }

    char *command = malloc(sizeof(char) * command_size);

    printf("Command size: %ld", sizeof(command));

    for (int j = 0; j < command_size; j++)
    {
        command[j] = buffer[j];
    }

    // printf("BEGIN");
    // for (int k = 0; k < sizeof(command); k++)
    // {
    //     printf("\n");
    //     printf("%c", command[k]);
    //             printf("\n");
    // }
    // printf("END");

    if (strcmp(command, "tweet") == 0)
    {
        //todo TWEET
        printf("tweet");            
    }
    else if (strcmp(command, "subscribe") == 0)
    {
        //todo subscribe #hashtag
        printf("subscribe");
    }
    else if (strcmp(command, "unsubscribe") == 0)
    {
        //todo unsubscribe #hashtag                
        printf("unsubscribe");
    }
    else if (strcmp(command, "timeline") == 0)
    {
        printf("timeline");
    }
    else if (strcmp(command, "getusers") == 0)
    {
        printf("getusers");
    }
    else if (strcmp(command, "gettweets") == 0)
    {
        printf("gettweets");
    }
    else if (strcmp(command, "exit") == 0)
    {
        if (strcmp(buffer, "exit"))
        {
            printf("SPLOSH %s", ILLHASH);
        }
        else
        {
            printf("exit");
            send(sock, command, strlen(command), 0);
            fflush(stdin);
            free(command);
            //memset(command, 0, sizeof(command));
        }
    }
    else
    {
        printf("%s", ILLHASH);
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
