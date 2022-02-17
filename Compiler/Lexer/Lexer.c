#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Lexer.h"
#include "../helpers/ErrorHandling.h"
#include "../helpers/Globals.h"

char *sourceCode;
TokenList* tokenList;

void initialize()
{
    sourceCode = calloc(1, MAX_CODE_SIZE);
    tokenList = calloc(1, sizeof(TokenList));
}

void loadData(char* loc, char** arr)
{
    FILE* fptr = fopen(loc, "r");

    if (fptr == NULL)
    {
        char BUFF[100];
        sprintf(BUFF, "Could not open file: %s", loc);
        displayError(BUFF);
        errorCode = FILE_READ_ERROR;
        return;
    }

    char BUFF[9];

    int lines_read = 0;
    while (fscanf(fptr, "%s", BUFF) != EOF)
    {
        lines_read++;
        if (lines_read == 1)
            continue;

        arr[lines_read - 2] = calloc(strlen(BUFF) + 1, sizeof(char));
        strcpy(arr[lines_read - 2], BUFF);
    }

    printf("File: %s\n", loc);
    for (int i = 1; i < lines_read; ++i)
        printf("\t%s\n", arr[i - 1]);

    printf("\n");

    fclose(fptr);
}

void loadCode(char* loc)
{
    initialize();
    FILE* fptr = fopen(loc, "r");

    if (fptr == NULL)
    {
        displayError("Could not open code file");
        errorCode = FILE_READ_ERROR;
        return;
    }

    char ch, * code_ptr = sourceCode;
    int count = 0;

    while ((ch = fgetc(fptr)) != EOF)
    {
        count++;
        if (count < 4)
            continue;

        *code_ptr = ch;
        code_ptr++;
    }
    *code_ptr = '\0';
}

Token* DFA(int start_index)
{

}   

Token* symbolTable(int start_index)
{
    
}

void insertToken(Token* token)
{
    assert(token != NULL);

    TokenNode* node = calloc(1, sizeof(TokenNode));
    node->token = token;

    if (tokenList->head == NULL)
    {
        tokenList->head = tokenList->tail = node;
        return;
    }

    if (token->type == TK_ERROR && tokenList->tail->token->type == TK_ERROR)
    {
        tokenList->tail->token->length += token->length;
        free(token);

        return;
    }

    tokenList->tail->next = node;
    tokenList->tail = node;
}

TokenNode* getTokens()
{
    int line_number = 1;
    int start_index = 0;

    while(sourceCode[start_index] != '\0')
    {
        if (sourceCode[start_index] == '\n')
        {
            line_number++;
            start_index++;
            continue;
        }

        Token* token = symbolTable(start_index);
        if (token == NULL)
            token = DFA(start_index);
        
        if (token == NULL)
        {
            token = calloc(1,sizeof(Token));
            token->type = TK_ERROR;
            token->line_number = line_number;
            token->start_index = start_index;
            token->length = 1;
            insertToken(token);
        }

        token->start_index = start_index;
        token->line_number = line_number;

        start_index += token->length;

        if(token->type == TK_COMMENT || token->type ==  TK_WHITESPACE)
        {
            free(token);
            continue;
        }

        token->lexeme = calloc(token->length + 1, sizeof(char));

        for (int i = 0; i < token->length; i++)
            token->lexeme[i] = sourceCode[start_index + i];

        insertToken(token);
    }
}