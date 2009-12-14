TARGET_LIB = template_lib.a

OBJS += main.o

INCDIR = 
CFLAGS = -G0 -Wall -O3
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

doc :
	doxygen