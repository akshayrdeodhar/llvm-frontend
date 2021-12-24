kaleidoscope: kaleidoscope.cpp
	clang++ -g3 -Wall kaleidoscope.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o kaleidoscope

theirkaleidoscope: theirkaleidoscope.cpp
	clang++ -g3 -Wall theirkaleidoscope.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o theirkaleidoscope

test: kaleidoscope fib.k
	cat fib.k | ./kaleidoscope

clean:
	rm kaleidoscope
