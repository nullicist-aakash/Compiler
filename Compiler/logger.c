#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

int wantToLog = 1;

void logIt(char* format, ...)
{
	if (!wantToLog)
		return;
	
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);	
}