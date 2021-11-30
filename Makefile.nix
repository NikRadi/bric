CFLAGS=-Ilib/ -Ofast
LIBS=lib/libtree-sitter.a
NAME=bric


all:
	gcc $(CFLAGS) -c src/tree-sitter-c/parser.c
	g++ $(CFLAGS) -Wall src/*.cpp parser.o $(LIBS) -o $(NAME)
	g++ $(CFLAGS) ast_counter/AstCounter.cpp parser.o $(LIBS) -o astcounter