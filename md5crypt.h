#ifndef MD5CRYPT_H
#define MD5CRYPT_H

/* md5crypt.c */
const char *crypt_md5 ( const char *pw , const char *salt );
const char *saltgen_md5 ( unsigned int seed );
int compare_md5 ( const char *passwd , const char *crypt );
#endif /* MD5CRYPT_H */

/*****************************----end code----*****************************/
// vi: noet sts=0 ts=4 sw=4
