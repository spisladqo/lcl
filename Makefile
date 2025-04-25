PROJECT=main
IMG_OUT_DIR=images/output/
IMG_SERIAL_OUT_DIR=$(IMG_OUT_DIR)/serial/

all:
	$(CC) lib/conv/serial.c lib/conv/common.c lib/libbmp/libbmp.c test/conv/test_serial.c -o $(PROJECT)

clean:
	rm -f $(PROJECT)
	rm -rf $(IMG_SERIAL_OUT_DIR)/*
