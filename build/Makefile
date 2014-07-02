
t-page: t-page.cc
	g++ -o $@ t-page.cc `fox-config --cflags --libs`

t-page.static: t-page.cc
	g++ -static -s -o $@ t-page.cc `fox-config --cflags` -lFOX-1.6 \
      -lXft -lfontconfig -lexpat -lfreetype \
     -lXcursor -lXrender -lXrandr -lXfixes -lXext \
     -Wl,--start-group -lX11 -lXau -Wl,--end-group -ltiff -ljpeg \
     -lpng -lbz2 -lz

foxhelpc: foxhelpc.c
	gcc -o $@ foxhelpc.c -lpng -lz

foxhelpc.static: foxhelpc.c
	gcc -static -s -o $@ foxhelpc.c -lpng -lz


CFLAGS=-I../src -I../imports
t-parse: t-parse.c buffer.o critbit.o ifile.o pngdim.o
	gcc -o $@ -I../src/ -I../imports $^ -lpng

ifile.o: ../src/ifile.c
buffer.o: ../imports/buffer.o
critbit.o: ../imports/critbit.c
pngdim.o: ../src/pngdim.c

