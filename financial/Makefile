CC=g++
LIB_PATH=
CFLAGS=-I./include
CFLAGS+=-Ofast
#CFLAGS += -g3 -O0
CFLAGS+=-std=c++11

LDFLAGS=
SRCPATH=src/
SRCS=src/portfolio.cpp
OBJS=$(SRCS:.cpp=.o)

UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	#osx
	CFLAGS+=-Wno-deprecated-declarations
	LIB_PATH+=-Lbin/osx

	BINPATH=bin/osx/
else
	#generic link flags
	LIB_PATH+=-Lbin/linux

	BINPATH=bin/linux/
endif

OBJS_BIN=$(addprefix $(BINPATH), $(notdir $(OBJS)))
#echo $(OBJS_BIN)

all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libfinance.a test

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH):
	mkdir -p $(BINPATH)

$(BINPATH)portfolio.o: $(SRCPATH)portfolio.cpp
	$(CC) $(CFLAGS) -c src/portfolio.cpp -o $(BINPATH)portfolio.o

$(BINPATH)libfinance.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libfinance.a $(OBJS_BIN)

test: test.cpp $(BINPATH)libfinance.a
	$(CC) $(CFLAGS) test.cpp -o test $(LIB_PATH) $(LDFLAGS)

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libfinance.a
	rm -f test
