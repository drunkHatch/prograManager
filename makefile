run:
	gcc -D_POSIX_C_SOURCE -std=c99 -o test a1jobs.c
exe:
	./test
clean:
	rm test
