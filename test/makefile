CFLAGS=-Wall -pedantic
CPPFLAGS=-I../

all : test-match test-argv wcalc base26

test-match : ../match.c test-match.c

test-argv : ../argv.c test-argv.c

base26 : ../base26.c
	$(CC) $(CPPFLAGS) -DBASE26_MAIN $(CFLAGS) $(LDFLAGS) -lm -o $@ $^

wcalc : ../wcalc.c 
	$(CC) $(CPPFLAGS) -DWCALC_MAIN $(CFLAGS) $(LDFLAGS) -lm -o $@ $^

clean :
	$(RM) wcalc test-argv test-match base26
