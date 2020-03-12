#Name: Tian Ye
#Email: tianyesh@gmail.com
#UID: 704931660
default:
	gcc -lmraa -lm -Wall -Wextra -g -o lab4b lab4b.c
check: default
	./tests.sh
clean:
	rm -f lab4b *.tar.gz
dist:
	tar -czvf lab4b-704931660.tar.gz lab4b.c tests.sh Makefile README