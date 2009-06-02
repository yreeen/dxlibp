TARGET = Test
OBJS = main.o

#OBJS += ../DXLibraryPortable/dxlibp.o ../DXLibraryPortable/dxpfileio.o ../DXLibraryPortable/dxpvram.o ../DXLibraryPortable/dxpinput.o ../DXLibraryPortable/dxpgu.o ../DXLibraryPortable/dxpgu_draw.o ../DXLibraryPortable/dxpgu_load.o ../DXLibraryPortable/dxpdebug.o ../DXLibraryPortable/zenkaku.o
#OBJS += ../DXLibraryPortable/dxposk.o ../DXLibraryPortable/dxpunicode.o ../DXLibraryPortable/dxpmusic_wavedata.o ../DXLibraryPortable/dxpmusic_wavestream.o ../DXLibraryPortable/dxpmusic_mp3stream.o
#OBJS += ../DXLibraryPortable/dxpmusic2.o ../DXLibraryPortable/libccc.o ../DXLibraryPortable/intraFont.o ../DXLibraryPortable/dxpgu_intrafont0.31.o
#OBJS += ../DXLibraryPortable/dxaddfun.o



INCDIR = 
CFLAGS = -O2 -G0 -Wall# -pg
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

LIBS +=
LIBS += ../DXLibraryPortable/dxlibp.a 
LIBS += -lpspgum -lpspgu -lpng -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lstdc++ -lpsputility -lpspjpeg# -lpspprof#-ljpeg  

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = DXライブラリPortable

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
