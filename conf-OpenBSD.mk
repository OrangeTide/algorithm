# configuration for OpenBSD

# use the following for MD5 style passwords
# SRCS+=md5crypt.c users_md5.c

CPPFLAGS+= -DBUILD_STRCASESTR
# use the following for crypt() passwords
SRCS+= strcasestr.c users.c 

## end
