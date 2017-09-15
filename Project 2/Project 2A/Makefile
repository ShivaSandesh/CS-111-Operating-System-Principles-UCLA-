#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 2A  - CS 111 - Summer 2017

#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra -lpthread
sources = lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile README lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png csv_generate.sh lab2_list.gp lab2_add.gp

sources_To_Remove = lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2a-404772648.tar.gz lab2_add lab2_list

default:  
	$(CC) $(CFLAGS) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) -o lab2_list SortedList.c lab2_list.c
tests:  
	@echo "Running test script....... This might generate Seg Faults but script won't stop. Thanks for the patience.\n"
	 -./csv_generate.sh 
graphs:
	/usr/local/cs/bin/gnuplot lab2_add.gp
	/usr/local/cs/bin/gnuplot lab2_list.gp
clean: 
	@rm -f $(sources_To_Remove)

dist:	default tests graphs  
	tar -cf lab2a-111111111.tar.gz $(sources)
