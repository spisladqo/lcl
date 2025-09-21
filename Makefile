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
UNIT_DIR=$(TESTS)/unit

CC=gcc

build: all

all:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(PARSE_DIR)/*.c -o lcl $(CFLAGS)

debug:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* -g -fsanitize=address -o $(PROJECT) $(CFLAGS)

bench:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(BENCH_DIR)/*.c -o $(PROJECT)_bench

test:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* -Wall -Wextra -g $(TESTS)/unit/* -o $(PROJECT)_tests -lcmocka
	./$(PROJECT)_tests

fmt:
	clang-format -style="{BasedOnStyle: Google, IndentWidth: 4, SortIncludes: false}" -i $(LIB_CONV_DIR)/* $(PARSE_DIR)/* $(UNIT_DIR)/* $(BENCH_DIR)/bench.c

fmt-check:
	clang-format -style="{BasedOnStyle: Google, IndentWidth: 4, SortIncludes: false}" --dry-run --Werror  $(LIB_CONV_DIR)/* $(PARSE_DIR)/* $(UNIT_DIR)/* $(BENCH_DIR)/bench.c

clean-img:
	rm -rf $(IMG_SERIAL_OUT_DIR)/*

clean: clean-img
	rm -f $(PROJECT)
	rm -f liblcl_tests
	rm -f liblcl_bench
	rm -f lcl
