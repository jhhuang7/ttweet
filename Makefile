CFLAGS = -Wall -pedantic -std=gnu99 -g -pthread
.PHONY: all extra
.DEFAULT_GOAL := all
all: ttweetser ttweetcli

ttweetser: ttweetser.o
	gcc ttweetser.o -o ttweetser

ttweetcli: ttweetcli.o
	gcc ttweetcli.o -o ttweetcli

ttweetsrv.o: ttweetser.c
	gcc $(CFLAGS) -c ttweetser.c

ttweetcli.o: ttweetcli.c
	gcc $(CFLAGS) -c ttweetcli.c


clean:
	rm ttweetser ttweetcli ttweetser.o ttweetcli.o