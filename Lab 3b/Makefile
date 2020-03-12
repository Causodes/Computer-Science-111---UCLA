#Name: Tian Ye
#Email: tianyesh@gmail.com
#UID: 704931660
default:
	rm -f lab3b
	echo '#!/bin/bash' > lab3b
	echo 'python lab3b.py $$1' >> lab3b
	chmod +x lab3b
clean:
	rm -rf lab3b lab3b-704931660.tar.gz
dist:
	tar -czvf lab3b-704931660.tar.gz README Makefile lab3b.py