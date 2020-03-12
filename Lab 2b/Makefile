#Name: Tian Ye
#Email: tianyesh@gmail.com
#UID: 704931660

default:
	gcc -pthread -o lab2_list -Wall -Wextra -g SortedList.c lab2_list.c
tests: default
	touch lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=24 	--iterations=1000 	--sync=m							>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000 	--sync=s							>> lab2b_list.csv
	-./lab2_list --threads=24 	--iterations=1000 	--sync=s 							>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=2 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=4 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=8 					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=12 				--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=16					--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=10		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=10		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=20		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=20		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=40		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=40		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=80		--sync=m	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=80		--sync=s	--yield=id 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=m			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=s			 	--lists=4 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=m			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=s			 	--lists=8 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=m			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=1 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=2 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=4 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=8 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=12 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
	-./lab2_list --threads=16 	--iterations=1000	--sync=s			 	--lists=16 	>> lab2b_list.csv
graphs: tests
	./lab2_list.gp
profile:
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=cpu_profile ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list cpu_profile > profile.out
	pprof --list=thread_exec lab2_list cpu_profile >> profile.out
clean:
	rm -f lab2_list *.o *.tar.gz
dist: graphs
	tar -czvf lab2b-704931660.tar.gz SortedList.h SortedList.c lab2_list.c lab2b_list.csv lab2_list.gp lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png profile.out Makefile README
