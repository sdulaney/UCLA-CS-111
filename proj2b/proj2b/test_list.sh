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
