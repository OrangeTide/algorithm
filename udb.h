#ifndef UDB_H
#define UDB_H
#include <stddef.h>
struct udb_handle;
struct udb_handle *udb_open(const char *filename, int (*parse_key_cb)(const char *line, char *key_out, size_t max));
void udb_refresh(struct udb_handle *h);
int udb_lookup(struct udb_handle *h, const char *key);
int udb_read_field(struct udb_handle *h, char *buf, size_t len);
int udb_ignore_field(struct udb_handle *h);
void udb_close(struct udb_handle *h);
#endif

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
