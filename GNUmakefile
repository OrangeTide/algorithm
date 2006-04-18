LIBS:= -lm
CFLAGS:= -Wall -O2 -pedantic
# enable to remove debugging tests
# CFLAGS+= -DNDEBUG

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

