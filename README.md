# CS 3251 Computer Networks, April 7 2020, Programming Assignment 2

## PA2Group 49:
- Juhua Huang (903572368), jhuang499@gatech.edu
- Lorin Achey (903089055), lorin.achey@gatech.edu
- Patrick Coppock (GTID), pcoppock3@gatech.edu

## Division of Work:
- Initial arg checking: Juhua
- Set up TCP connection on client side: Lorin
- Process client requests: Lorin
- Multi-threading on server to handle multiple clients: Patrick
- Process requests from client on server side: Juhua

## Descriptions of all files submitted:
- ttweetser.c: C source code for server
- ttweetcli.c: C source code for client
- ttweet.h: Header file used for ttweetser.c and ttweetcli.c
- Makefile: File to be run with "make" to convert the source code into binary

## Protocol description of the messages exchanged between Client and Server:
- The client will read from stdin a maximum of 3 strings. The first string is 
the command and the reading two strings are specific to the command.
- Based on the command, the client will then send a string buffer to the server. 
The string buffer will consist of a command code (first two characters) with the 
second and third strings read from stdin appended onto the end.
- Command codes: -w for tweet​; -s for subscribe; -n for unsubscribe; 
-i for timeline; -u for getusers; -t for gettweets; -e for exit
- Once the sever receives the string buffer from the client, it will parse the 
buffer and split the buffer into 3 strings. Based on the first string 
(command code), the server will then use the second and third strings to 
complete the given client request.
- Once client requests are completed on the server side with info stored/ 
cleared from memory done, it will send back a response to the client. 
The client simply receives this response and prints it to stdout.

## Instructions for compiling and running client and server programs:
- Run "make" in terminal of the file directory
- Use "./ttweetser ServerPort" to invoke server through command line
- Use "./ttweetcli ServerIP ServerPort Username" to invoke client
- No dependent packages or any special instructions needed to test code
