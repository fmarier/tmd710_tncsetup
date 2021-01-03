APP = tmd710_tncsetup
OBJS = tmd710_tncsetup.o
CFLAGS = -O2 -Wall -Werror -fstack-protector-strong -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIE
LDFLAGS = -Wl,-z,now -Wl,-z,relro -Wl,-pie -Wl,--hash-style=gnu

prefix ?= /usr/local

all: $(APP)

$(APP): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -g -o $@ -c $< $(CFLAGS)

install: all
	mkdir -p $(DESTDIR)$(prefix)/bin/
	install -m 755 $(APP) $(DESTDIR)$(prefix)/bin/$(APP)

uninstall:
	rm $(DESTDIR)$(prefix)/bin/$(APP)

clean:
	rm -f $(APP) *.o

.PHONY: clean
