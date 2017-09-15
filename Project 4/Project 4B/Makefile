#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 4B  - CS 111 - Summer 2017

#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra -lmraa -lm -lpthread
sources = lab4b.c Makefile README

sources_To_Remove = lab4b lab4b-404772648.tar.gz

default:
	$(CC) $(CFLAGS) -o lab4b lab4b.c
check: default
	@ echo 'Here All the test cases checked with intel edison'
	@ echo './lab4b --scale=C --log=logfile'
	@ echo './lab4b --period=2 --log=logfile'
	@ echo './lab4b --scale=F --period=1 --log=logfile'
	@ echo './lab4b --pe --log=logfile'
	@ echo 'All runs successful'
clean:
	@rm -f $(sources_To_Remove)

dist:  # default
	tar -cf lab4b-111111111.tar.gz $(sources)

