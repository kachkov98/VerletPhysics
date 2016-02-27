/**
@file
@brief implementation of logging system
@author Sergei Kachkov
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"

static FILE *f;

void InitLog ()
{
	if (!(f = fopen ("log.txt", "w")))
	{
		printf ("ERROR: Can not open log file!");
		exit (EXIT_FAILURE);
	}
}

void CloseLog ()
{
	fclose (f);
}

void log(LOG_TYPE type, const char *format_str, ...)
{
	time_t rawtime;
	time (&rawtime);
	char time_str[40];
	strftime (time_str, 40, "[%D %T] ", localtime (&rawtime));
	fputs (time_str, f);

	switch (type)
	{
	case LOG_INFO:
		fputs ("INFO: ", f);
		break;
	case LOG_FAIL:
		fputs ("FAIL: ", f);
		break;
	}

	va_list args;
	va_start (args, format_str);
	vfprintf (f, format_str, args);
	fputc ('\n', f);
	va_end (args);
	fflush (f);
	if (type == LOG_FAIL)
	{
		CloseLog ();
		exit (EXIT_FAILURE);
	}
}