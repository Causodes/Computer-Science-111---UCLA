#Name: Tian Ye
#Email: tianyesh@gmail.com
#UID: 704931660
default:
	gcc -lmraa -lm  -Wall -Wextra -g -o lab4c_tcp lab4c_tcp.c
	gcc -lssl -lcrypto -lmraa -lm  -Wall -Wextra -g -o lab4c_tls lab4c_tls.c
clean:
	rm -f lab4c_tls lab4c_tcp *.o *.tar.gz *.txt
dist: default
	tar -czvf lab4c-704931660.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c