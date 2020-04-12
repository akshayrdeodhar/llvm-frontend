kaleidoscope: kaleidoscope.cpp
	clang++ kaleidoscope.cpp -o kaleidoscope

test: kaleidoscope fib.k
	cat fib.k | ./kaleidoscope

clean:
	rm kaleidoscope
