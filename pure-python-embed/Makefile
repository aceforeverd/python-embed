PY_LDFLAGS := $(shell (python3-config --ldflags --embed))
PY_CFLAGS := $(shell (python3-config --cflags))

CC ?= gcc

.PHONY: c-call-python-script clean all

all: c-call-python-script

c-call-python-script: c-call-python-script.c
	$(CC) $^ $(PY_CFLAGS) $(PY_LDFLAGS) -o c-call-python

simple-call: simple-call.c
	$(CC) $^ $(PY_CFLAGS) $(PY_LDFLAGS) -o simple-call

clean:
	rm -vf *.o c-call-python simple-call
