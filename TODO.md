Programming Assignment 2
========================

Components
----------

### `ttweetcli`

sets up a TCP connection with the server and calls `client`

### `client`

begins a loop, each iteration receiving input from the user and
calling respective helper procedures below

### `tweet`, `subscribe`, `unsubscribe`, `timeline`, `getusers`, `gettweets`, `exit`

may take char * arguments if the respective command takes string
arguments

formats and sends a request to the server

receives and prints the server's response

### `ttweetser`

sets up a listening socket and necessary data structures

calls server when an incoming connection is received

### `server`

sets up a connection socket

begins a loop, each iteration receiving input from the client, parsing
it, and calling respective helper procedures below

### `tweet`, `subscribe`, `unsubscribe`, `getusers`, `gettweets`, `exit`

may take char * arguments if the respective command takes string
arguments

formats and sends a response to the client
