
	CC = 		kos-cc
	STRIP =		$(KOS_STRIP)
	CFLAGS =	-O2 -Wall
	INCLUDE =	-I../include
	DEPFLAGS =
	LIBS =		-lgl -lpng -lz -lm -lkosutils
	GENROMFS =	$(KOS_GENROMFS)

	OBJS =	cube.o rom.o

	TARGET = cube.elf

	SRC =	cube.c

	ROMDISK = rom.img

all:	$(TARGET)

run:	$(TARGET)
	$(STRIP) $(TARGET)
	dc-tool -b 115200 -x $(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: cleaner
cleaner: clean
	rm -f *.d

$(TARGET):	$(OBJS) $(ROMDISK)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

.PHONY: $(ROMDISK)
$(ROMDISK):
	$(GENROMFS) -f $@ -d imagedir

rom.o:	$(ROMDISK)
	$(KOS_BASE)/utils/bin2o/bin2o rom.img rom rom.o

rom.d:
	touch $@

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.d:	%.c
	$(CC) $(CFLAGS) $(INCLUDE) -MM $< > $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleaner)
include $(OBJS:.o=.d)
endif
endif

