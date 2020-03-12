#!/bin/bash

rm lab2_add.csv
touch lab2_add.csv

for i in 1 2 4 8 12
do
    for j in 10 20 40 80 100 1000 10000 100000
    do
		./lab2_add 		--threads=$i 	--iterations=$j 							>> lab2_add.csv
    done
done

for i in 1 2 4 8 12
do
    for j in 10 20 40 80 100 1000 10000 100000
    do
        ./lab2_add		--threads=$i 	--iterations=$j 	--yield 				>> lab2_add.csv
    done
done

for i in 2 4 8 12
do
    for j in 1000 10000
    do
		./lab2_add 		--threads=$i 	--iterations=$j 	--yield 	--sync=m 	>> lab2_add.csv
		./lab2_add 		--threads=$i 	--iterations=$j 	--yield 	--sync=s 	>> lab2_add.csv
		./lab2_add 		--threads=$i 	--iterations=$j 	--yield 	--sync=c 	>> lab2_add.csv
    done
done

for i in 1 2 4 8 12
do
    for j in 100 300 1000 3000 10000
    do
        ./lab2_add 		--threads=$i 	--iterations=$j 							>> lab2_add.csv
        ./lab2_add 		--threads=$i 	--iterations=$j 				--sync=m 	>> lab2_add.csv
        ./lab2_add 		--threads=$i 	--iterations=$j 				--sync=s 	>> lab2_add.csv
        ./lab2_add 		--threads=$i 	--iterations=$j 				--sync=c 	>> lab2_add.csv
    done
done