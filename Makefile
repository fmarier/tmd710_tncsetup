APP = tmd710_tncsetup
VERSION := $(shell grep 'VERSION "' $(APP).c  | cut -d\" -f2)
MANPAGE = $(APP).1
OBJS = tmd710_tncsetup.o
CFLAGS ?= -O2 -Wall -Werror -pedantic -fstack-protector-strong -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIE
LDFLAGS ?= -Wl,-z,now -Wl,-z,relro -Wl,-pie -Wl,--hash-style=gnu

prefix ?= /usr/local
bindir = $(DESTDIR)$(prefix)/bin/
mandir = $(DESTDIR)$(prefix)/share/man/man1/

all: $(APP) $(MANPAGE).gz

$(APP): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

$(MANPAGE).gz: $(MANPAGE)
	gzip --keep --best $<

%.o: %.c
	$(CC) -g -o $@ -c $< $(CFLAGS)

install: all
	mkdir -p $(bindir)
	install -m 755 $(APP) $(bindir)$(APP)
	mkdir -p $(mandir)
	install -m 644 $(MANPAGE).gz $(mandir)$(MANPAGE).gz

uninstall:
	rm $(bindir)$(APP)
	rm $(mandir)$(MANPAGE).gz

dist:
	git commit -a -m "Bump version for release"
	git tag -s $(APP)-$(VERSION) -m "$(VERSION) release"

clean:
	rm -f $(APP) $(MANPAGE).gz *.o

.PHONY: clean dist
