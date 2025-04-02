CC=g++
CXX=g++
RANLIB=ranlib
AR=ar
RM=rm

LIBSRC=Threads.h Threads.cpp uthreads.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

MAINSRC=main.cpp
MAINOBJ=$(MAINSRC:.cpp=.o)

INCS=-I.
CFLAGS = -Wall -std=c++11 -g $(INCS)
CXXFLAGS = -Wall -std=c++11 -g $(INCS)

OSMLIB = libuthreads.a
TARGETS = $(OSMLIB) main

TAR=tar
TARFLAGS=-cvf
TARNAME=ex1.tar
TARSRCS=$(LIBSRC) main.cpp Makefile README

all: $(TARGETS)

$(OSMLIB): $(LIBOBJ)
		$(AR) rcs $@ $^
		$(RANLIB) $@

main: $(MAINOBJ) $(OSMLIB)
		$(CXX) $(CXXFLAGS) -o $@ $^ -pthread

%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
		$(RM) $(TARGETS) $(OSMLIB) $(MAINOBJ) $(LIBOBJ) *~ *core

depend:
		makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC) $(MAINSRC)

tar:
		$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)