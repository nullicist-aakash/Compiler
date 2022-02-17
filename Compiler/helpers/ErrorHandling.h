#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H


typedef enum
{
    NO_ERROR = 0,
    FILE_READ_ERROR = 1,
    LEXICAL_ERROR = 2
} ErrorCode;

void displayError(char* message);

#endif