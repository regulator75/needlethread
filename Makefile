test.out: src/test.c src/needlethread.c src/pthread.h
	gcc src/test.c src/needlethread.c -o $@