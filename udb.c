/* udb.c : micro database 
 * a simple text-base database for POSIX systems
 */
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "udb.h"

#define HASH_SZ 4096 /* hash table size - does not have to be power of 2 */
#define KEY_MAX 256 /* maximum keysize we support */
#define LINE_MAX 16384 /* maximum line length */

/** test if a udb_ent is empty */
#define UDB_IS_EMPTY(ent) (!(ent) || (ent)->key==NULL)

struct udb_ent {
	char *key;
	fpos_t ofs;
	struct udb_ent *next; /* linked list for hash collisions */
};

struct udb_handle {
	FILE *f;
	char *filename;
	/* callback is used in udb_refresh() to turn the first line into a key */
	int (*parse_key_cb)(const char *line, char *key_out, size_t max);
	struct stat last_stat; /* result of last stat() call */
	struct udb_ent hash[HASH_SZ]; /* note: first level are not pointers */
};

/** calculates a hash of a null terminated string */
static unsigned strhash(const char *str) {
	unsigned h=0;

	assert(str!=NULL);

	while(*str) {
		/* same as: h = h * 65599 + *str++; */
		h=*str+++(h<<6)+(h<<16)-h;
	}
	return h;
}

#if 0 /* not used */
/** calculates a hash of a series of characters */
static unsigned strnhash(const char *str, size_t len) {
	unsigned h=0;

	assert(str!=NULL);

	while(len) {
		/* same as: h = h * 65599 + *str++; */
		h=*str+++(h<<6)+(h<<16)-h;
		len--;
	}
	return h;
}
#endif

/** removed newline from end of a string 
 * return non-zero if a newline was found */
static int scrubnl(char *s) {
	assert(s!=NULL);
	s=strrchr(s, '\n');
	if(s) {
		*s=0;
		return 1;
	} else {
		return 0;
	}
}

static void update_stat(struct udb_handle *h) {
	assert(h!=NULL);

	if(fstat(fileno(h->f), &h->last_stat)) {
		perror(h->filename);
		/* ignore the error */
	}
}

/** return non-zero if file stat has changed 
 */
static int file_has_changed(struct udb_handle *h) {
	struct stat st;

	assert(h!=NULL);

	if(stat(h->filename, &st)) {
		perror(h->filename);
		return 1; /* treat error as if the file has changed */
	}

	if(st.st_ino!=h->last_stat.st_ino || st.st_mtime!=h->last_stat.st_mtime) {
		return 1; /* something has changed */
	}
	return 0; /* no change */
}

/** checks if the DB file has been altered since we last loaded,
 * then reloads the database. */
static void refresh_if_changed(struct udb_handle *h) {
	assert(h!=NULL);

	if(file_has_changed(h)) {
		fprintf(stderr, "Refreshing DB for %s\n", h->filename);
		udb_refresh(h);
	}
}

/** free and clear hash table */
static void free_hash(struct udb_handle *h) {
	int i;

	assert(h!=NULL);

	for(i=0;i<HASH_SZ;i++) {
		struct udb_ent *curr, *next;
		/* first level of hash entries are not pointers */
		free(h->hash[i].key);
		/* the chained collision entries are just a linked list */
		for(curr=h->hash[i].next;curr;curr=next) {
			next=curr->next;
			free(curr->key);
			free(curr);	
		}
		/* clear out entry */
		memset(&h->hash[i], 0, sizeof h->hash[i]);
	}
}

/** generic parser function. just take the first word in the line */
static int generic_parse_key(const char *line, char *key_out, size_t max) {
	assert(line!=NULL);

	while(max>0) {
		if(!*line || isspace(*line)) {
			*key_out=0; /* null terminator */
			return 1; /* success */
		}
		*key_out=*line; /* copy data */
		key_out++;
		max--;
		line++;
	}
	return 0; /* failure */
}

static int find_dup(struct udb_ent *head, const char *key) {
	struct udb_ent *curr;

	assert(key!=NULL);

	/* search for duplicate */
	for(curr=head;!UDB_IS_EMPTY(curr);curr=curr->next) {
		assert(curr!=NULL);
		assert(curr->key!=NULL);
		if(strcmp(key, curr->key)==0) {
			return 1; /* found duplicate */
		}
	}
	return 0; /* no duplicate found */
}

static int add_hash_entry(struct udb_handle *h, const char *key, fpos_t ofs) {
	struct udb_ent *ent;
	unsigned new_hash;

	assert(h!=NULL);
	assert(key!=NULL);

	/* add new record */
	new_hash=strhash(key)%HASH_SZ;
	ent=&h->hash[new_hash];

	if(find_dup(ent, key)) {
		fprintf(stderr, "Duplicate key '%s' found in DB file %s\n", key, h->filename);
		return 0; /* failure */
	}
		
	/* at the end of this bit, ent points to the entry to fill in */
	/* right now ent points to the h->hash[new_hash] */	
	if(!UDB_IS_EMPTY(ent)) { /* detected collision, create linked list entry */
		struct udb_ent *new_ent;
		new_ent=malloc(sizeof *new_ent);
		/* push onto the list */
		new_ent->next=ent->next; /* save start of linked list */
		ent->next=new_ent; /* push as the new start */
		ent=new_ent; /* now ent is the entry to fill in below */
	}

	/* fill in ent */
	ent->key=strdup(key);
	ent->ofs=ofs;

	return 1; /* success */
}

/** uses filename for backing of a database, and parse_key() call back is
 * called at any time for the first line after a record separator.
 *
 * parse_key_cb callback takes in line, and writes to key_out (up to max
 * characters) as the unique key for this record in the database.
 *
 * if parse_key_cb is NULL then use generic_parse_key() function.
 */
