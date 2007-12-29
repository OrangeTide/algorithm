# configuration for Cygwin

# requires gcc 3.x or later
# CFLAGS+= -std=gnu99 -mno-cygwin
CFLAGS+= -std=gnu99

CPPFLAGS+= -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED -DBUILD_STRCASESTR

# use the following for MD5 style passwords
# SRCS+=md5crypt.c users_md5.c

# use the following for crypt() passwords
SRCS+= strcasestr.c users.c
LIBS+=-lcrypt
LDLIBS=-lcrypt

## end
