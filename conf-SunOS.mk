# configuration for SunOS

CC:= gcc
CFLAGS:= -Wall -O2 -pedantic
CPPFLAGS+= -D_GNU_SOURCE -DBUILD_STRCASESTR -DBUILD_ISINF
LDLIBS:= -lm -lsocket -lnsl

# use the following for MD5 style passwords
# SRCS+=md5crypt.c users_md5.c

# use the following for crypt() passwords
SRCS+= strcasestr.c users.c isinf.c
LDLIBS+= -lcrypt

## end
