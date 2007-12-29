# configuration for Darwin/OS X

# requires gcc 3.x or later
CFLAGS+= -std=c99

#CPPFLAGS+= -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED

# use the following for MD5 style passwords
# SRCS+=md5crypt.c users_md5.c

# use the following for crypt() passwords
SRCS+= users.c 

## end
