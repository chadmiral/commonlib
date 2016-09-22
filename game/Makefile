CC=g++
LIB_PATH=
CFLAGS=-I./include -I./include/cinematics -I/usr/local/include/
CFLAGS+=-I./include/switchboard
CFLAGS+=-I../graphics/include/ -I../ui/include
CFLAGS+=-I../mathlib/include/
CFLAGS+=$(shell pkg-config --cflags sdl2)
CFLAGS+=$(shell pkg-config --cflags freetype2)
CFLAGS+=-Ofast
#CFLAGS += -g3 -O0
CFLAGS+=-std=c++11

LDFLAGS=$(shell pkg-config --libs sdl2)
LDFLAGS+=$(shell pkg-config SDL2_image --libs)
LDFLAGS+=$(shell pkg-config --libs freetype2)
LDFLAGS+=$(shell pkg-config --libs glew)
LDFLAGS+=-lmath -lui
SRCPATH=src/
SRCS=src/sdl_game.cpp src/console.cpp src/cinematics/fade_screen.cpp
SRCS+=src/object_3d.cpp src/game_controller.cpp
OBJS=$(SRCS:.cpp=.o)

UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	CFLAGS+=-Wno-deprecated-declarations
	LIB_PATH+=-Lbin/osx
	LIB_PATH+=-L../graphics/bin/osx
	LIB_PATH+=-L../mathlib/bin/osx
	LIB_PATH+=-L../ui/bin/osx
	LDFLAGS+=-framework OpenGL

	BINPATH=bin/osx/
else
	#generic link flags
	LIB_PATH+=-Lbin/linux
	LIB_PATH+=-L../graphics/bin/linux
	LIB_PATH+=-L../mathlib/bin/linux

	LDFLAGS+=$(shell pkg-config --libs gl)

	BINPATH=bin/linux/
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))
#echo $(OBJS_BIN)

all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libgame.a test

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH):
	mkdir -p $(BINPATH)

$(BINPATH)sdl_game.o: $(SRCPATH)sdl_game.cpp
	$(CC) $(CFLAGS) -c src/sdl_game.cpp -o $(BINPATH)sdl_game.o

$(BINPATH)object_3d.o: $(SRCPATH)object_3d.cpp
	$(CC) $(CFLAGS) -c src/object_3d.cpp -o $(BINPATH)object_3d.o

$(BINPATH)console.o: $(SRCPATH)console.cpp
	$(CC) $(CFLAGS) -c src/console.cpp -o $(BINPATH)console.o

$(BINPATH)fade_screen.o: $(SRCPATH)cinematics/fade_screen.cpp
	$(CC) $(CFLAGS) -c src/cinematics/fade_screen.cpp -o $(BINPATH)fade_screen.o

$(BINPATH)game_controller.o: $(SRCPATH)game_controller.cpp
	$(CC) $(CFLAGS) -c src/game_controller.cpp -o $(BINPATH)game_controller.o

$(BINPATH)libgame.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libgame.a $(OBJS_BIN)

test: test.cpp $(BINPATH)libgame.a
	$(CC) $(CFLAGS) test.cpp -o test $(LIB_PATH) -lgame -lgraphics $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libgame.a
	rm -f test
