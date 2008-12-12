#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "udb.h"
#include "proto.h"

#define NR(x) (sizeof(x)/sizeof*(x))

static struct udb_handle *proto_h;

/* ignore this class */
static const char *keywords_ignore[] = {
	"static", "const",
};

/* if this class, then only this class but multiple of them are allowed */
static const char *keywords_types[] = {
	"unsigned", "signed", "short", "long", "int",
	"char", "double", "float", "void"
};

/* if this class, then a single identifier follows */
static const char *keywords_single[] = {
	"enum", "union", "struct",
};

static unsigned match_word(const char *line, const char *word) {
	unsigned i;
	for(i=0;*line && !isspace(*line);i++) {
		if(*word!=*line) return 0; /* failed to match */
		line++;
		word++;
	}
	if(*word)
		return 0; /* failure, did not use all of word */

	return i; /* success */
}

static unsigned next_word(const char *line) {
	unsigned i;
	for(i=0;line[i] && !isspace(line[i]);i++) {
	}
	return i;
}

static int isident(char c) {
	return isalnum(c) || c=='_';
}

static unsigned next_ident(const char *line) {
	unsigned i;
	for(i=0;isident(line[i]);i++) {
	}
	return i;
}

/** eats spaces */
static unsigned eat_spaces(const char *line) {
	unsigned i;
	for(i=0;isspace(line[i]);i++) {
	}
	return i;
}

/** eats spaces * and ( */
static unsigned eat_spaces_and_others(const char *line) {
	unsigned i;
	for(i=0;line[i]=='*' || line[i]=='(' || isspace(line[i]);i++) {
	}
	return i;
}

/** find any of the words on the list */
static unsigned find_any_word(const char *line, const char *words[], unsigned nr_words) {
	unsigned i, res;
	for(i=0;i<nr_words;i++) {
		res=match_word(line, words[i]);
		if(res) {
			return res;
		}
	}
	return 0; /* no match */
}

static int parse_proto_key(const char *line, char *key_out, size_t max) {
	unsigned ofs, res;

	ofs=0;

	/* eat some keywords that we're just ignoring */
	ofs+=eat_spaces_and_others(line+ofs);
	ofs+=find_any_word(line+ofs, keywords_ignore, NR(keywords_ignore));

	ofs+=eat_spaces_and_others(line+ofs);

	if((res=find_any_word(line+ofs, keywords_single, NR(keywords_single)))) {
		/* found: struct, union, enum */
		ofs+=res;
		ofs+=eat_spaces(line+ofs);
		ofs+=res=next_ident(line+ofs); /* */
		if(!res) {
			fprintf(stderr, "Could not parse '%s'\n", line);
			return 0; /* could not find identifier */
		}
		ofs+=eat_spaces_and_others(line+ofs);
	} else if((res=find_any_word(line+ofs, keywords_types, NR(keywords_types)))) {
		ofs+=res;
		/* look for int,long,unsigned,... */
		while(line[ofs]) {
			ofs+=eat_spaces_and_others(line+ofs);
			res=find_any_word(line+ofs, keywords_types, NR(keywords_types));
			if(!res) {
				break; /* found an ident */
			}
			ofs+=res;
		}
		ofs+=eat_spaces_and_others(line+ofs);
	} else {
		/* anything else like size_t, FILE*, etc */
		ofs+=res=next_ident(line+ofs); /* */
		if(!res) {
			fprintf(stderr, "Could not parse '%s'\n", line);
			return 0; /* could not find identifier */
		}
		ofs+=eat_spaces_and_others(line+ofs);
	}

	res=next_ident(line+ofs);
	if(!res) {
		fprintf(stderr, "Could not parse '%s'\n", line);
		return 0; /* could not find identifier */
	}
	if(res+1>max) {
		fprintf(stderr, "Could not parse '%s'\n", line);
		return 0; /* no room in key_out */
	}
	memcpy(key_out, line+ofs, res);
	key_out[res]=0;
	/*
	fprintf(stderr, "PROTO: '%s'\n", key_out);
	*/
	return 1; /* success */
}

int proto_init(void) {
	if(proto_h) {
		fprintf(stderr, "proto_init() already called\n");
		return 0;
	}
	proto_h=udb_open("proto.udb", parse_proto_key);
	if(!proto_h) {
		return 0;
	}
	return 1;
}

void proto_shutdown(void) {
	udb_close(proto_h);
}

int proto_result(char *dest, size_t max, const char *in) {
	unsigned res;
	char key[64];
	char proto[200], from[80], headers[100];

	while(isspace(*in)) in++;

	res=next_word(in);
	if(!res || res+1>64) {
		snprintf(dest, max, "Makes no sense.");
		return 0; /* not found */
	}

	memcpy(key,	in, res);
	key[res]=0;

	if(!udb_lookup(proto_h, key)) {
		snprintf(dest, max, "I don't know about '%s'.", key);
		return 0; /* not found */
	}

	if(!udb_read_field(proto_h, proto, sizeof proto)) {
		fprintf(stderr, "Error reading proto record for '%s'\n", key);
		snprintf(dest, max, "Error reading proto record for '%s'.", key);
		return 0; /* not found / error */
	}

	if(!udb_read_field(proto_h, from, sizeof from)) {
		snprintf(dest, max, "%s", proto);
	} else if(!udb_read_field(proto_h, headers, sizeof headers)) {
		snprintf(dest, max, "%s /* %s */", proto, from);
	} else {
		snprintf(dest, max, "%s %s /* %s */", headers, proto, from);
	}


	return 1;
}

/*** UNIT TEST ***/
#if 0
int main(int argc, char **argv) {
	char buf[256];
	int i;

	if(!proto_init()) {
		return 0;
	}

	if(argc<=1) {
		proto_result(buf, sizeof buf, "signal");
		printf("%s\n", buf);
	} else for(i=1;i<argc;i++) {
		proto_result(buf, sizeof buf, argv[i]);
		printf("%s\n", buf);
	}
	return 0;
}
#endif

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
