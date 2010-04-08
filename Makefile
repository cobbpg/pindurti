OPTIONS=-O1 -Wall -Wextra -fomit-frame-pointer -fmessage-length=0
CFLAGS=$(OPTIONS)
CRES=windres

OBJECTS=\
	hw/emucore.o\
	hw/hwcore.o\
	hw/ti82.o\
	hw/ti83.o\
	hw/ti83p.o\
	hw/files.o\
	hw/lcd.o\
	hw/z80.o\
	hw/flash.o\
	misc/cmd.o\
	misc/gif.o\
	debug/debug.o\
	debug/dz80.o\
	debug/dlcd.o\
	debug/dti82.o\
	debug/dti83.o\
	debug/dti83p.o\
	debug/dtrap.o\
	win32/debug/dwin.o\
	win32/debug/dtree.o\
	win32/debug/cwin.o\
	win32/debug/ckeyval.o\
	win32/debug/cdisasm.o\
	win32/debug/cz80.o\
	win32/debug/clcddata.o\
	win32/debug/cmemhex.o\
	win32/debug/cvat.o\
	win32/debug/clinkhis.o\
	win32/debug/clogger.o\
	win32/debug/common.o\
	win32/debug/kvdata.o\
	win32/pindurti.ro\
	win32/main.o

all: $(OBJECTS)
	$(CC) $(CFLAGS) -mwindows -s -o pindurti.exe $(OBJECTS) -lwinmm -lcomctl32
	upx pindurti.exe

send: win32/send.c
	$(CC) $(CFLAGS) -s -o send win32/send.c

clean:
	-rm pindurti.exe $(OBJECTS)

include Make-includes

.c.o:
	${CC} ${CFLAGS} -o $@ -c $<

.rc.ro:
	cd $(<D);\
	${CRES} $(<F) $(@F)

.SUFFIXES: .rc .ro

