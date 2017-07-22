CC=g++

CFLAGS=-I./include -I../mathlib/include -I../graphics/include
CFLAGS+=-I/usr/local/include -I../game/include -I../game/include/animation
CFLAGS+=-I../structures/include
CFLAGS+=-I./include/minilzo -I../
CFLAGS+=$(shell pkg-config mxml --cflags)
CFLAGS+=$(shell pkg-config sdl2 --cflags)
CFLAGS+=$(shell pkg-config --cflags glew)
#CFLAGS+=-Ofast
CFLAGS+= -g3 -O0

LIB_PATH=
LDFLAGS=$(shell pkg-config mxml --libs)
LDFLAGS+=$(shell pkg-config --libs sdl2)
LDFLAGS+=$(shell pkg-config SDL2_image --libs)
LDFLAGS+=$(shell pkg-config --libs glew)
LDFLAGS+=-lgraphics -ltool -lmath
SRCPATH=src/
SRCS=src/static_mesh_baker.cpp src/bakery.cpp src/tool.cpp
SRCS+=src/bphys_baker.cpp src/shader_baker.cpp
SRCS+=src/minilzo/minilzo.cpp src/package_baker.cpp
SRCS+=src/skeleton_baker.cpp src/animation_baker.cpp
SRCS+=src/material_baker.cpp src/lens_flare_baker.cpp
OBJS=$(SRCS:.cpp=.o)

UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	CFLAGS+=-Wno-deprecated-declarations
	LIB_PATH+=-Lbin/osx
	LIB_PATH+=-L../graphics/bin/osx
	LIB_PATH+=-L../mathlib/bin/osx
	BINPATH=bin/osx/
else
	#generic link flags
	LIB_PATH += -Lbin/linux
	BINPATH=bin/linux/
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))
#echo $(OBJS_BIN)


all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libtool.a bake

$(BINPATH):
	mkdir -p $(BINPATH)

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH)tool.o: $(SRCPATH)tool.cpp
	$(CC) $(CFLAGS) -c src/tool.cpp -o $(BINPATH)tool.o

$(BINPATH)bakery.o: $(SRCPATH)bakery.cpp
	$(CC) $(CFLAGS) -c src/bakery.cpp -o $(BINPATH)bakery.o

$(BINPATH)package_baker.o: $(SRCPATH)package_baker.cpp
	$(CC) $(CFLAGS) -c src/package_baker.cpp -o $(BINPATH)package_baker.o

$(BINPATH)static_mesh_baker.o: $(SRCPATH)static_mesh_baker.cpp
	$(CC) $(CFLAGS) -c src/static_mesh_baker.cpp -o $(BINPATH)static_mesh_baker.o

$(BINPATH)bphys_baker.o: $(SRCPATH)bphys_baker.cpp
	$(CC) $(CFLAGS) -c src/bphys_baker.cpp -o $(BINPATH)bphys_baker.o

$(BINPATH)shader_baker.o: $(SRCPATH)shader_baker.cpp
	$(CC) $(CFLAGS) -c src/shader_baker.cpp -o $(BINPATH)shader_baker.o

$(BINPATH)skeleton_baker.o: $(SRCPATH)skeleton_baker.cpp
	$(CC) $(CFLAGS) -c src/skeleton_baker.cpp -o $(BINPATH)skeleton_baker.o

$(BINPATH)animation_baker.o: $(SRCPATH)animation_baker.cpp
	$(CC) $(CFLAGS) -c src/animation_baker.cpp -o $(BINPATH)animation_baker.o

$(BINPATH)material_baker.o: $(SRCPATH)material_baker.cpp
	$(CC) $(CFLAGS) -c src/material_baker.cpp -o $(BINPATH)material_baker.o

$(BINPATH)lens_flare_baker.o: $(SRCPATH)lens_flare_baker.cpp
	$(CC) $(CFLAGS) -c src/lens_flare_baker.cpp -o $(BINPATH)lens_flare_baker.o

$(BINPATH)minilzo.o: $(SRCPATH)minilzo/minilzo.cpp
	$(CC) $(CFLAGS) -c src/minilzo/minilzo.cpp -o $(BINPATH)minilzo.o

$(BINPATH)libtool.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libtool.a $(OBJS_BIN)

bake: bake.cpp $(BINPATH)libtool.a $(SRCS)
	$(CC) $(CFLAGS) bake.cpp -o bake $(LIB_PATH) $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libtool.a
	rm -f bake
