#!/bin/bash
# This script is used to generate the .csv file for the lab.
# FNU SHIVA SANDESH 
# UID : 404772648 
# Email: sandesh.shiva362@gmail.com

# for the first part of the lab.
rm -rf lab2b_list.csv
rm -rf lab2b_list1.csv

num_threads_a=(1 2 4 8 12 16 24)
for i in "${num_threads_a[@]}"; do
    ./lab2_list --threads=$i --iterations=1000 --sync=s  >> lab2b_list1.csv 
    ./lab2_list --threads=$i --iterations=1000 --sync=m  >> lab2b_list1.csv 
done

# Yield with no synchronization
num_threads_c=(1 4 8 12 16)
num_iterations_c=(1 2 4 8 16)
for j in "${num_threads_c[@]}"; do
    for k in "${num_iterations_c[@]}"; do
        ./lab2_list --threads=$j --iterations=$k  --yield=id --lists=4 >> lab2b_list.csv 2> /dev/null
        done
done
#yield with synchronization
num_threads_d=(1 4 8 12 16)
num_iterations_d=(10 20 40 80)
for j in "${num_threads_d[@]}"; do
    for k in "${num_iterations_d[@]}"; do
        ./lab2_list --threads=$j --iterations=$k  --yield=id --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=$j --iterations=$k  --yield=id --lists=4 --sync=s >> lab2b_list.csv
	done
done

lists=(1 4 8 16)
num_threads_e=(1 2 4 8 12)
for i in "${num_threads_e[@]}"; do
    for j in "${lists[@]}"; do
	./lab2_list --threads=$i --iterations=1000 --sync=m --lists=$j >> lab2b_list.csv
	./lab2_list --threads=$i --iterations=1000 --sync=s --lists=$j >> lab2b_list.csv
    done
done


