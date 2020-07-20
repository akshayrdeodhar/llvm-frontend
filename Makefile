kaleidoscope: kaleidoscope.cpp
	clang++ -g -O3 -Wall kaleidoscope.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o kaleidoscope

test: kaleidoscope fib.k
	cat fib.k | ./kaleidoscope

clean:
	rm kaleidoscope
