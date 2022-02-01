#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Lexer.h"

char code[MAX_CODE_SIZE];
char* keywords[KEYWORD_COUNT], * symbols[SYMBOL_COUNT], * operators[OPERATOR_COUNT];
ErrorCode errorCode;

void displayError(char* message)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d\t--\t%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
}

void loadDataFiles()
{
	FILE* fptr = fopen("./1Keywords.txt", "r");
    if(fptr == NULL)
    {
        displayError("Could not open file - Keyword.txt");
        errorCode = FILE_READ_ERROR;
        return;
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

void loadCode(char* loc)
{
    FILE* fptr = fopen(loc, "r");

    if (fptr == NULL) 
    {
        displayError("Could not open code file");
        errorCode = FILE_READ_ERROR;
        return;
    }
    
    char ch, * code_ptr = code;
    
    while ((ch = fgetc(fptr)) != EOF) 
    {
        *code_ptr = ch;
        code_ptr++;
    }
    *code_ptr = '\0';

    printf("%s", code);
}