#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 4C  - CS 111 - Summer 2017
#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra -lmraa -lm
sources = lab4c_tcp.c lab4c_tls.c Makefile README

sources_To_Remove = lab4c_tcp lab4c_tls lab4c-404772648.tar.gz

default:
	$(CC) $(CFLAGS) -o lab4c_tcp lab4c_tcp.c
	$(CC) $(CFLAGS) -lssl -lcrypto -o lab4c_tls lab4c_tls.c
clean:
	@rm -f $(sources_To_Remove)

dist:
	tar -cf lab4c-11111111.tar.gz $(sources)
