cc=gcc
all:szbmain.o unpack.o pack.o
	cc -o leszb  szbmain.o unpack.o pack.o
szbmain.o:szbmain.c unpack.c pack.c szbtools.h
	cc -c unpack.c
	cc -c pack.c
	cc -c szbmain.c
clean:
	rm *.o szbtools
