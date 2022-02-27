compile:
	mkdir bin
	gcc -c Lexer/Lexer.c -o bin/Lexer.o
	gcc -c helpers/ErrorHandling.c -o bin/ErrorHandling.o
	# gcc -c helpers/Globals.h -o bin/Globals.o
	gcc -c Compiler.c -o bin/Compiler.o
	gcc -o Compiler.out bin/Lexer.o bin/ErrorHandling.o helpers/Globals.h bin/Compiler.o

clean:
	rm -r bin
	rm Compiler.out
