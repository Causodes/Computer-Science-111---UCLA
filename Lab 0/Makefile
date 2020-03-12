#Tian Ye
#tianyesh@gmail.com
#704931660

default:
	gcc -Wall -Wextra -g lab0.c -o lab0
check: smoketests
clean:
	rm -f *.o lab0 *.txt *.tar.gz
dist: default
	tar -czf lab0-704931660.tar.gz lab0.c Makefile README smoketest.sh *.png
smoketests:
	@chmod +x smoketest.sh; \
	./smoketest.sh