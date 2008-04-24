/* strhash.c */
/*
 * Copyright (c) 2008 Jon Mayo
 * This work may be modified and/or redistributed, as long as this license and
 * copyright text is retained. There is no warranty, express or implied.
 *
 * Last Updated: April 22, 2008
 */

#include <assert.h>
#include <stddef.h>
#include <ctype.h>
#include "strhash.h"

/** calculates a hash of a null terminated string */
unsigned strhash(const char *str) {
	unsigned h=0;

	assert(str!=NULL);

	while(*str) {
		/* same as: h = h * 65599 + *str++; */
		h=*str+++(h<<6)+(h<<16)-h;
	}
	return h;
}

/** calculates a hash of a null terminated string of any case */
unsigned strcasehash(const char *str) {
	unsigned h=0;

	assert(str!=NULL);

	while(*str) {
		/* same as: h = h * 65599 + *str++; */
		h=tolower(*str++)+(h<<6)+(h<<16)-h;
	}
	return h;
}

/** calculates a hash of a series of characters */
unsigned strnhash(const char *str, size_t len) {
	unsigned h=0;

	assert(str!=NULL);

	while(len) {
		/* same as: h = h * 65599 + *str++; */
		h=*str+++(h<<6)+(h<<16)-h;
		len--;
	}
	return h;
}
