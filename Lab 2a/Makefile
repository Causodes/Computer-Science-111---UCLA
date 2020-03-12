#Name: Tian Ye
#Email: tianyesh@gmail.com
#UID: 704931660

default:
	gcc -pthread -o lab2_add lab2_add.c
	gcc -pthread -Wall -Wextra -g SortedList.c  -o lab2_list lab2_list.c

clean:
	rm -f lab2_add lab2_list *.o *.tar.gz
	
dist: graphs
	tar -czvf lab2a-704931660.tar.gz lab2_add.c lab2_list.c SortedList.h lab2_add.csv lab2_list.csv lab2_add-1.png  lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png  lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png Makefile README SortedList.c lab2_add_test.sh lab2_list_test.sh lab2_add.gp lab2_list.gp

graphs: tests
	./lab2_add.gp
	./lab2_list.gp
	
tests:
	./lab2_add_test.sh
	./lab2_list_test.sh