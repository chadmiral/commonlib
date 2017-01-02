CC=g++
LIB_PATH=
CFLAGS=-I./include -I../game/include  -I../game/include/cinematics -I/usr/local/include
CFLAGS+=-I../graphics/include -I../ui/include -I../game/include/switchboard -I../
CFLAGS+=-I../game/include/animation/
CFLAGS+=$(shell pkg-config --cflags freetype2)
CFLAGS+=-D__USE_BOOST__
CFLAGS+=-D__USE_CGAL3D__
CFLAGS+=-Ofast
#CFLAGS+= -g3 -O0
LDFLAGS=$(shell pkg-config --libs sdl2)
LDFLAGS+=$(shell pkg-config --libs SDL2_image)
LDFLAGS+=$(shell pkg-config --libs freetype2)
LDFLAGS+=$(shell pkg-config --libs glew)
LDFLAGS+=-lCGAL -lCGAL_Core -lgmp
SRCPATH=src/
SRCS=src/math_utility.cpp src/perlin.cpp src/matrix.cpp src/aabb.cpp
SRCS+=src/triangulation.cpp src/triangulation_3d.cpp src/triangulation_sphere.cpp
SRCS+=src/quaternion.cpp src/voronoi.cpp src/shapes.cpp
SRCS+=src/function.cpp src/curve.cpp src/voronoi_sphere.cpp
SRCS+=src/gpu_voronoi.cpp src/vector.cpp
MXML_SRCS=../tools/src/mxml-2.9/mxml-attr.c ../tools/src/mxml-2.9/mxml-entity.c ../tools/src/mxml-2.9/mxml-file.c
MXML_SRCS+=../tools/src/mxml-2.9/mxml-get.c ../tools/src/mxml-2.9/mxml-index.c ../tools/src/mxml-2.9/mxml-node.c
MXML_SRCS+=../tools/src/mxml-2.9/mxml-private.c ../tools/src/mxml-2.9/mxml-search.c ../tools/src/mxml-2.9/mxml-set.c
MXML_SRCS+=../tools/src/mxml-2.9/mxml-string.c
OBJS=$(SRCS:.cpp=.o)
MXML_OBJS=$(MXML_SRCS:.cpp=.o)


UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	LIB_PATH+=-Lbin/osx
	LIB_PATH+=-L../game/bin/osx
	LIB_PATH+=-L../graphics/bin/osx
	LIB_PATH+=-L../ui/bin/osx
	LIB_PATH+=-L/usr/local/Cellar/cgal/4.6.1/lib/
	BINPATH=bin/osx/

	LDFLAGS+=-framework OpenGL
	#LDFLAGS+=-lCGAL -lCGAL_Core
else
	#generic link flags
	LIB_PATH+=-Lbin/linux
	LIB_PATH+=-L../game/bin/linux
	LIB_PATH+=-L../graphics/bin/linux
	BINPATH=bin/linux/

	#LDFLAGS+=
	LDFLAGS+=-lGL
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))

all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libmath.a test

$(BINPATH):
	mkdir -p $(BINPATH)

#$(BINPATH)mxml.o: $(MXML_SRCS)
#	gcc $(CFLAGS) -c $(MXML_SRCS) -o $(BINPATH)$(MXML_OBJS)

$(BINPATH)math_utility.o: $(SRCPATH)math_utility.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)math_utility.cpp -o $(BINPATH)math_utility.o

$(BINPATH)perlin.o: $(SRCPATH)perlin.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)perlin.cpp -o $(BINPATH)perlin.o

$(BINPATH)matrix.o: $(SRCPATH)matrix.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)matrix.cpp -o $(BINPATH)matrix.o

$(BINPATH)aabb.o: $(SRCPATH)aabb.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)aabb.cpp -o $(BINPATH)aabb.o

$(BINPATH)quaternion.o: $(SRCPATH)quaternion.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)quaternion.cpp -o $(BINPATH)quaternion.o

$(BINPATH)voronoi.o: $(SRCPATH)voronoi.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)voronoi.cpp -o $(BINPATH)voronoi.o

$(BINPATH)voronoi_sphere.o: $(SRCPATH)voronoi_sphere.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)voronoi_sphere.cpp -o $(BINPATH)voronoi_sphere.o

$(BINPATH)vector.o: $(SRCPATH)vector.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)vector.cpp -o $(BINPATH)vector.o

$(BINPATH)gpu_voronoi.o: $(SRCPATH)gpu_voronoi.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)gpu_voronoi.cpp -o $(BINPATH)gpu_voronoi.o

$(BINPATH)triangulation.o: $(SRCPATH)triangulation.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)triangulation.cpp -o $(BINPATH)triangulation.o

$(BINPATH)triangulation_3d.o: $(SRCPATH)triangulation_3d.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)triangulation_3d.cpp -o $(BINPATH)triangulation_3d.o

$(BINPATH)triangulation_sphere.o: $(SRCPATH)triangulation_sphere.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)triangulation_sphere.cpp -o $(BINPATH)triangulation_sphere.o

$(BINPATH)shapes.o: $(SRCPATH)shapes.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)shapes.cpp -o $(BINPATH)shapes.o

$(BINPATH)function.o: $(SRCPATH)function.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)function.cpp -o $(BINPATH)function.o

$(BINPATH)curve.o: $(SRCPATH)curve.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)curve.cpp -o $(BINPATH)curve.o

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH)libmath.a: $(OBJS_BIN) $(BINPATH)
	ar -cvq $(BINPATH)libmath.a $(OBJS_BIN)

matrix_test: matrix_test.cpp $(BINPATH)libmath.a
	$(CC) $(CFLAGS) matrix_test.cpp -o matrix_test $(LIB_PATH) -lmath $(LDFLAGS)

test: test.cpp $(BINPATH)libmath.a #$(BINPATH)mxml.o
	$(CC) $(CFLAGS) test.cpp -o test $(LIB_PATH) -lmath -lgame -lgraphics -lui $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libmath.a
	rm -f test
	rm -f matrix_test
