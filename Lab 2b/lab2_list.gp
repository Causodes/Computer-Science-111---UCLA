#! /usr/bin/gnuplot
# Name: Tian Ye
# Email: tianyesh@gmail.com
# ID: 704931660

# general plot parameters
set terminal png
set datafile separator ","

# Graph 1:
set title "List-1: Throughput vs Number of Threads"
set xlabel "Number of Threads"
set xrange [0.5:]
set ylabel "Throughput (operations/sec)"
set logscale x 2
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'Mutex' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
    title 'Spin-Lock' with linespoints lc rgb 'green'

# Graph 2:
set title "List-2: Wait-for-Lock Time vs Average Time per Operation"
set xlabel "Number of Threads"
set xrange [0.5:]
set ylabel "Time (ns)"
set logscale x 2
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
    title 'Average Time per Operation' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
    title 'Average Wait Time for Lock' with linespoints lc rgb 'green'

# Graph 3:
set title "List-3: Iterations that run without Failure"
set xrange [0.5:]  #xrange has to be greater than 0 for logscale
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale x 2
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb "red" title "Unprotected", \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb "green" title "Mutex", \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
    with points lc rgb "blue" title "Spin lock"

# Graph 4:
set title "List-4: Aggregated Throughput vs. The Number of Threads (Mutex Lock)"
set xrange [0.5:] #xrange has to be greater than 0 for logscale
set xlabel "Threads"
set ylabel "Throughput (operations/sec)"
set logscale x 2
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '4 List' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '16 List' with linespoints lc rgb 'purple'


# Graph 5:
set title "List-5: Aggregated Throughput vs. The Number of Threads (Spin Lock)"
set xrange [0.5:]
set xlabel "Threads"
set ylabel "Throughput (operations/sec)"
set logscale x 2
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '1 List' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '4 List' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000 / ($7)) \
	title '16 List' with linespoints lc rgb 'purple'
