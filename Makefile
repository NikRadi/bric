CFLAGS=-Ilib/include -g
LIBS=lib/libtree-sitter.a
NAME=bric


all:
	gcc $(CFLAGS) -c src/tree-sitter-java/src/parser.c
	g++ $(CFLAGS) src/*.cpp parser.o $(LIBS) -o $(NAME)


.PHONE: clean
clean:
	rm -f $(NAME)
	rm -f parser.o
	rm -f Test_reduced.c
	rm -f a.out
