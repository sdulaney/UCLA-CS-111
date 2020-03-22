# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904064791

CC = gcc
CFLAGS = -g -Wall -Wextra -lmraa -lm -lssl -lcrypto
ID = 904064791
LAB = 4c
ARCHIVE = lab$(LAB)-$(ID).tar.gz
sources = lab$(LAB).c Makefile README

build: lab$(LAB)

lab$(LAB):
	$(CC) $(CFLAGS) lab$(LAB).c -o lab$(LAB)
	-ln lab$(LAB) lab$(LAB)_tcp
	-ln lab$(LAB) lab$(LAB)_tls

clean:
	-rm lab$(LAB)
	-rm lab$(LAB)_tcp
	-rm lab$(LAB)_tls
	-rm $(ARCHIVE)

dist: clean
	-tar -czf $(ARCHIVE) $(sources)
