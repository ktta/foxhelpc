# Makefile for the foxhelpc program. You can control this build by three
# variables on the command line: SELF, STATIC and DBG.
#
# If SELF=1 is given on the command line, the resulting program will
# recognize the -H switch and will contain the help document inside.
# 
# If STATIC=1 is given, then the resulting binary will be static.
#
# If DBG=1 is given a debug build is made. Otherwise -O2 is used for
# optimizing the compilation 
# 

ifeq ($(STATIC),1)
LFLAGS=-static -s 
FOXLIBS=-lFOX-1.6 \
  -lXft -lfontconfig -lexpat -lfreetype \
  -lXcursor -lXrender -lXrandr -lXfixes -lXext \
  -Wl,--start-group -lX11 -lXau -Wl,--end-group -ltiff -ljpeg \
  -lpng -lbz2 -lz
NOFOXLIBS=-lpng -lz
else
LFLAGS=
FOXLIBS=`fox-config --libs`
NOFOXLIBS=-lpng
endif

ifeq ($(SELF),1)
LIBS=$(FOXLIBS)
S_FLAG=-DSELFHELP
OBJS=foxhelpc.o selfhelp.o
LINKER=g++
else
LIBS=$(NOFOXLIBS)
S_FLAG=
OBJS=foxhelpc.o
LINKER=gcc
endif

ifeq ($(DBG),1)
DBGLINK=-g
DBGCOM=-g
else
DBGLINK=
DBGCOM=-O2
endif

foxhelpc: $(OBJS)
	$(LINKER) $(DBGLINK) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

foxhelpc.o: foxhelpc.c
	gcc $(S_FLAG) $(DBGCOM) -c foxhelpc.c

ifeq ($(SELF),1)
selfhelp.o: selfhelp.cc
	g++ $(DBGCOM) -c `fox-config --cflags` selfhelp.cc
endif

