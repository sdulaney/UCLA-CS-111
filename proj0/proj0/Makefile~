# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904-064-791 

.SILENT: check

build:
	gcc lab0.c -o lab0 -g -Wall -Wextra

check: clean build
	echo "exercising input/output features"
	EXIT_OK=0
	EXIT_ARG=1
	EXIT_BADIN=2
	EXIT_BADOUT=3
	EXIT_FAULT=4
	SIG_SEGFAULT=139

	echo "... correctly copy input to output"
	echo "Hello, world!" > in
	cat in | ./lab0 > out
	RC=$?
	if [ $RC -eq 0 ]
        then
                echo "        RC=$RC ... OK"
        else
                echo "        RC=$RC ... FAILURE"
        fi
	cmp in out > /dev/null
	if [ $? -eq 0 ]
	then
		echo "        data comparison ... OK"
	else
		echo "        data comparison ... FAILURE"
	fi

clean:
	rm lab0
	rm lab0-904-064-791.tar.gz

dist:
	tar -czf lab0-904-064-791.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
