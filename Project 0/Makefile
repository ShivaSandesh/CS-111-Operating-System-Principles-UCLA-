#       NAME: FNU SHIVA SANDESH
#       EMAIL: sandesh.shiva362@gmail.com
#       ID: 111-111-111
#       Project 0 - CS 111 - Summer 2017

#       target : dependencies
CC = gcc
CFLAGS = -g -Wextra   
sources = lab0.c Makefile backtrace.png breakpoint.png README
default:
	$(CC) $(CFLAGS)-o lab0 lab0.c
check: 
	@echo Begining the test.
	@touch data; chmod 600 data; ./lab0 --input=data --output=out; if [ $$? -ne 0 ] ; then echo "Incorrect Return value ! It is: $?" ; exit 1 ; fi; diff -q data out; rm -f data out
	@./lab0 --segfault --catch 2> /dev/null ; if [ $$? -ne 4 ] ; then echo "Incorrect Return value for --catch ! It is: $?" ; exit 1 ; fi
	@touch data;./lab0 --input=data --segfault --catch 2> /dev/null ; if [ $$? -ne 4 ] ; then echo "Incorrect Return value for --catch! It is: $?" ; exit 1 ; fi
	@touch data; chmod 600 data; ./lab0 --input=data; rm -f data; 
	@touch out; chmod 000 out; ./lab0 --output=out 2> /dev/null; if [ $$? -ne 3 ] ; then echo "Incorrect Return value for --output! It is: $?" ; exit 1 ; fi;rm -f out
	@./lab0 --not_an_option 2> /dev/null; if [ $$? -ne 1 ] ; then echo "Incorrect Return value ! It is: $?" ; exit 1; fi 
	@echo Test compeleted successfuly.
clean:
	@rm -f lab0 lab0-111111111.tar.gz
dist:
	tar -cf lab0-111111111.tar.gz $(sources) 
