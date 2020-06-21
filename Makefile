APP = tmd710_tncsetup
OBJS = tmd710_tncsetup.o
CFLAGS = -O2 -Wall -Werror -fstack-protector-strong -Wformat -Wformat-security -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIE
LDFLAGS = -Wl,-z,now -Wl,-z,relro -Wl,-pie -Wl,--hash-style=gnu

all: $(APP)

$(APP): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(APP) *.o

.PHONY: clean
