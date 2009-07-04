TARGET_LIB = dxlibp.a
OBJS = dxlibp.o dxpfileio.o dxpvram.o dxpinput.o dxpgu.o dxpgu_admin.o dxpgu_load.o dxppng.o dxpjpeg.o dxpdebug.o zenkaku.o
OBJS += dxposk.o dxpunicode.o dxpmusic_wavedata.o dxpmusic_wavestream.o dxpmusic_mp3stream.o
OBJS += dxpmusic2.o libccc.o intraFont.o dxpgu_intrafont0.31.o dxppsplib.o

INCDIR = 
CFLAGS = -O3 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

