CFLAGS = -Wall -pedantic -std=gnu99 -g -pthread
CC = gcc

all: ttweetser ttweetcli

ttweetser: ttweetser.c

ttweetcli: ttweetcli.c

test: ttweetcli ttweetser
	python3 judge2.separate.py "./"

clean:
	rm -r ttweetser ttweetcli cs3251*.client.txt network*.client.txt \
		cxworks.client.txt receiver.client.txt sender.client.txt \
		client.txt server.txt
