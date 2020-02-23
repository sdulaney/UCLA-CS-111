#!/bin/sh

# lab2b_1.png, lab2b_2.png
./lab2_list --sync=m --iterations=1000 --threads=1
./lab2_list --sync=m --iterations=1000 --threads=2
./lab2_list --sync=m --iterations=1000 --threads=4
./lab2_list --sync=m --iterations=1000 --threads=8
./lab2_list --sync=m --iterations=1000 --threads=12
./lab2_list --sync=m --iterations=1000 --threads=16
./lab2_list --sync=m --iterations=1000 --threads=24
./lab2_list --sync=s --iterations=1000 --threads=1
./lab2_list --sync=s --iterations=1000 --threads=2
./lab2_list --sync=s --iterations=1000 --threads=4
./lab2_list --sync=s --iterations=1000 --threads=8
./lab2_list --sync=s --iterations=1000 --threads=12
./lab2_list --sync=s --iterations=1000 --threads=16
./lab2_list --sync=s --iterations=1000 --threads=24
# lab2b_3.png
# Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations (and no synchronization) to see how many iterations it takes to reliably fail (and make sure your Makefile expects some of these tests to fail).
./lab2_list --lists=4 --iterations=1 --threads=1 --yield=id
./lab2_list --lists=4 --iterations=2 --threads=1 --yield=id
./lab2_list --lists=4 --iterations=4 --threads=1 --yield=id
./lab2_list --lists=4 --iterations=8 --threads=1 --yield=id
./lab2_list --lists=4 --iterations=16 --threads=1 --yield=id
./lab2_list --lists=4 --iterations=1 --threads=4 --yield=id
./lab2_list --lists=4 --iterations=2 --threads=4 --yield=id
./lab2_list --lists=4 --iterations=4 --threads=4 --yield=id
./lab2_list --lists=4 --iterations=8 --threads=4 --yield=id
./lab2_list --lists=4 --iterations=16 --threads=4 --yield=id
./lab2_list --lists=4 --iterations=1 --threads=8 --yield=id
./lab2_list --lists=4 --iterations=2 --threads=8 --yield=id
./lab2_list --lists=4 --iterations=4 --threads=8 --yield=id
./lab2_list --lists=4 --iterations=8 --threads=8 --yield=id
./lab2_list --lists=4 --iterations=16 --threads=8 --yield=id
./lab2_list --lists=4 --iterations=1 --threads=12 --yield=id
./lab2_list --lists=4 --iterations=2 --threads=12 --yield=id
./lab2_list --lists=4 --iterations=4 --threads=12 --yield=id
./lab2_list --lists=4 --iterations=8 --threads=12 --yield=id
./lab2_list --lists=4 --iterations=16 --threads=12 --yield=id
./lab2_list --lists=4 --iterations=1 --threads=16 --yield=id
./lab2_list --lists=4 --iterations=2 --threads=16 --yield=id
./lab2_list --lists=4 --iterations=4 --threads=16 --yield=id
./lab2_list --lists=4 --iterations=8 --threads=16 --yield=id
./lab2_list --lists=4 --iterations=16 --threads=16 --yield=id
# Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 10, 20, 40, 80 iterations, --sync=s and --sync=m to confirm that updates are now properly protected (i.e., all runs succeeded).
./lab2_list --lists=4 --iterations=10 --threads=1 --yield=id --sync=m
./lab2_list --lists=4 --iterations=20 --threads=1 --yield=id --sync=m
./lab2_list --lists=4 --iterations=40 --threads=1 --yield=id --sync=m
./lab2_list --lists=4 --iterations=80 --threads=1 --yield=id --sync=m
./lab2_list --lists=4 --iterations=10 --threads=4 --yield=id --sync=m
./lab2_list --lists=4 --iterations=20 --threads=4 --yield=id --sync=m
./lab2_list --lists=4 --iterations=40 --threads=4 --yield=id --sync=m
./lab2_list --lists=4 --iterations=80 --threads=4 --yield=id --sync=m
./lab2_list --lists=4 --iterations=10 --threads=8 --yield=id --sync=m
./lab2_list --lists=4 --iterations=20 --threads=8 --yield=id --sync=m
./lab2_list --lists=4 --iterations=40 --threads=8 --yield=id --sync=m
./lab2_list --lists=4 --iterations=80 --threads=8 --yield=id --sync=m
./lab2_list --lists=4 --iterations=10 --threads=12 --yield=id --sync=m
./lab2_list --lists=4 --iterations=20 --threads=12 --yield=id --sync=m
./lab2_list --lists=4 --iterations=40 --threads=12 --yield=id --sync=m
./lab2_list --lists=4 --iterations=80 --threads=12 --yield=id --sync=m
./lab2_list --lists=4 --iterations=10 --threads=16 --yield=id --sync=m
./lab2_list --lists=4 --iterations=20 --threads=16 --yield=id --sync=m
./lab2_list --lists=4 --iterations=40 --threads=16 --yield=id --sync=m
./lab2_list --lists=4 --iterations=80 --threads=16 --yield=id --sync=m
./lab2_list --lists=4 --iterations=10 --threads=1 --yield=id --sync=s
./lab2_list --lists=4 --iterations=20 --threads=1 --yield=id --sync=s
./lab2_list --lists=4 --iterations=40 --threads=1 --yield=id --sync=s
./lab2_list --lists=4 --iterations=80 --threads=1 --yield=id --sync=s
./lab2_list --lists=4 --iterations=10 --threads=4 --yield=id --sync=s
./lab2_list --lists=4 --iterations=20 --threads=4 --yield=id --sync=s
./lab2_list --lists=4 --iterations=40 --threads=4 --yield=id --sync=s
./lab2_list --lists=4 --iterations=80 --threads=4 --yield=id --sync=s
./lab2_list --lists=4 --iterations=10 --threads=8 --yield=id --sync=s
./lab2_list --lists=4 --iterations=20 --threads=8 --yield=id --sync=s
./lab2_list --lists=4 --iterations=40 --threads=8 --yield=id --sync=s
./lab2_list --lists=4 --iterations=80 --threads=8 --yield=id --sync=s
./lab2_list --lists=4 --iterations=10 --threads=12 --yield=id --sync=s
./lab2_list --lists=4 --iterations=20 --threads=12 --yield=id --sync=s
./lab2_list --lists=4 --iterations=40 --threads=12 --yield=id --sync=s
./lab2_list --lists=4 --iterations=80 --threads=12 --yield=id --sync=s
./lab2_list --lists=4 --iterations=10 --threads=16 --yield=id --sync=s
./lab2_list --lists=4 --iterations=20 --threads=16 --yield=id --sync=s
./lab2_list --lists=4 --iterations=40 --threads=16 --yield=id --sync=s
./lab2_list --lists=4 --iterations=80 --threads=16 --yield=id --sync=s
