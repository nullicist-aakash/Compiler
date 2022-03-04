compile:
	rm -r bin
	rm Compiler.out
	mkdir bin
	gcc -c lexer.c -o bin/lexer.o
	gcc -c stack.c -o bin/stack.o
	gcc -c trie.c -o bin/trie.o
	gcc -c parser.c -o bin/parser.o
	gcc -c driver.c -o bin/driver.o
	gcc -o Compiler.out bin/lexer.o bin/stack.o bin/trie.o bin/parser.o bin/driver.o
	rm -r bin
	
clean:
	rm -r bin
	rm Compiler.out
