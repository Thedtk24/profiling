CC=gcc
CFLAGS=-O3 -g -fopenmp

TARGET=mandel

all: $(TARGET)

libppm.so: ppm.c
	$(CC) -o $@ $< $(CFLAGS) -fpic -shared

mandel: mandel.c libppm.so
	$(CC) -o $@ $< $(CFLAGS) -L. -lppm -lm

clean:
	rm -f $(TARGET) *.so
