TARGET = template_prx
OBJS = main.o 

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = export.exp

LIBDIR =
#PSP_FW_VERSION=390
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpspdebug


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak 

all:
	psp-build-exports -s $(PRX_EXPORTS) 
