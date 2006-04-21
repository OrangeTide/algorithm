#include <stdio.h>
#include "argv.h"

void test_argv_shell(const char *str) {
	int i;
	int argc;
	char **argv;
	makeargv_shell(str, &argc, &argv);
	for(i=0;i<argc;i++) {
		printf("argv[%d]=\"%s\"\n", i, argv[i]);    
	}
	freeargv(argc, argv);
}

int main(int argc, char **argv) {
	int i;
	char buf[4096];
	/*
	for(i=1;i<argc;i++) {
		test_argv_shell(argv[i]);
		printf("\n");
	}
	*/
	while(fgets(buf, sizeof buf, stdin)) {
		test_argv_shell(buf);
	}
}
