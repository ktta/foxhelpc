
CFLAGS=-I../src -I../imports
foxhelpc: t-parse.c buffer.o critbit.o ifile.o pngdim.o
	gcc -o $@ -I../src/ -I../imports $^ -lpng

ifile.o: ../src/ifile.c
buffer.o: ../imports/buffer.o
critbit.o: ../imports/critbit.c
pngdim.o: ../src/pngdim.c