struct udb_handle *udb_open(const char *filename, int (*parse_key_cb)(const char *line, char *key_out, size_t max)) {
	struct udb_handle *ret;

	assert(filename!=NULL);

	ret=malloc(sizeof *ret);
	if(!ret) {
		perror("malloc()");
		return 0;
	}
	ret->f=fopen(filename, "r");
	if(!ret->f) {
		perror(filename);
		free(ret);
		return 0; /* failed */
	}
	ret->filename=strdup(filename);
	ret->parse_key_cb=parse_key_cb?parse_key_cb:generic_parse_key;
	memset(ret->hash, 0, sizeof ret->hash);
	memset(&ret->last_stat, 0, sizeof ret->last_stat);
	/* (uncomment to force refresh on load)
	update_stat(ret);
	udb_refresh(ret);
	*/
	return ret;
}

/** force a refresh */
void udb_refresh(struct udb_handle *h) {
	char line[LINE_MAX];
	char key[KEY_MAX];
	int record_count=0;

	assert(h!=NULL);
	assert(h->f!=NULL);

	fclose(h->f);
	free_hash(h); /* throw out old hash table */
	/* reopen the file */
	h->f=fopen(h->filename, "r");
	if(!h->f) {
		perror(h->filename);
		fprintf(stderr, "Fatal error in DB for %s!\n", h->filename);
		return;
	}

	update_stat(h);

	/* TODO: lock the file before we read it in */

	do{
		fpos_t tmp_ofs;

		/* save the record's start position */
		if(fgetpos(h->f, &tmp_ofs)) {
			perror(h->filename);
			fprintf(stderr, "Fatal error in DB for %s!\n", h->filename);
			return;
		}

		/* read first line. this has the key */
		if(udb_read_field(h, line, LINE_MAX)) {

			/* parse the first line */
			if(h->parse_key_cb(line, key, KEY_MAX)) {
				add_hash_entry(h, key, tmp_ofs);
				record_count++;
			} else {
				fprintf(stderr, "Key parse error in DB file %s (ignoring record)\n", h->filename);
			}

			/* swallow remaining records, looking for next record */
			while(udb_ignore_field(h)) {
				/* do nothing */
			}
		}
	/* repeat until there is an error or EOF */
	} while(!feof(h->f) && !ferror(h->f));

	fprintf(stderr, "Loaded %d records from DB %s\n", record_count, h->filename);
}

/** look up an entry in the hash and position the database cursor to it.
 * return 0 on failure (cursor is not repositioned)
 * return non-zero on success (cursor points to start of record)
 */
int udb_lookup(struct udb_handle *h, const char *key) {
	struct udb_ent *curr;
	unsigned new_hash;

	assert(h!=NULL);
	assert(key!=NULL);

	refresh_if_changed(h);

	new_hash=strhash(key)%HASH_SZ;

	for(curr=&h->hash[new_hash];!UDB_IS_EMPTY(curr);curr=curr->next) {
		assert(curr!=NULL);
		assert(curr->key!=NULL);
		if(strcmp(curr->key, key)==0) {
			/* found the entry */
			if(fsetpos(h->f, &curr->ofs)) {
				perror(h->filename);
				fprintf(stderr, "Fatal error in DB for %s!\n", h->filename);
				return 0; /* failure */
			}
			return 1; /* success */
		}
	}
	return 0; /* failure */
}

/** read next field in the current record 
 * return 0 when end of record is reached */
int udb_read_field(struct udb_handle *h, char *buf, size_t len) {
	assert(h!=NULL);
	assert(len>2);
	if(!fgets(buf, len, h->f)) {
		if(ferror(h->f)) {
			perror(h->filename);
		}
		return 0; /* EOF or Error */
	}

	if(!scrubnl(buf)) {
		fprintf(stderr, "Truncated record in %s\n", h->filename);
	}
	
	if(buf[0]=='%' && buf[1]==0) {
		return 0; /* end of record */
	}

	/* TODO: lock while reading records. need an api to report when finished
	 * with a record, or require that records always be fully read(bad idea) */
	return 1; /* success */
}

/** like udb_read_field, but don't save the data */
int udb_ignore_field(struct udb_handle *h) {
	char line[LINE_MAX];
	/* TODO: this could be more sophesticated */
	return udb_read_field(h, line, LINE_MAX);
}

/** close and free all data */
void udb_close(struct udb_handle *h) {
	free_hash(h);
	fclose(h->f);
	h->f=0;
	free(h->filename);
	h->filename=0;
	free(h);
}

/* unit test */
#if 1
void demo(struct udb_handle *h, const char *word) {
	if(udb_lookup(h, word)) {
		char word[64], def[256];
		int has_def;
		fprintf(stderr, "Found Entry!\n");
		if(udb_read_field(h, word, sizeof word)) {
			has_def=udb_read_field(h, def, sizeof def);

			printf("word=%s\n", word);
			if(has_def) {
				printf("def=\"%s\"\n", def);
			} else {
				printf("def=NONE\n");
			}
		}
	} else {
		fprintf(stderr, "Entry not found: %s\n", word);
	}
}

int main(int argc, char **argv) {
	struct udb_handle *h;
	int i;


	h=udb_open("dict.udb", 0);
	if(!h) {
		fprintf(stderr, "I need something open\n");
		return 0;
	}

	/* udb_refresh(h); *//* force a load */

	if(argc==1) {
		demo(h, "anneal");
		demo(h, "xyz");
		demo(h, "piglet");
	} else for(i=1;i<argc;i++) {
		demo(h, argv[i]);
	}

	udb_close(h);
	return 0;
}

#endif
