compile:
	mkdir bin
	gcc -c lexer.c -o bin/lexer.o
	gcc -c stack.c -o bin/stack.o
	gcc -c trie.c -o bin/trie.o
	gcc -c parser.c -o bin/parser.o
	gcc -c ast.c -o bin/ast.o
	gcc -c driver.c -o bin/driver.o
	gcc -o stage1exe bin/lexer.o bin/stack.o bin/trie.o bin/parser.o bin/ast.o bin/driver.o
	rm -r bin
	
clean:
	rm -r bin
	rm stage1exe
