all:
	gcc -O2 -s -o proxy proxy.c
mips:
	mipsel-uclibc-gcc -O2 -s -o proxy proxy.c
clean:
	rm -rf proxy proxy.exe