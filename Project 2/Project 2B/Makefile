#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 2A  - CS 111 - Summer 2017

#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra -lpthread
sources = SortedList.h SortedList.c lab2_list.c Makefile README lab2b_list.csv lab2b_list1.csv lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png csv_generate.sh lab2_list.gp profile.out 
sources_To_Remove = lab2b_list.csv lab2b_list1.csv lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b-404772648.tar.gz lab2_list

default:
	$(CC) $(CFLAGS) -o lab2_list SortedList.c lab2_list.c
tests:  
	@echo "Running test script.......  Thanks for the patience."
	 -./csv_generate.sh 
graphs: default tests 
	@/usr/local/cs/bin/gnuplot lab2_list.gp
clean: 
	@rm -f $(sources_To_Remove)

dist:	default tests graphs profile  
	@tar -cf lab2b-111111111.tar.gz $(sources)
profile:
	@LD_PRELOAD=~/pprof/lib/libprofiler.so.0 CPUPROFILE=raw ./lab2_list  --iterations=1000 --threads=12 --sync=s
	@~/pprof/bin/pprof --text lab2_list raw > proff.out	
	@~/pprof/bin/pprof --list=busy_Worker lab2_list raw > profile.out
