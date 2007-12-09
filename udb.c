/* udb.c : micro database 
 * a simple text-base database for POSIX systems
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "udb.h"

#define HASH_SZ 4096 /* hash table size */
#define KEY_MAX 256 /* maximum keysize we support */

/** test if a udb_ent is empty */
#define UDB_IS_EMPTY(ent) (!(ent) || (ent)->key!=NULL)

struct udb_ent {
	char *key;
	fpos_t ofs;
	struct udb_ent *next;
};

struct udb_handle {
	FILE *f;
	char *filename;
	struct stat last_stat; /* result of last stat() call */
	struct udb_ent hash[HASH_SZ];
};

/** calculates a hash of a null terminated string */
static unsigned strhash(const char *str) {
	unsigned h=0;
	while(*str) {
		h=*str+++(h<<6)+(h<<16)-h;
	}
	return h;
}

/** calculates a hash of a series of characters */
static unsigned strnhash(const char *str, size_t len) {
	unsigned h=0;
	while(len) {
		h=*str+++(h<<6)+(h<<16)-h;
		len--;
	}
	return h;
}

/** removed newline from end of a string 
 * return non-zero if a newline was found */
static int scrubnl(char *s) {
	s=strrchr(s, '\n');
	if(s) {
		*s=0;
		return 1;
	} else {
		return 0;
	}
}

static void update_stat(struct udb_handle *h) {
	if(fstat(fileno(h->f), &h->last_stat)) {
		perror(h->filename);
		/* ignore the error */
	}
}

/** return non-zero if file stat has changed 
 */
static int file_has_changed(struct udb_handle *h) {
	struct stat st;
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
	if(file_has_changed(h)) {
		fprintf(stderr, "Refreshing DB for %s\n", h->filename);
		udb_refresh(h);
	}
}

/** uses filename for backing of a database, and parse_key() call back is
 * called at any time for the first line after a record separator.
 *
 * parse_key callback takes in line, and writes to key_out (up to max
 * characters) as the unique key for this record in the database.
 */
struct udb_handle *udb_open(const char *filename, int (*parse_key)(const char *line, char *key_out, size_t max)) {
	struct udb_handle *ret;
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
	memset(ret->hash, 0, sizeof ret->hash);
	memset(&ret->last_stat, 0, sizeof ret->last_stat);
	update_stat(ret);
	return ret;
}

/** force a refresh */
void udb_refresh(struct udb_handle *h) {
}

/** look up an entry in the hash and position the database cursor to it.
 * return 0 on failure (cursor is not repositioned)
 * return non-zero on success (cursor points to start of record)
 */
int udb_lookup(struct udb_handle *h, const char *key) {
	refresh_if_changed(h);
	/* TODO: lock while reading */

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

	return 1; /* success */
}

int main(int argc, char **argv) {
	return 0;
}
