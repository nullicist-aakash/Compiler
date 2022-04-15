compile:
	mkdir bin || true
	gcc -c lexer.c -o bin/lexer.o -g
	gcc -c stack.c -o bin/stack.o -g
	gcc -c trie.c -o bin/trie.o -g
	gcc -c parser.c -o bin/parser.o -g
	gcc -c ast.c -o bin/ast.o -g
	gcc -c symbolTable.c -o bin/symbolTable.o -g
	gcc -c typeChecker.c -o bin/typeChecker.o -g
	gcc -c driver.c -o bin/driver.o -g
	gcc -c toposort.c -o bin/toposort.o -g
	gcc -c logger.c -o bin/logger.o -g
	gcc -c DriverHelper.c -o bin/DriverHelper.o
	gcc -o stage1exe bin/lexer.o bin/stack.o bin/DriverHelper.o bin/logger.o bin/toposort.o bin/trie.o bin/parser.o bin/ast.o bin/driver.o bin/symbolTable.o bin/typeChecker.o -g
	rm -r bin
	
clean:
	rm -r bin
	rm stage1exe
