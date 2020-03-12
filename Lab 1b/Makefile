default:
		gcc -lz -Wall -Wextra -o lab1b-client lab1b-client.c -lz
		gcc -lz -Wall -Wextra -o lab1b-server lab1b-server.c -lz
clean:
		rm -f *.o lab1b-client lab1b-server *.tar.gz
dist: default
		tar -czf lab1b-704931660.tar.gz lab1b-client.c lab1b-server.c Makefile README