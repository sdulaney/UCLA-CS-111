# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904-064-791

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa
ID = 904-064-791
LAB = 4b
ARCHIVE = lab$(LAB)-$(ID).tar.gz
SCRIPTS = test.sh
sources = lab4b.c Makefile README $(SCRIPTS)

build: lab4b

lab4b:
	$(CC) $(CFLAGS) lab4b.c -o lab4b

check: build
	-chmod +x test.sh
	-./test.sh

clean:
	-rm lab4b
	-rm lab$(LAB)-904-064-791.tar.gz

dist: clean
	tar -czf $(ARCHIVE) $(sources)
