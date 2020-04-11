CFLAGS = -Wall -pedantic -std=gnu99 -g -pthread  -fsanitize=address
CC = clang

all: ttweetser ttweetcli

ttweetser: ttweetser.c

ttweetcli: ttweetcli.c

test: ttweetcli ttweetser
	python3 judge2.separate.py c
	mv client.txt our_client.txt
	mv server.txt our_server.txt
	git diff --no-index their_client.txt our_client.txt > \
	    client.diff

clean:
	rm ttweetser ttweetcli cs3251*.client.txt network*.client.txt \
	    cxworks.client.txt receiver.client.txt sender.client.txt
