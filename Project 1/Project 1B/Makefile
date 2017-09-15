#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 1b - CS 111 - Summer 2017

#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra -lmcrypt
sources = lab1b-server.c lab1b-client.c Makefile README my.key
all: server client

server: 
	$(CC) $(CFLAGS) -o lab1b-server lab1b-server.c
client:
	$(CC) $(CFLAGS) -o lab1b-client lab1b-client.c
clean: 
	@rm -f lab1b-server lab1b-client lab1b-1111111111.tar.gz
dist: 
	tar -cf lab1b-111111111.tar.gz $(sources)
