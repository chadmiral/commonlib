CC=g++
LIB_PATH=
CFLAGS=-I./include -I../mathlib/include
CFLAGS+=-I/usr/local/include -I../game/include
#CFLAGS+=$(shell pkg-config freetype2 --cflags)
#CFLAGS+=$(shell pkg-config glew --cflags)
#CFLAGS+=$(shell pkg-config sdl2 --cflags)
#CFLAGS+=$(shell pkg-config SDL2_image --cflags)
#CFLAGS+=-Ofast
CFLAGS+= -g3 -O0

#LDFLAGS=$(shell pkg-config freetype2 --libs)
#LDFLAGS+=$(shell pkg-config glew --libs)
#LDFLAGS+=$(shell pkg-config sdl2 --libs)
#LDFLAGS+=$(shell pkg-config SDL2_image --libs)
#LDFLAGS+=-lmath -lgame -lui
LDFLAGS=-lgpucompute
SRCPATH=src/
SRCS=src/gpu_compute.cpp
OBJS=$(SRCS:.cpp=.o)

UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	#CFLAGS+=-I/opt/X11/include/freetype2/
	CFLAGS+=-Wno-deprecated-declarations
	#CFLAGS+=-I$(shell xcrun --show-sdk-path)/System/Library/Frameworks/

	LIB_PATH+=-Lbin/osx
	#LIB_PATH+=-L../mathlib/bin/osx
	#LIB_PATH+=-L../ui/bin/osx
	#LIB_PATH+=-L../game/bin/osx

	#LDFLAGS+=-framework SDL2
	#LDFLAGS+=-framework SDL2_image
	LDFLAGS+=-framework OpenCL

	BINPATH=bin/osx/
else
	#generic link flags
	LIB_PATH += -Lbin/linux
	#LIB_PATH += -L../mathlib/bin/linux
	#LIB_PATH += -L../ui/bin/linux
	#LIB_PATH += -L../game/bin/linux

	BINPATH=bin/linux/
	#CFLAGS+=-I/usr/include/freetype2/
	#CFLAGS+=$(shell pkg-config --cflags sdl2)

#	LDFLAGS+=$(shell pkg-config --libs sdl2)
	#LDFLAGS+=$(shell pkg-config --libs SDL2_image)
	LDFLAGS+=-lCL
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))
#echo $(OBJS_BIN)


all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libgpucompute.a test

$(BINPATH):
	mkdir -p $(BINPATH)

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH)gpu_compute.o: $(SRCPATH)gpu_compute.cpp
	$(CC) $(CFLAGS) -c src/gpu_compute.cpp -o $(BINPATH)gpu_compute.o

$(BINPATH)libgpucompute.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libgpucompute.a $(OBJS_BIN)

test: test.cpp $(BINPATH)libgpucompute.a $(SRCS)
	$(CC) $(CFLAGS) test.cpp -o test $(LIB_PATH) $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libgpucompute.a
	rm -f test
