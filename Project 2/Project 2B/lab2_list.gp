#! /usr/bin/gnuplot
#
# purpose:
#  generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
# 1. test name
# 2. # threads
# 3. # iterations per thread
# 4. # lists
# 5. # operations performed (threads x iterations x (ins + lookup + delete))
# 6. run time (ns)
# 7. run time per operation (ns)
#
# output:
# lab2_list-1.png ... cost per operation vs threads and iterations
# lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
# lab2_list-3.png ... threads and iterations that run (protected) w/o failure
# lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
# Managing data is simplified by keeping all of the results in a single
# file.  But this means that the individual graphing commands have to
# grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Throughput vs Threads"
set xlabel "Iterations"
set logscale x 10
set ylabel "Throuput Anaysis"
set logscale y 10
set output 'lab2b_1.png'
#set xrange [0.75:]
# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list1.csv" using ($2):(1000000000/$7) \
     title '--sync=m' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list1.csv" using ($2):(1000000000/$7) \
     title '--sync=s' with linespoints lc rgb 'green'


set title "Wait-for-lock time, and Avrg. time per operation against threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-none-m,' lab2b_list1.csv" using ($2):($8) \
     title 'wait for lock' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,' lab2b_list1.csv" using ($2):($7) \
     title 'Average time per opearation' with linespoints lc rgb 'red'
     
set title "List-3: List = 4 with properly protected synchronization"
unset logscale x
set xlabel "No. of Threads"
set ylabel "successful iterations"
#set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'red' title 'No Synchronization', \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb 'black' title '--sync = s',\
    "< grep 'list-id-m,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
     title '--sync=m' with points lc rgb 'blue' 
    
set title "List-4: Throughput with --sync = m with partitioned list"
set xlabel "Threads"
set ylabel "Throughput"
set logscale y
set output 'lab2b_4.png'
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '1 sub-list with mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '4 sub-list with mutex' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '8 sub-listlist w/mutex' with linespoints lc rgb 'orange', \
     "< grep -e 'list-none-m,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '16 sub-list with mutex' with linespoints lc rgb 'black'

set title "List-5: Throughput with --sync = s with partitioned list"
set xlabel "Threads"
set ylabel "Throughput"
set logscale y
set output 'lab2b_5.png'
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '1 sub-list with spinlock' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '4 sub-list with spinlock' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '8 sub-list with spinlock' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-s,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/$7)\
     title '16 sub-list with spinlock' with linespoints lc rgb 'black'
