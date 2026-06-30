# Host-side unit tests for firmware modules
# Usage:  make          – build and run all tests
#         make test     – same
#         make clean    – remove build artefacts

CC      ?= gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O0 \
           -I. -I../App -I../App/app -I../App/driver

TESTS   := test_crc test_dcs test_misc test_frequencies test_dtmf
BINS    := $(addprefix build/,$(TESTS))

.PHONY: all test clean

all: test

test: $(BINS)
	@echo ""
	@echo "=============================="
	@echo " Running all unit tests"
	@echo "=============================="
	@FAIL=0; \
	for t in $(BINS); do \
		echo ""; \
		./$$t || FAIL=1; \
	done; \
	echo ""; \
	if [ $$FAIL -eq 0 ]; then \
		echo "All test suites passed."; \
	else \
		echo "Some tests FAILED."; exit 1; \
	fi

build:
	mkdir -p build

build/test_crc: test_crc.c test_framework.h ../App/driver/crc.c | build
	$(CC) $(CFLAGS) -o $@ $<

build/test_dcs: test_dcs.c test_framework.h ../App/dcs.c | build
	$(CC) $(CFLAGS) -o $@ $<

build/test_misc: test_misc.c test_framework.h | build
	$(CC) $(CFLAGS) -o $@ $<

build/test_frequencies: test_frequencies.c test_framework.h ../App/frequencies.c | build
	$(CC) $(CFLAGS) -DENABLE_WIDE_RX -o $@ $<

build/test_dtmf: test_dtmf.c test_framework.h | build
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf build
