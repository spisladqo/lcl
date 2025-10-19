PROJECT=lcl

IMG_OUT_DIR=images/output
IMG_SERIAL_OUT_DIR=$(IMG_OUT_DIR)/serial

SRC_DIR=src
LIB_DIR=$(SRC_DIR)/lib
PARSE_DIR=$(SRC_DIR)/parse
TEST_DIR=tests

BENCH_DIR=$(TEST_DIR)/benchmarks
UNIT_DIR=$(TEST_DIR)/unit

CC=gcc
CFLAGS+= -O1

build: all

all:
	$(CC) $(LIB_DIR)/*/* $(PARSE_DIR)/* -o $(PROJECT) $(CFLAGS)

debug:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(PARSE_DIR)/*.c -g -fsanitize=address -o $(PROJECT) $(CFLAGS)

test:
	$(CC) $(LIB_DIR)/*/* -Wall -Wextra -g $(UNIT_DIR)/* -o $(PROJECT)_tests -lcmocka
	./$(PROJECT)_tests

fmt:
	clang-format -style="{BasedOnStyle: Google, IndentWidth: 4, SortIncludes: false}" -i $(LIB_DIR)/*/* $(PARSE_DIR)/*

fmt-check:
	clang-format -style="{BasedOnStyle: Google, IndentWidth: 4, SortIncludes: false}" --dry-run --Werror  $(LIB_DIR)/*/* $(PARSE_DIR)/*

clean-img:
	rm -rf $(IMG_OUT_DIR)/*

clean: clean-img
	rm -f $(PROJECT)*
	rm -f out*
