
	CC =		kos-cc
	CFLAGS =	-O0 -g
	INCLUDE =	-I../include
	STRIP =		kos-strip
	DEPFLAGS =	-DDREAMCAST_KOS -D_arch_dreamcast
	GENROMFS =	$(KOS_GENROMFS)

	OBJS =		draw_torus.o voronoi.o wobble_tube.o waves.o timing.o \
			romdisk.o
	ROMDISK =	romdisk.img

	TARGET =	demo.elf

	SRC =		draw_torus.c voronoi.c wobble_tube.c waves.c timing.c

.PHONY:	.depend

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

cleaner:	clean
	rm -f *.d

run:	$(TARGET)
	/usr/local/bin/dc-tool -x $(TARGET)

$(TARGET):	$(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) $(OBJS) -L../libcompass -lgl -lpng -lkmg -lz -lcompass -lkosutils -lm $(KOS_LIBS)
	$(STRIP) $(TARGET)

.PHONY: $(ROMDISK)
$(ROMDISK): imagedir_clean
	$(GENROMFS) -f $@ -d imagedir_clean

.PHONY: imagedir_clean
imagedir_clean:
	rm -rf imagedir_clean
	rsync -Pav --exclude='.svn' imagedir/ imagedir_clean

romdisk.o:	$(ROMDISK)
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

romdisk.d:
	touch $@

%.o:    %.c
	$(CC) $(CFLAGS) $(INCLUDE) -DDREAMCAST_KOS -c $< -o $@

%.d:    %.c
	$(CC) $(CFLAGS) $(INCLUDE) -MM $< > $@

.depend:	Makefile.dc $(SRC)
	$(KOS_CC) $(DEPFLAGS) $(INCLUDE) -MM $(SRC) > .depend

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleaner)
include $(OBJS:.o=.d)
endif
endif

