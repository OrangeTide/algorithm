LIBS:= -lm
CFLAGS:= -Wall -O2 -pedantic
# CFLAGS+= -DNDEBUG
CFLAGS+= -std=gnu99 
CPPFLAGS:= -D_XOPEN_SOURCE=500 -D_XOPEN_SOURCE_EXTENDED

EXEC:=bot
SRCS:=\
	bot.c \
	calcdb.c \
	dcalc.c \
	wcalc.c \
	rpn.c \
	rc.c \
	strcasestr.c \
	calcnotfound.c

# use the following for MD5 style passwords
# SRCS+=md5crypt.c users_md5.c

# use the following for crypt() passwords
SRCS+= users.c 
LIBS+= -lcrypt

OBJS:=$(SRCS:%.c=%.o)

$(EXEC) : LDFLAGS:=$(LIBS)
$(EXEC) : $(OBJS)

.PHONY: clean backup

clean:
	-$(RM) $(OBJS)

clean-all: clean
	-$(RM) $(EXEC) depend.mk *~

depend.mk : $(SRCS)
	$(CC) $(CPPFLAGS) -MM $^ >$@

-include depend.mk

