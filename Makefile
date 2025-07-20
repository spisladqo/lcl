PROJECT=liblcl
IMG_OUT_DIR=images/output
IMG_SERIAL_OUT_DIR=$(IMG_OUT_DIR)/serial
LIB=lib
TEST=test
LIB_CONV_DIR=$(LIB)/conv
LIB_LIBBMP_DIR=$(LIB)/libbmp
TEST_CONV_DIR=$(TEST)/conv

CC=gcc

# CFLAGS=-O2
CFLAGS+=-lpthread

all:
	$(CC) $(LIB_CONV_DIR)/* $(LIB_LIBBMP_DIR)/* $(TEST_CONV_DIR)/test.c -o $(PROJECT) $(CFLAGS)

clean:
	rm -f $(PROJECT)
	rm -rf $(IMG_SERIAL_OUT_DIR)/*
