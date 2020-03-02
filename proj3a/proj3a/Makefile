# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904-064-791

CC = gcc
CFLAGS = -g -Wall -Wextra
ID = 904-064-791
LAB = 3a
ARCHIVE = lab$(LAB)-$(ID).tar.gz
OUTPUT = lab2b_list.csv profile.out
GRAPHS = lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
SCRIPTS = lab2_list.gp test_list.sh
sources = lab3a.c Makefile README $(OUTPUT) $(GRAPHS) $(SCRIPTS)

build: lab3a

lab3a:
	$(CC) $(CFLAGS) lab3a.c -o lab3a

dist: clean
	tar -czf $(ARCHIVE) $(sources)

clean:
	-rm lab3a
	-rm lab$(LAB)-904-064-791.tar.gz

