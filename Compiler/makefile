compile:
	mkdir bin
	gcc -c Lexer.c -o bin/Lexer.o
	gcc -c Stack.c -o bin/Stack.o
	gcc -c Trie.c -o bin/Trie.o
	gcc -c Parser.c -o bin/Parser.o
	gcc -c Driver.c -o bin/Driver.o
	gcc -O0 -o Compiler.out bin/Lexer.o bin/Stack.o bin/Trie.o bin/Parser.o bin/Driver.o -fsanitize=address
	rm -r bin
	./Compiler.out
clean:
	rm -r bin
	rm Compiler.out
