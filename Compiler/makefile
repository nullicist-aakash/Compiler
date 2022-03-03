compile:
	mkdir bin
	gcc -c Lexer/Lexer.c -o bin/Lexer.o
	gcc -c helpers/Stack.c -o bin/Stack.o
	gcc -c helpers/Trie.c -o bin/Trie.o
	gcc -c Parser/Parser.c -o bin/Parser.o
	gcc -c Driver.c -o bin/Driver.o
	gcc -O0 -o Compiler.out bin/Lexer.o bin/Stack.o bin/Trie.o bin/Parser.o bin/Driver.o -fsanitize=address
	rm -r bin
clean:
	rm -r bin
	rm Compiler.out
