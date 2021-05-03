test.out: src/test.c src/needlethread.c src/pthread.h src/call_on_new_stack.s src/switch.s
	gcc -g src/test.c src/needlethread.c src/call_on_new_stack.s src/switch.s -o $@