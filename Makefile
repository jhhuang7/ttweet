CFLAGS = -Wall -pedantic -std=gnu99 -g -pthread
CC = gcc

all: ttweetser ttweetcli

ttweetser: ttweetser.c

ttweetcli: ttweetcli.c

clean: rm ttweetser ttweetcli
