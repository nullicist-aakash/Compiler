#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Lexer.h"
#include "../helpers/ErrorHandling.h"
#include "../helpers/Globals.h"

#define NUM_STATES 51
#define NUM_LINES_TEXT 54

char *sourceCode;
TokenList* tokenList;
int** transitions;
TokenType* finalStates;

// TODO: Transfer to global
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

    char BUFF[128];

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

void loadLexer()
{
    sourceCode = calloc(1, MAX_CODE_SIZE);
    tokenList = calloc(1, sizeof(TokenList));
    transitions = calloc(NUM_STATES, sizeof(int*));
    for (int i = 0; i < NUM_STATES; ++i)
    {
        transitions[i] = calloc(128, sizeof(int*));

        for (int i = 0; i < 128; ++i)
            transitions[i] = -1;
    }

    char** input = calloc(NUM_LINES_TEXT, sizeof(char*));
    loadData("./data/DFA_Structure.txt", input);

    for (int line = 0; line < NUM_LINES_TEXT; ++line)
    {
        int from, to;
        char BUFF[64];
        sscanf(input[line], "%d %d %s", from, to, BUFF);
        for (int i = 0; i < 64; ++i)
        {
            if (BUFF[i] == '\0')
                break;
            transitions[from][BUFF[i]] = to;
        }
    }
    for (int i = 0; i < 128; i++)
        transitions[48][i] = 48;

    transitions[48]['\n'] = 49;
    
    transitions[0][' '] =
        transitions[0]['\t'] =
        transitions[0]['\r'] =
        transitions[0]['\n'] = 50;

    transitions[50][' '] =
        transitions[50]['\t'] =
        transitions[50]['\r'] =
        transitions[50]['\n'] = 50;
    
    for (int i = 0; i < NUM_LINES_TEXT; i++)
        if (input[i] != NULL)
            free(input[i]);
    free(input);

    finalStates = calloc(NUM_STATES, sizeof(TokenType));

    for (int i = 0; i < NUM_STATES; ++i)
        finalStates[i] = -1;

    finalStates[1] = TK_NUM;
    finalStates[4] = TK_RNUM;
    finalStates[8] = TK_RNUM;
    finalStates[10] = TK_ID;
    finalStates[11] = TK_ID;
    finalStates[14] = TK_RUID;
    finalStates[18] = TK_LT;
    finalStates[21] = TK_ASSIGNOP;
    finalStates[22] = TK_LE;
    finalStates[24] = TK_GE;
    finalStates[26] = TK_EQ;
    finalStates[28] = TK_NE;
    finalStates[29] = TK_PLUS;
    finalStates[30] = TK_MINUS;
    finalStates[31] = TK_MUL;
    finalStates[32] = TK_DIV;
    finalStates[33] = TK_DOT;
    finalStates[34] = TK_COMMA;
    finalStates[35] = TK_COLON;
    finalStates[36] = TK_SEM;
    finalStates[39] = TK_AND;
    finalStates[42] = TK_OR;
    finalStates[43] = TK_NOT;
    finalStates[44] = TK_SQL;
    finalStates[45] = TK_SQR;
    finalStates[46] = TK_OP;
    finalStates[47] = TK_CL;
    finalStates[49] = TK_COMMENT;
    finalStates[50] = TK_WHITESPACE;    
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
    TokenType ttype;
    int last_final = -1;
    int input_final_pos = start_index - 1;

    int len = 0;

    int cur_state = 0;
    // start index = index of character to read next

    while (1)
    {
        char input = sourceCode[start_index];
        
        if (finalStates[cur_state] != -1)
        {
            last_final = cur_state;
            ttype = finalStates[cur_state];
            input_final_pos = start_index - 1;
        }

        cur_state = transitions[cur_state][input];

        if (cur_state == -1)    // return
        {
            if (len == 0)
                return NULL;

            Token* token = calloc(1, sizeof(Token));
            token->type = ttype;
            token->length = len;
            return token;
        }

        start_index++;
        len++;

    }

    // this should not be reachable as our DFA is capable of handling every case
    assert(0);
}

Token* lookUpTable(int start_index)
{
    return NULL;
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

        Token* token = lookUpTable(start_index);
        if (token == NULL)
            token = DFA(start_index);
        
        if (token == NULL)
        {
            token = calloc(1, sizeof(Token));
 
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

void cleanExtraMemory()
{
    for (int i = 0; i < 128; ++i)
        free(transitions[i]);
    free(transitions);
    transitions = NULL;

    free(sourceCode);
    sourceCode = NULL;

    while (tokenList->head != NULL)
    {
        // clean token
        if (tokenList->head->token->lexeme != NULL)
            free(tokenList->head->token->lexeme != NULL);

        free(tokenList->head->token);
        tokenList->head = tokenList->head->next;
    }

    free(tokenList);
    free(finalStates);
}