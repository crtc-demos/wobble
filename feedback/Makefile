
	CC =		kos-cc
	CFLAGS =	-O3
	STRIP =		kos-strip
	DEPFLAGS =	-DDREAMCAST_KOS -D_arch_dreamcast
	GENROMFS =	$(KOS_GENROMFS)

	OBJS =		perlin.o feedback.o romdisk.o
	ROMDISK =	romdisk.img

	TARGET =	feedback.elf

	SRC =		feedback.c

.PHONY:	.depend

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

cleaner:	clean
	rm -f *.d

run:	$(TARGET)
	/usr/local/bin/dc-tool -b 115200 -x $(TARGET)

$(TARGET):	$(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) $(OBJS) -lgl -lpng -lz -lm -lkosutils $(KOS_LIBS)
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
	$(CC) $(CFLAGS) -MM $< > $@

.depend:	Makefile.dc $(SRC)
	$(KOS_CC) $(DEPFLAGS) $(INCLUDE) -MM $(SRC) > .depend

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleaner)
include $(OBJS:.o=.d)
endif
endif

