# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904-064-791 

build:
	gcc lab1a.c -o lab1a -g -Wall -Wextra

check: build
	@echo "correctly copy input to output"
	@echo "Hello, world!" > in
	@cat in | ./lab0 > out; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp in out > /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "stdout...$$X";

	@echo "correctly implements --input"
	@./lab0 --input=in > out; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp in out > /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "stdout...$$X";

	@echo "correctly implements --output"
	@cat in | ./lab0 --output=out; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp in out > /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "stdout...$$X";

	@echo "implements combined --input + --output"
	@./lab0 --input=in --output=out; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp in out > /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 0 ] && X="OK"; echo "stdout...$$X";

	@echo "correct handling of un-openable/un-creatable output file"
	@echo "- un-openable output file"
	@chmod -rw out
	@cat in | ./lab0 --output=out 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 3 ] && X="OK"; echo "RC=$$RC...$$X";
	@touch empty
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "stderr...$$X";
	@echo "- un-creatable output file"
	@cat in | ./lab0 --output=/ 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 3 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo \
"stderr...$$X";

	@echo "correct handling of non-existent/un-openable input file"
	@echo "- non-existent input file"
	@./lab0 --input=i 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 2 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "stderr...$$X";

	@echo "- un-openable input file"
	@chmod -rw in
	@./lab0 --input=in 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 2 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "stderr...$$X";

	@chmod +rw in
	@chmod +rw out

	@echo "correct handling of invalid arguments"
	@./lab0 --blah 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "stderr...$$X";

	@echo "generate (and die from) SIGSEGV"
	@./lab0 --segfault; RC=$$?; X="FAIL"; [ $$RC -eq 139 ] && X="OK"; echo "RC=$$RC...$$X";

	@echo "catch and report SIGSEGV"
	@./lab0 --catch --segfault 2> out.2; RC=$$?; X="FAIL"; [ $$RC -eq 4 ] && X="OK"; echo "RC=$$RC...$$X";
	@cmp empty out.2 &> /dev/null; RC=$$?; X="FAIL"; [ $$RC -eq 1 ] && X="OK"; echo "stderr...$$X";

clean:
	-rm lab0
	-rm lab0-904-064-791.tar.gz
	-rm in
	-rm out
	-rm out.2
	-rm empty

dist:
	tar -czf lab1a-904-064-791.tar.gz lab1a.c Makefile README
