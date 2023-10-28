#!/bin/bash

# This is a shell script to compile and run 1.c program 10 times.
cd os
truncate -s 0 game_log.txt

for ((i = 0; i < 10; i++)); do
    gcc -o a 1.c
    ./a
done