CC     = gcc
CFLAGS = -O2 -Wall
#CFLAGS = -Wall -g
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include -I/usr/include/cairo -I/usr/include/json
DESTDIR= /usr
MANDIR = /usr/share/man
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext -lm -lcairo -lcurl -lpthread -ljson-c

# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket -lnsl

OBJS   = wmHGmail.o \
         xutils_cairo.o \
	 goauth.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmHGmail.o wmHGmail

wmHGmail.o: wmHGmail_master.xpm wmHGmail_mask.xbm
wmHGmail:	$(OBJS) 
	$(CC) $(COPTS) $(SYSTEM) -o wmHGmail $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmHGmail

install:: wmHGmail
	install -c -s -m 0755 wmHGmail $(DESTDIR)/bin
	install -c -m 0644 wmHGmail.1 $(MANDIR)/man1 

