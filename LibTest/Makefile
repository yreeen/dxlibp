TARGET = Test
OBJS = main.o

#OBJS += ../DXLibralyPortable/dxlibp.o ../DXLibralyPortable/dxpfileio.o ../DXLibralyPortable/dxpvram.o ../DXLibralyPortable/dxpinput.o ../DXLibralyPortable/dxpgu.o ../DXLibralyPortable/dxpgu_draw.o ../DXLibralyPortable/dxpgu_load.o ../DXLibralyPortable/dxpdebug.o ../DXLibralyPortable/zenkaku.o
#OBJS += ../DXLibralyPortable/dxposk.o ../DXLibralyPortable/dxpunicode.o ../DXLibralyPortable/dxpmusic_wavedata.o ../DXLibralyPortable/dxpmusic_wavestream.o ../DXLibralyPortable/dxpmusic_mp3stream.o
#OBJS += ../DXLibralyPortable/dxpmusic2.o ../DXLibralyPortable/libccc.o ../DXLibralyPortable/intraFont.o ../DXLibralyPortable/dxpgu_intrafont0.31.o
#OBJS += ../DXLibralyPortable/dxaddfun.o



INCDIR = 
CFLAGS = -O2 -G0 -Wall# -pg
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

LIBS +=
LIBS += ../DXLibralyPortable/dxlibp.a 
LIBS += -lpspgum -lpspgu -lpng -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lstdc++ -lpsputility -lpspjpeg# -lpspprof#-ljpeg  

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = DXライブラリPortable

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
