	CFLAGS = -O2

C=edgelist.c geometry.c heap.c main.c memory.c output.c voronoi.c 
O=edgelist.o geometry.o heap.o main.o memory.o output.o voronoi.o

all: voronoi

tt: voronoi t
	voronoi -t <t >tt
voronoi: $O
	cc -O2 -o voronoi $O -lm
$O:vdefs.h

.PHONY:	clean
clean:
	rm -f *.o voronoi

voronoi.tar : $C vdefs.h Makefile Doc t
	tar -cf voronoi.tar $C vdefs.h Makefile Doc t

mailable:	$C vdefs.h Makefile t
	bundle $C vdefs.h Makefile t > mailable
