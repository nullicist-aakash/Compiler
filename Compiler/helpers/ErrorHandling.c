#include "ErrorHandling.h"
#include <time.h>
#include <stdio.h>

ErrorCode errorCode = NO_ERROR;

void displayError(char* message)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d\t--\t%s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
}
