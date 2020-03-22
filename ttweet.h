// Hash includes
#include <unistd.h>
#include <ctype.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <pthread.h>

// Hash defines
#define MIN        0
#define MAXHASH    3
#define MAXCONNS   5
#define MAXHASHLEN 15
#define MSGMAX     150
#define BUFFERSIZE 10000
#define MAXPORT    65535
#define HASHTAG    '#'
#define ALL        "#ALL"

// Error Messages 
#define SARGE       "error: args should contain <ServerPort>\n"
#define SERCON      "server get connection!\n"
#define CARGE       "error: args should contain <ServerIP> <ServerPort> <Username>\n"
#define CONER       "connection error, please check your server: Connection refused\n"
#define INVSERIP    "server ip invalid, connection refused.\n"
#define INVSERPORT  "server port invalid, connection refused.\n"
#define INVUSER     "username has wrong format, connection refused.\n"
#define LOGGEDIN    "username illegal, connection refused.\n"
#define WRONGPARAMS "wrong number of parameters, connection refused.\n"
#define ILLMSGLEN   "message length illegal, connection refused.\n"
#define MSGNONE     "message format illegal.\n"
#define ILLHASH     "hashtag illegal format, connection refused.\n"
#define LOGIN       "username legal, connection established.\n"

// Success Messages
#define BYE    "bye bye\n"
#define SUCCOP "operation success\n"

// Valid Commands
#define TWEET     "tweet​"
#define SUBS      "subscribe"
#define UNSUBS    "unsubscribe"
#define TIMELINE  "timeline"
#define GETUSERS  "getusers"
#define GETTWEETS "gettweets"
#define EXIT      "exit"

// Global variables
int numusers = 0;

// Structs
typedef struct {
    char* message;
    char* hashtag;
} Tweet;

typedef struct {
    int socket;
    char* username;
    int numsusbs;
    char* subscriptions[MAXHASH];
    int numtwts;
    Tweet* tweets;
} User;
