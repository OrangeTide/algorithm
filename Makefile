LIBS:= -lm
CFLAGS:= -std=gnu99 -Wall -O2 -pedantic # -DNDEBUG
CPPFLAGS:=

EXEC:=bot
SRCS:=bot.c calcdb.c dcalc.c rpn.c rc.c strcasestr.c

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
	$(CC) -MM $^ >$@

-include depend.mk

