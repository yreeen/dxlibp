TARGET_LIB = dxlibp.a
OBJS = dxlibp.o dxpfileio.o dxpvram.o dxpinput.o dxpgu.o dxpgu_load.o dxpdebug.o zenkaku.o dxposk.o dxpunicode.o dxpmusic_wavedata.o dxpmusic_wavestream.o dxpmusic_mp3stream.o dxpmusic2.o libccc.o intraFont.o dxpgu_intrafont0.31.o


INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

