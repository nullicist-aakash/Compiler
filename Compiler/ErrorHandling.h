#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H
#include <time.h>
#include <stdio.h>

typedef enum
{
    NO_ERROR = 0,
    FILE_READ_ERROR = 1,
    LEXICAL_ERROR = 2
} ErrorCode;

void displayError(char* message)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d\t--\t%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
}

ErrorCode errorCode = NO_ERROR;


#endif