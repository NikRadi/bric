CFLAGS=-Ilib/include -g
LIBS=lib/libtree-sitter.a
NAME=bric


all:
	cp Test.save.c Test.c
	gcc $(CFLAGS) -c src/tree-sitter-c/src/parser.c
	g++ $(CFLAGS) -Wall src/*.cpp parser.o $(LIBS) -o $(NAME)


.PHONE: clean
clean:
	rm -f $(NAME)
	rm -f parser.o
	rm -f Test_reduced.c
	rm -f a.out
