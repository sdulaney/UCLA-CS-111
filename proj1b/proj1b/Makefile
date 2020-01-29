# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904-064-791 

build: lab1b-client lab1b-server

lab1b-client:
	gcc lab1b-client.c -o lab1b-client -g -Wall -Wextra -lz

lab1b-server:
	gcc lab1b-server.c -o lab1b-server -g -Wall -Wextra -lz

clean:
	-rm lab1b-client
	-rm lab1b-server
	-rm lab1b-904-064-791.tar.gz

dist:
	tar -czf lab1b-904-064-791.tar.gz lab1b-client.c lab1b-server.c Makefile README
