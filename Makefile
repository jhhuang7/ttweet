CFLAGS = -Wall -pedantic -std=gnu99 -g -pthread
.PHONY: all extra
.DEFAULT_GOAL := all
all: ttweetser ttweetcli

ttweetser: ttweetser.c
	gcc $(CFLAGS) ttweetser.c -o ttweetser

ttweetcli: ttweetcli.c
	gcc $(CFLAGS) ttweetcli.c -o ttweetcli
	
clean:
	rm ttweetser ttweetcli