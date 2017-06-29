CC=g++
LIB_PATH=
CFLAGS=-I./include
CFLAGS+=-Ofast
#CFLAGS += -g3 -O0
CFLAGS+=-std=c++11

LDFLAGS=
SRCPATH=src/
SRCS=src/portfolio.cpp src/account.cpp src/currency.cpp
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

all: $(BINPATH) $(OBJS_BIN) $(BINPATH)libeconomy.a test

#$(OBJS): $(SRCS)
#	$(CC) $(CFLAGS) -c $< -o $@
#	mv $@ $(BINPATH)

$(BINPATH):
	mkdir -p $(BINPATH)

$(BINPATH)account.o: $(SRCPATH)account.cpp
	$(CC) $(CFLAGS) -c src/account.cpp -o $(BINPATH)account.o

$(BINPATH)currency.o: $(SRCPATH)currency.cpp
	$(CC) $(CFLAGS) -c src/currency.cpp -o $(BINPATH)currency.o

$(BINPATH)portfolio.o: $(SRCPATH)portfolio.cpp
	$(CC) $(CFLAGS) -c src/portfolio.cpp -o $(BINPATH)portfolio.o

$(BINPATH)libeconomy.a: $(OBJS_BIN)
	ar -cvq $(BINPATH)libeconomy.a $(OBJS_BIN)

test: test.cpp $(BINPATH)libeconomy.a
	$(CC) $(CFLAGS) test.cpp  -o test $(LIB_PATH) $(LDFLAGS) -leconomy

clean:
	rm -f $(BINPATH)*.o
	rm -f $(BINPATH)libeconomy.a
	rm -f test
