
	CC =		kos-cc
	CFLAGS =	-O3
	INCLUDE =	-Iinclude
	STRIP =		kos-strip
	DEPFLAGS =	-DDREAMCAST_KOS -D_arch_dreamcast
	GENROMFS =	$(KOS_GENROMFS)
	DCTOOL =	/usr/local/bin/dc-tool

	OBJS =
	ROMDISK =	romdisk.img

	TARGET =	demo.elf

	ARFLAGS =	cr

	SRC =		

	COMPASS_SRC =	libcompass/fakephong.c libcompass/envmap_dual_para.c \
			libcompass/object.c libcompass/palette.c \
			libcompass/restrip.c

	COMPASS_OBJ =	libcompass/fakephong.o libcompass/envmap_dual_para.o \
			libcompass/object.o libcompass/palette.o \
			libcompass/restrip.o
	LIBCOMPASS =	libcompass/libcompass.a

.PHONY:	.depend

#all:	$(TARGET)
all:	libcompass/libcompass.a

clean:
	rm -f $(OBJS) $(TARGET)

cleaner:	clean
	rm -f *.d

run:	$(TARGET)
	$(DCTOOL) -b 115200 -x $(TARGET)

$(TARGET):	$(OBJS) $(LIBCOMPASS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) $(OBJS) -Llibcompass -lcompass -lgl -lpng -lz -lm -lkosutils $(KOS_LIBS)
	$(STRIP) $(TARGET)

$(LIBCOMPASS):	$(COMPASS_OBJ)
	$(KOS_AR) $(ARFLAGS) $(LIBCOMPASS) $(COMPASS_OBJ)

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

