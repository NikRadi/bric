CFLAGS=-Ilib/include -g
LIBS=lib/libtree-sitter.a
NAME=bric


all:
	gcc $(CFLAGS) -c src/tree-sitter-c/src/parser.c
	g++ $(CFLAGS) -Wall src/*.cpp parser.o $(LIBS) -o $(NAME)