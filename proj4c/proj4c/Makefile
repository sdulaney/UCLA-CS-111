# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904064791

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa -lm
ID = 904064791
LAB = 4c
ARCHIVE = lab$(LAB)-$(ID).tar.gz
SCRIPTS = test.sh
sources = lab$(LAB).c Makefile README $(SCRIPTS)

build: lab$(LAB)

lab$(LAB):
	$(CC) $(CFLAGS) lab$(LAB).c -o lab$(LAB)

check: build
	-chmod +x test.sh
	-./test.sh

clean:
	-rm lab$(LAB)
	-rm $(ARCHIVE)

dist: clean
	-tar -czf $(ARCHIVE) $(sources)
