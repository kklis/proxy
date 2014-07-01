all:
	gcc -O2 -s -o proxy proxy.c
tomato:
	mipsel-uclibc-gcc -O2 -s -o proxy proxy.c
openwrt:
	mipsel-linux-uclibc-gcc -O2 -s -o proxy proxy.c
clean:
	rm -rf proxy proxy.exe
