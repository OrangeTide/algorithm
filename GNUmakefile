LIBS:= -lm
CFLAGS:= -Wall -Wextra -O2 -pedantic
CFLAGS+=-ggdb
# enable to remove debugging tests
# CFLAGS+= -DNDEBUG

EXEC:=bot
SRCS:=\
	bot.c \
	calcdb.c \
	calcnotfound.c \
	dcalc.c \
	rc.c \
	rpn.c \
	strcasestr.c \
	udb.c \
	wcalc.c

include conf-$(shell uname -s).mk

OBJS:=$(SRCS:%.c=%.o)

$(EXEC) : LDFLAGS:=$(LIBS)
$(EXEC) : $(OBJS)

.PHONY: clean clean-all backup

clean:
	-$(RM) $(OBJS)

clean-all: clean
	-$(RM) $(EXEC) depend.mk *~

depend.mk : $(SRCS)
	$(CC) $(CPPFLAGS) -MM $^ >$@

-include depend.mk

