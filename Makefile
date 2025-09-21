PROJECT=liblcl

IMG_OUT_DIR=images/output
IMG_SERIAL_OUT_DIR=$(IMG_OUT_DIR)/serial

LIB=lib
TESTS=tests

LIB_CONV_DIR=$(LIB)/conv
LIB_LIBBMP_DIR=$(LIB)/libbmp
TEST_CONV_DIR=$(TESTS)/conv
PARSE_DIR = $(LIB)/parse

BENCH_DIR=$(TESTS)/benchmarks

CC=gcc

all:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(PARSE_DIR)/*.c -o lcl $(CFLAGS)

lib:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* -o $(PROJECT) $(CFLAGS)

debug:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* -g -fsanitize=address -o $(PROJECT) $(CFLAGS)

bench:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(BENCH_DIR)/*.c -o $(PROJECT)_bench

test:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* -Wall -Wextra -g $(TESTS)/unit/* -o $(PROJECT)_tests -lcmocka

clean-img:
	rm -rf $(IMG_SERIAL_OUT_DIR)/*

clean-bench:
	rm liblcl_bench

clean-tests:
	rm liblcl_tests

clean: clean-img clean-bench clean-tests
	rm -f $(PROJECT)