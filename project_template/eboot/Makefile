TARGET = template_eboot
OBJS = main.o 

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)


LIBDIR =
LDFLAGS =

LIBS = 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = template_eboot

#PSP_EBOOT_ICON = #xmb icon 140x80 png
#PSP_EBOOT_PIC1 = #xmb background 480x272 png
#PSP_EBOOT_SND0 = #xmb bgm atrac3
#PSP_EBOOT_ICON1 = #xmb movie pmf


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
