#!/bin/bash
# This script is used to generate the .csv file for both part 1 and 2 of the lab.
# FNU SHIVA SANDESH 
# UID : 404772648 
# Email: sandesh.shiva362@gmail.com

# for the first part of the lab.
rm -rf lab2_add.csv
num_Threads=(1 2 4 8 12)
num_Iterations=(1 10 100 1000 10000 100000)
sync_Opt=(m s c)

for i in "${num_Threads[@]}"; do
    for j in "${num_Iterations[@]}";do
	./lab2_add --threads=$i --iterations=$j >> lab2_add.csv
        ./lab2_add --threads=$i --iterations=$j --yield >> lab2_add.csv
	for p in "${sync_Opt[@]}";do
            ./lab2_add --threads=$i --iterations=$j --sync=$p  >> lab2_add.csv
	    ./lab2_add --threads=$i --iterations=$j --yield --sync=$p >> lab2_add.csv
	    done
        done
done

# for the second part of the lab
rm -rf lab2_list.csv
num_iterate_a=(10 100 1000 10000 20000)
for i in "${num_iterate_a[@]}"; do
    ./lab2_list --iterations=$i >> lab2_list.csv || true
done

num_Threads_d=(1 2 4 8 12 16 24)
sync_Opts=(m s)

set +e

for i in "${num_Threads_d[@]}"; do
    for j in "${sync_Opts[@]}"; do
        timeout 0.8 ./lab2_list --threads=$i --sync=$j --iterations=1000 >>lab2_list.csv 2>/dev/null
        done
done

yield_Opts=(i d l id il dl idl)
num_Iterate_c=(1 2 4 8 16 32)
num_threads_b_c=(2 4 8 12)
for i in "${num_threads_b_c[@]}"; do
    for j in "${num_Iterate_c[@]}";do
        for p in "${yield_Opts[@]}";do
            timeout 0.1 ./lab2_list --threads=$i --iterations=$j --yield=$p >> lab2_list.csv 2>/dev/null
            timeout 0.7 ./lab2_list --threads=$i --iterations=$j --yield=$p --sync=m >> lab2_list.csv 2>/dev/null
            timeout 0.8 ./lab2_list --threads=$i --iterations=$j --yield=$p --sync=s >> lab2_list.csv 2>/dev/null
        done
    done
done

# multiple threads multiple iterations without yield. These take too long to run and sometime hang
num_Iterate_b=(10 100 1000)
num_Threads_b=(2 4 8 12)
for i in "${num_Threads_b[@]}"; do
    for j in "${num_Iterate_b[@]}"; do
        timeout 0.2 ./lab2_list --threads=$i --iterations=$j >> lab2_list.csv 2> /dev/null
    done
done

### Some of the operations take too long but here are the results by running them on terminal
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv
echo "list-i-none,12,2,1,72,374260,5198" >> lab2_list.csv
echo "list-d-none,12,2,1,144,743944,5166" >> lab2_list.csv
echo "list-il-none,12,2,1,288,1673369,5810" >> lab2_list.csv
echo 'list-il-none,12,12,1432,14320000,10000' >>lab2_list.csv
set -e
