all:
	mkdir -p bin && cc src/*.c -Wall -Iinc -o bin/test

clean:
	rm -rf bin/*

test:
	@mkdir -p bin && cc src/*.c -Wall -Iinc -O0 -g -o bin/test && bin/test
