CC		= cc
CFLAGS		= -Wall -O2 -std=c99 -I/usr/local/include 
INSTALL		= install
LDFLAGS		= -L/usr/local/lib -llowdown -lm
MKDIR		= mkdir -p
PREFIX		?= /usr/local
PROG		= ana
RM		= rm -f

install: ana
	$(MKDIR) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -o root $(PROG) $(DESTIR)$(PREFIX)/bin

ana:
	$(CC) $(CFLAGS) -o $(PROG) $(LDFLAGS) ana.c

.PHONY: clean install

clean:
	$(RM) *~ *.o *.core core $(PROG)
