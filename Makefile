kaleidoscope: kaleidoscope.cpp
	clang++ -Wall -fsanitize=address kaleidoscope.cpp -o kaleidoscope 

test: kaleidoscope fib.k
	cat fib.k | ./kaleidoscope

clean:
	rm kaleidoscope
