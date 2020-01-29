/*
 * usage: logfilter [--tag=string] file ...
 *
 *	process lines of the form
 *		TAG ## bytes: data
 *
 *	pull out the data, and send it to stdout
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

#define	MAXLINE	512

/*
 * this routine is the active ingredient
 *	doing a byte-at-a-time scan
 *		look for the specified tag
 *		look for the trailing white space
 *		look for the decimal byte count
 *		look for the delimiting colon
 *		eat one more blank/tab
 *		read the specified number of bytes
 *
 * 	and if anything doesn't match, go back to initial tag match
 */
void process(FILE *input, const char *tag) {

	const char *match = tag;
	for(;;) {
		/* scan until we hit the tag */
		int c;
		if ((c = fgetc(input)) == EOF)
			break;
		if (c != *match) {
			match = tag;
			continue;
		}
		if (*++match)
			continue;

		/* matched tag, skip over white space */
		match = tag;
		do {	c = fgetc(input);
		} while (c == ' ' || c == '\t');
		if (c == EOF)
			break;

		/* try to read a byte count followed by white space	*/
		int bytes = 0;
		while (c >= '0' && c <= '9') {
			bytes *= 10;
			bytes += c - '0';
			c = fgetc(input);
		}
		if (c != ' ' && c != '\t')
			continue;

		/* skip ahead to the colon	*/
		while( c != EOF && c != ':' )
			c = fgetc(input);

		/* eat one more space	*/
		c = fgetc(input);
		if (c != ' ' && c != '\t')
			continue;

		/* we hit data	*/
		while( bytes-- > 0 ) {
			c = fgetc(input);
			fputc(c, stdout);
		}
	}
}

struct option args[] = {
	{"tag",		required_argument,	NULL, 't'},
	{0, 0, 0, 0}
};

const char *usage = "Usage: %s [--tag=string] file ...\n";

int main( int argc, char **argv ) {
	const char *tag = "RECEIVED";

	/* process the arguments	*/
	int i;
	while( (i = getopt_long(argc, argv, "t:", args, NULL)) != -1 ) {
	    switch(i) {
	    	case 't':
	    		tag = optarg;
			break;

		default:
			fprintf(stderr, usage, argv[0]);
			exit(1);
	    }
	}

	/* process the specified input files, or stdin	*/
	if (optind < argc)
		while(optind < argc) {
			FILE *f = fopen(argv[optind++], "r");
			if (f == NULL) {
				fprintf(stderr, "Unable to open input file %s\n", argv[optind-1]);
				exit(1);
			}
			process(f, tag);
			fclose(f);
		}
	else
		process(stdin, tag);

	exit(0);
}
