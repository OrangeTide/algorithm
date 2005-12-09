#include <ctype.h>
#include <stdlib.h>
#include "strcasestr.h"

/*
* case-insensitive version of strstr()
*/
const char *
strcasestr ( const char *haystack, const char *needle)
{
	const char *h;
	const char *n;

	h = haystack;
	n = needle;
	if( !*n )
		return haystack;
	while( *haystack ) {
		if( tolower((unsigned char) *h) == tolower((unsigned char) *n) ) {
			h++;
			n++;
			if ( !*n )
				return haystack;
		} else {
			h = ++haystack;
			n = needle;
		}
	}
	return NULL;
} 
