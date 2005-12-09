LIBS:= -lm
LDFLAGS:=$(LIBS)
CFLAGS:= -std=gnu99 -Wall -O2 -pedantic # -DNDEBUG
CPPFLAGS:=

EXEC:=bot
SRCS:=bot.c calcdb.c users.c dcalc.c rpn.c rc.c md5crypt.c pQueue.c strcasestr.c
OBJS:=$(SRCS:%.c=%.o)

$(EXEC) : $(OBJS)

.PHONY: clean backup

clean:
	-$(RM) $(OBJS)

clean-all: clean
	-$(RM) $(EXEC) depend.mk *~

depend.mk : $(SRCS)
	$(CC) -MM $^ >$@

-include depend.mk

