CFLAGS=-O2 -std=c99 -s -Wall -DUSE_SPLICE
LDFLAGS=

all:
	gcc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
tomato:
	mipsel-uclibc-gcc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
openwrt:
	mipsel-linux-uclibc-gcc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
backfire:
	mipsel-openwrt-linux-uclibc-gcc $(CFLAGS) -o proxy proxy.c $(LDFLAGS)
clean:
	rm -f proxy proxy.exe
