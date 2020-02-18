#!/bin/sh

./lab2_add --iterations=100
./lab2_add --iterations=1000
./lab2_add --iterations=10000
./lab2_add --iterations=100000
./lab2_add --iterations=100 --threads=2
./lab2_add --iterations=1000 --threads=2
./lab2_add --iterations=10000 --threads=2
./lab2_add --iterations=100000 --threads=2
./lab2_add --iterations=100 --threads=4
./lab2_add --iterations=1000 --threads=4
./lab2_add --iterations=10000 --threads=4
./lab2_add --iterations=100000 --threads=4
./lab2_add --iterations=100 --threads=6
./lab2_add --iterations=1000 --threads=6
./lab2_add --iterations=10000 --threads=6
./lab2_add --iterations=100000 --threads=6
./lab2_add --iterations=100 --threads=8
./lab2_add --iterations=1000 --threads=8
./lab2_add --iterations=10000 --threads=8
./lab2_add --iterations=100000 --threads=8
./lab2_add --iterations=100 --threads=10
./lab2_add --iterations=1000 --threads=10
./lab2_add --iterations=10000 --threads=10
./lab2_add --iterations=100000 --threads=10
./lab2_add --iterations=10 --threads=2 --yield
./lab2_add --iterations=20 --threads=2 --yield
./lab2_add --iterations=40 --threads=2 --yield
./lab2_add --iterations=80 --threads=2 --yield
./lab2_add --iterations=100 --threads=2 --yield
./lab2_add --iterations=1000 --threads=2 --yield
./lab2_add --iterations=10000 --threads=2 --yield
./lab2_add --iterations=100000 --threads=2 --yield
./lab2_add --iterations=10 --threads=4 --yield
./lab2_add --iterations=20 --threads=4 --yield
./lab2_add --iterations=40 --threads=4 --yield
./lab2_add --iterations=80 --threads=4 --yield
./lab2_add --iterations=100 --threads=4 --yield
./lab2_add --iterations=1000 --threads=4 --yield
./lab2_add --iterations=10000 --threads=4 --yield
./lab2_add --iterations=100000 --threads=4 --yield
./lab2_add --iterations=10 --threads=8 --yield
./lab2_add --iterations=20 --threads=8 --yield
./lab2_add --iterations=40 --threads=8 --yield
./lab2_add --iterations=80 --threads=8 --yield
./lab2_add --iterations=100 --threads=8 --yield
./lab2_add --iterations=1000 --threads=8 --yield
./lab2_add --iterations=10000 --threads=8 --yield
./lab2_add --iterations=100000 --threads=8 --yield
./lab2_add --iterations=10 --threads=12 --yield
./lab2_add --iterations=20 --threads=12 --yield
./lab2_add --iterations=40 --threads=12 --yield
./lab2_add --iterations=80 --threads=12 --yield
./lab2_add --iterations=100 --threads=12 --yield
./lab2_add --iterations=1000 --threads=12 --yield
./lab2_add --iterations=10000 --threads=12 --yield
./lab2_add --iterations=100000 --threads=12 --yield
# lab2_add-4.png
./lab2_add --iterations=10000 --threads=2 --yield --sync=m
./lab2_add --iterations=10000 --threads=4 --yield --sync=m
./lab2_add --iterations=10000 --threads=8 --yield --sync=m
./lab2_add --iterations=10000 --threads=12 --yield --sync=m
./lab2_add --iterations=10000 --threads=2 --yield --sync=c
./lab2_add --iterations=10000 --threads=4 --yield --sync=c
./lab2_add --iterations=10000 --threads=8 --yield --sync=c
./lab2_add --iterations=10000 --threads=12 --yield --sync=c
./lab2_add --iterations=1000 --threads=2 --yield --sync=s
./lab2_add --iterations=1000 --threads=4 --yield --sync=s
./lab2_add --iterations=1000 --threads=8 --yield --sync=s
./lab2_add --iterations=1000 --threads=12 --yield --sync=s
# lab2_add-5.png
./lab2_add --iterations=10000 --threads=1
./lab2_add --iterations=10000 --threads=2
./lab2_add --iterations=10000 --threads=4
./lab2_add --iterations=10000 --threads=8
./lab2_add --iterations=10000 --threads=12
./lab2_add --iterations=10000 --threads=1 --sync=m
./lab2_add --iterations=10000 --threads=2 --sync=m
./lab2_add --iterations=10000 --threads=4 --sync=m
./lab2_add --iterations=10000 --threads=8 --sync=m
./lab2_add --iterations=10000 --threads=12 --sync=m
./lab2_add --iterations=10000 --threads=1 --sync=c
./lab2_add --iterations=10000 --threads=2 --sync=c
./lab2_add --iterations=10000 --threads=4 --sync=c
./lab2_add --iterations=10000 --threads=8 --sync=c
./lab2_add --iterations=10000 --threads=12 --sync=c
./lab2_add --iterations=10000 --threads=1 --sync=s
./lab2_add --iterations=10000 --threads=2 --sync=s
./lab2_add --iterations=10000 --threads=4 --sync=s
./lab2_add --iterations=10000 --threads=8 --sync=s
./lab2_add --iterations=10000 --threads=12 --sync=s
