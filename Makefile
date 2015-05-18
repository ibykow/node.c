all:
	mkdir -p bin && cc src/*.c -Wall -Iinc -o bin/test

clean:
	rm bin/*

test:
	make && bin/test
