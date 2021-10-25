CFLAGS=-O2 -std=c99 -Wall
OPTFLAGS=-s -DUSE_SPLICE
LDFLAGS=

all:
	gcc $(CFLAGS) $(OPTFLAGS) -o proxy proxy.c $(LDFLAGS)
tomato:
	mipsel-uclibc-gcc $(CFLAGS) $(OPTFLAGS) -o proxy proxy.c $(LDFLAGS)
openwrt:
	mipsel-linux-uclibc-gcc $(CFLAGS) $(OPTFLAGS) -o proxy proxy.c $(LDFLAGS)
backfire:
	mipsel-openwrt-linux-uclibc-gcc $(CFLAGS) $(OPTFLAGS) -o proxy proxy.c $(LDFLAGS)
darwin:
	gcc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
freebsd:
	cc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
clean:
	rm -f proxy proxy.exe
