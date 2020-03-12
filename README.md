CS 3251 Computer Networks, April 7 2020, Programming Assignment 2

PA2Group 49
-Juhua Huang (903572368), jhuang499@gatech.edu
-Lorin Achey (GTID), lorin.achey@gatech.edu
-Patrick Coppock (GTID), pcoppock3@gatech.edu

Division of Work:
-Initial arg checking: Juhua
-Set up TCP connection on client side: Lorin
-Process client requests: Lorin
-Multi-threading on server to handle multiple clients: Patrick
-Process requests from client on server side: Juhua

Descriptions of all files submitted:
-ttweetser.c: C source code for server
-ttweetcli.c: C source code for client
-ttweet.h: Header file used for ttweetser.c and ttweetcli.c
-Makefile: File to be run with "make" to convert the source code into binary

Protocol description of the messages exchanged between Client and Server:
-Protocol

Instructions for compiling and running client and server programs:
-Run "make" in terminal of the file directory
-Use "./ttweetser <ServerPort>" to invoke server through command line
-Use "./ttweetcli <ServerIP> <ServerPort> <Username>" to invoke client
-No dependent packages or any special instructions needed to test code
