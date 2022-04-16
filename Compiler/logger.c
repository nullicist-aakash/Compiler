/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

int wantToLog = 0;

void logIt(char *format, ...)
{
	if (!wantToLog)
		return;

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
}