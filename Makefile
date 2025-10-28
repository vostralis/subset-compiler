all: src/sbstcmp.cpp src/lexer.cpp src/token.cpp
	g++ -O2 -o sbstcmp -std=c++20 -g -Iinclude \
	src/*.cpp \
	-Wall -Wextra -Wreturn-type -pedantic

clean:
	rm ./sbstcmp
