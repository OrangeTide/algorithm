#include <stdio.h>
#include <ctype.h>
#include "match.h"

/*
int mask_match(const char *m, const char *s) {
	if(!m || !s || !*m || !*s) return 0;

	while(*m && (m[0]=='*' || m[0]=='?')) m++;
	if(!*m) return 1;

	while(*m=='?' || tolower(*m)==tolower(*s)) {
		m++; s++;
	}
}
*/

int test_match(const char *pattern, const char *str) {
	printf("'%s' %s '%s'\n", pattern, match(pattern, str) ? "matches" : "doesn't match", str);
}

int main(int argc, char **argv) {
	int i;
	char *mask;

	if(argc<2) return 0;
	mask=argv[1];
	
	for(i=2;i<argc;i++) {
		test_match(mask, argv[i]);
		printf("\n");
	}
	return 0;
}
