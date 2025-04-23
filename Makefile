PROJECT=main

all:
	$(CC) lib/conv/serial/serial.c lib/libbmp/libbmp.c -o $(PROJECT)

clean:
	rm -f $(PROJECT)