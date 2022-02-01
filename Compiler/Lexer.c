#include "Lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEYWORD_COUNT	33
#define SYMBOL_COUNT	21
#define OPERATOR_COUNT	36

char* keywords[KEYWORD_COUNT], * symbols[SYMBOL_COUNT], * operators[OPERATOR_COUNT];
char* code;

void displayError(char* message)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d\t--\t%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
}

void loadFiles()
{
	FILE* fptr = fopen("./Keywords.txt", "r");

    if(fptr == NULL)
    {
        displayError("Could not open file");
        exit(1);
    }

    char BUFF[9];

    int lines_read = 0;
    while(fscanf(fptr, "%s", BUFF) != EOF)
    {
        lines_read++;
        if (lines_read == 1)
            continue;

        printf("%s\n", BUFF);
    }
    
}

void ReadCode(char* loc)
{

}