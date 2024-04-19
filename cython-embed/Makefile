.PHONY: embeded embedded_main embedded.c libembedded embedded.o

PY_LDFLAGS := $(shell (python3-config --ldflags --embed))
PY_CFLAGS := $(shell (python3-config --cflags))

CC ?= gcc

all: embedded_main

embedded.c:
	cython embedded.pyx

embedded.o: embedded.c
	$(CC) -c $^ $(PY_CFLAGS)

libembedded: embedded.c
	$(CC) $^ $(PY_CFLAGS) $(PY_LDFLAGS) -shared -fPIC -o $@.so 

embedded_main: libembedded
	$(CC) $@.c $^.so $(PY_CFLAGS) $(PY_LDFLAGS) -o $@


clean:
	rm -vf embedded.h embedded.c embedded.o embedded_main libembedded.so
