all: test_simple_io bbfs

test_simple_io: test_simple_io.o caching.o log.o
	gcc -g -Wall -pthread -L/home/dzhao/bin/fuse/lib -lfuse -lrt -ldl -o test_simple_io test_simple_io.o caching.o log.o

test_simple_io.o: test_simple_io.c
	gcc -g -Wall -c test_simple_io.c

bbfs: bbfs.o log.o compress.o caching.o
	gcc -g -Wall -pthread -L/home/dzhao/bin/fuse/lib -lfuse -lrt -ldl -o bbfs bbfs.o log.o compress.o caching.o

bbfs.o: bbfs.c log.h params.h
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -I/home/dzhao/bin/fuse/include -c bbfs.c

log.o : log.c log.h params.h
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -I/home/dzhao/bin/fuse/include -c log.c

compress.o: compress.c common.h
	gcc -g -Wall -c compress.c

caching.o: caching.c common.h
	gcc -g -Wall -c caching.c

clean:
	rm -rf bbfs.log test_simple_io bbfs *.o
