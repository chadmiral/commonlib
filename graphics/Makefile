CC=g++
LIB_PATH=
CFLAGS=-I../
CFLAGS+=-I./include -I../mathlib/include -I/usr/include/libxml2 -I../fluids/include
CFLAGS+=-I/usr/local/include -I../game/include -I../ui/include -I./include/vfx
CFLAGS+=-I../../glm
CFLAGS+=$(shell pkg-config freetype2 --cflags)
CFLAGS+=$(shell pkg-config glew --cflags)
CFLAGS+=$(shell pkg-config sdl2 --cflags)
CFLAGS+=$(shell pkg-config SDL2_image --cflags)
#CFLAGS+=-Ofast
CFLAGS+= -g3 -O0 -D__DEBUG__

LDFLAGS=$(shell pkg-config freetype2 --libs)
LDFLAGS+=$(shell pkg-config glew --libs)
LDFLAGS+=$(shell pkg-config sdl2 --libs)
LDFLAGS+=$(shell pkg-config SDL2_image --libs)
LDFLAGS+=-lmath -lgraphics -lfluids -lgame -lui
SRCPATH=src/
SRCS=src/font.cpp src/glutility.cpp src/material.cpp src/shader.cpp src/texture.cpp
SRCS+=src/camera.cpp src/render_surface.cpp src/hdr_render_surface.cpp
SRCS+=src/render_surface_combiner.cpp src/gpu_hair_sim.cpp src/static_mesh.cpp
SRCS+=src/gpu_particle_system.cpp src/vfx/ribbon.cpp src/gl_error.cpp
OBJS=$(SRCS:.cpp=.o)

UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	#CFLAGS+=-I/opt/X11/include/freetype2/
	CFLAGS+=-Wno-deprecated-declarations
	#CFLAGS+=-I$(shell xcrun --show-sdk-path)/System/Library/Frameworks/

	LIB_PATH+=-Lbin/osx
	LIB_PATH+=-L../mathlib/bin/osx
	LIB_PATH+=-L../fluids/bin/osx
	LIB_PATH+=-L../ui/bin/osx
	LIB_PATH+=-L../game/bin/osx

	#LDFLAGS+=-framework SDL2
	#LDFLAGS+=-framework SDL2_image
	LDFLAGS+=-framework OpenGL

	BINPATH=bin/osx/
else
	#generic link flags
	LIB_PATH += -Lbin/linux
	LIB_PATH += -L../mathlib/bin/linux
	LIB_PATH += -L../fluids/bin/linux
	LIB_PATH += -L../ui/bin/linux
	LIB_PATH += -L../game/bin/linux

	BINPATH=bin/linux/
	#CFLAGS+=-I/usr/include/freetype2/
	CFLAGS+=$(shell pkg-config --cflags sdl2)

#	LDFLAGS+=$(shell pkg-config --libs sdl2)
	#LDFLAGS+=$(shell pkg-config --libs SDL2_image)
	LDFLAGS+=-lGL
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))
#echo $(OBJS_BIN)


all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libgraphics.a test

$(BINPATH):
	mkdir -p $(BINPATH)

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH)gl_error.o: $(SRCPATH)gl_error.cpp
	$(CC) $(CFLAGS) -c src/gl_error.cpp -o $(BINPATH)gl_error.o

$(BINPATH)font.o: $(SRCPATH)font.cpp
	$(CC) $(CFLAGS) -c src/font.cpp -o $(BINPATH)font.o

$(BINPATH)glutility.o: $(SRCPATH)glutility.cpp
	$(CC) $(CFLAGS) -c src/glutility.cpp -o $(BINPATH)glutility.o

$(BINPATH)material.o: $(SRCPATH)material.cpp
	$(CC) $(CFLAGS) -c src/material.cpp -o $(BINPATH)material.o

$(BINPATH)shader.o: $(SRCPATH)shader.cpp
	$(CC) $(CFLAGS) -c src/shader.cpp -o $(BINPATH)shader.o

#$(BINPATH)static_mesh.o: $(SRCPATH)static_mesh.cpp
#	$(CC) $(CFLAGS) -c src/static_mesh.cpp -o $(BINPATH)static_mesh.o

$(BINPATH)texture.o: $(SRCPATH)texture.cpp
	$(CC) $(CFLAGS) -c src/texture.cpp -o $(BINPATH)texture.o

$(BINPATH)camera.o: $(SRCPATH)camera.cpp
	$(CC) $(CFLAGS) -c src/camera.cpp -o $(BINPATH)camera.o

$(BINPATH)render_surface.o: $(SRCPATH)render_surface.cpp
	$(CC) $(CFLAGS) -c src/render_surface.cpp -o $(BINPATH)render_surface.o

$(BINPATH)hdr_render_surface.o: $(SRCPATH)hdr_render_surface.cpp
	$(CC) $(CFLAGS) -c src/hdr_render_surface.cpp -o $(BINPATH)hdr_render_surface.o

$(BINPATH)render_surface_combiner.o: $(SRCPATH)render_surface_combiner.cpp
	$(CC) $(CFLAGS) -c src/render_surface_combiner.cpp -o $(BINPATH)render_surface_combiner.o

$(BINPATH)gpu_hair_sim.o: $(SRCPATH)gpu_hair_sim.cpp
	$(CC) $(CFLAGS) -c src/gpu_hair_sim.cpp -o $(BINPATH)gpu_hair_sim.o

$(BINPATH)gpu_particle_system.o: $(SRCPATH)gpu_particle_system.cpp
	$(CC) $(CFLAGS) -c src/gpu_particle_system.cpp -o $(BINPATH)gpu_particle_system.o

$(BINPATH)static_mesh.o: $(SRCPATH)static_mesh.cpp
	$(CC) $(CFLAGS) -c src/static_mesh.cpp -o $(BINPATH)static_mesh.o

$(BINPATH)ribbon.o: $(SRCPATH)/vfx/ribbon.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)vfx/ribbon.cpp -o $(BINPATH)ribbon.o

$(BINPATH)libgraphics.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libgraphics.a $(OBJS_BIN)

test: test.cpp $(BINPATH)libgraphics.a $(SRCS)
	$(CC) $(CFLAGS) test.cpp -o test $(LIB_PATH) $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libgraphics.a
	rm -f test
