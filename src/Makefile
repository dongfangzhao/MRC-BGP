fusionfs : fusionfs.o util.o log.o
	gcc -g -pthread -L/home/dzhao/bin/fuse/lib -lfuse -lrt -ldl -o fusionfs fusionfs.o util.o log.o -L./ffsnet -lffsnet_bridger -L./zht/lib -lzht -lstdc++ -L/home/dzhao/bin/gbuf/lib -lprotobuf -L/home/dzhao/bin/gbuf-c/lib -lprotobuf-c -lpthread 

fusionfs.o : fusionfs.c log.h params.h util.h
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -I/home/dzhao/bin/fuse/include/fuse -I/home/dzhao/bin/gbuf/include -I/home/dzhao/bin/gbuf-c/include -c fusionfs.c 
	
util.o : util.c log.h params.h
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -I/home/dzhao/bin/fuse/include/fuse -I/home/dzhao/bin/gbuf/include -I/home/dzhao/bin/gbuf-c/include -c util.c 

log.o : log.c log.h params.h
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -I/home/dzhao/bin/fuse/include/fuse -c log.c

clean:
	rm -f fusionfs *.o 

dist:
	rm -rf fuse-tutorial/
	mkdir fuse-tutorial/
	cp ../*.html fuse-tutorial/
	mkdir fuse-tutorial/example/
	mkdir fuse-tutorial/example/mountdir/
	mkdir fuse-tutorial/example/rootdir/
	echo "a bogus file" > fuse-tutorial/example/rootdir/bogus.txt
	mkdir fuse-tutorial/src
	cp Makefile *.c *.h fuse-tutorial/src/
	tar cvzf ../../fuse-tutorial.tgz fuse-tutorial/
	rm -rf fuse-tutorial/
