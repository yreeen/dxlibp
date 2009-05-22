TARGET = Test
OBJS = main.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

LIBS = ../DXLibralyPortable/dxlibp.a -lpspgum -lpspgu -lpng -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lstdc++ -lpsputility -lpspjpeg #-ljpeg 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = DXライブラリPortable

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
