TARGET = FontSample
OBJS = main.o

CFLAGS = -O3 -G0 -Wall 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
#PSP_FW_VERSION=390
LIBS = ../../DXLibraryPortable/dxlibp.a -lpspaudiolib -lpspaudio -lpspmpeg -lpspaudiocodec -lpspgu -lpng -lz -lm -lpsppower -lpsprtc -lpspgum -lpspjpeg -lpspgu
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FontSample
PSP_EBOOT_ICON =
PSP_EBOOT_PIC1 =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
