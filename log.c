/*
 * =====================================================================================
 *
 *       Filename:  log.c
 *
 *    Description:  logging stuff
 *
 *        Version:  1.0
 *        Created:  13.09.2007 16:37:34 EEST
 *       Revision:  $Id: log.c 710 2008-07-31 21:32:55Z vlad $
 *       Compiler:  gcc
 *
 *         Author:  Vladislav Moskovets, mail: devvlad dot gmail dot com
 *
 * =====================================================================================
 */

#ifdef _WIN32
#include "stdafx.h"
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>


#include "log.h"

#ifdef LIBLOG_ENABLED

#define _CRT_SECURE_NO_DEPRECATE 1

#define LIBLOG_DEBUG_INDENT_CHAR_COUNT 3
#define LIBLOG_DEBUG_INDENT_CHAR " "
#define LIBLOG_DEBUG_ENTER_PREFIX "/ Entering"
#define LIBLOG_DEBUG_LEAVE_PREFIX "\\ Leaving"

char debug_buf[32768];
char debug_prefix_buf[1024];
int  debug_indent=0;
char debug_indent_buf[1024];
FILE *log_file = NULL;

void liblog_init(const char *filename)
{
	FILE *file;
	if (filename) {
		file = fopen(filename, "a+");
		if (file) {
#if 0
			fprintf(stderr, "opened '%s'\n", filename);
#endif
			liblog_set_file(file);
			liblog_first_line();
		} else
			fprintf(stderr, "failed to open '%s'\n", filename);
	} else 
		liblog_set_file(stderr);
}

void liblog_set_file(FILE* file)
{
	log_file = file;
}

/* Return prefix string for indention */
const char *liblog_get_indent() {
	int i,s;
	debug_indent_buf[0]='\0';
	if (debug_indent <= 0)
		return debug_indent_buf;
	for (i = 0; i < debug_indent; i++) 
		for ( s = 0; s < LIBLOG_DEBUG_INDENT_CHAR_COUNT; s++ )
			strcat(debug_indent_buf, LIBLOG_DEBUG_INDENT_CHAR); 
	return debug_indent_buf;
};

/* Print out stub */
void liblog_puts(const char * str) 
{
	fputs(str, log_file);
	fflush(log_file);
};

void liblog_done()
{
	liblog_print("DONE", "Close log file");	
	fclose(log_file);
}

void liblog_first_line()
{
	static int first=1;
	if (first) {
		liblog_puts("Started log: $LastChangedRevision: 710 $, $LastChangedBy: vlad $, $LastChangedDate: 2008-08-01 00:32:55 +0300 (Птн, 01 Авг 2008) $\n");
		first=0;
	}
}

/* Combine together prefix, format string and params */
void liblog_print(const char* prefix, const char* str, ...) 
{
	va_list ap;	
	char *cur = debug_buf;
	static int sem;
	int c=0;
	time_t t;
	struct tm *t1;

	while ( sem ) { usleep(1000); if ( (c % 1000*5) == 0) liblog_puts("semaphore timed out\n"); c++; };
	sem=1;
	liblog_first_line();

	time(&t);
	t1=localtime(&t);
	cur += SNPRINTF(cur, sizeof(debug_buf), BOLD"%04d%02d%02d %02d:%02d:%02d ", 1900 + t1->tm_year,t1->tm_mon+1,t1->tm_mday
                    ,t1->tm_hour,t1->tm_min,t1->tm_sec);
#ifdef _WIN32
	cur += SNPRINTF(cur, sizeof(debug_buf) - strlen(debug_buf) - 2, "[%04d:%04d]", GetCurrentProcessId(), GetCurrentThreadId());
#else
	cur += sprintf(cur, "[%d]"RESET, getpid());
#endif

   	cur += SNPRINTF(cur, sizeof(debug_buf) - strlen(debug_buf) - 2, "%s%s ", liblog_get_indent(), prefix);
	va_start(ap, str);
	vsnprintf(cur, sizeof(debug_buf)-200, str, ap); 
	strcat(cur, "\n");
	liblog_puts(debug_buf);
#ifdef WIN32
	if ( strstr(prefix, "ERR") ) 
		AfxMessageBox(debug_buf);
#endif	
	va_end(ap);
	sem=0;
	return; 
}


inline const char* liblog_get_stripped_filename(const char *filename)
{
	const char *striped_file = strrchr(filename, '\\');
	if ( striped_file ) 
		striped_file++;
	else
		striped_file = filename;
	return striped_file;
}

/* Return formatted string with Function name, File, and Line number */
const char* liblog_get_debug_prefix(const char *prefix, const char* file, int line, const char* func)
{
	SNPRINTF(debug_prefix_buf, sizeof(debug_prefix_buf), "%s %s() %s:%d", prefix, func, liblog_get_stripped_filename(file), line);
	return debug_prefix_buf; 
};

void liblog_none(const char* str, ...) { if(0 && str) ((void)0); return; }

/* Print 'ASSERT FAILED: main(), cgi.c:11' and exits */
void liblog_assert(const char* file, int line, const char* func, const char *expr_str)
{
	liblog_print(liblog_get_debug_prefix("ASSERT FAILED!!!:", liblog_get_stripped_filename(file), line, func), "%s", expr_str); 
	exit(1);
}

/* Print 'ASSERT FAILED: main(), cgi.c:11: 'File not found'' and exits */
void liblog_assert_perror(const char* file, int line, const char* func, int errornumber)
{
	liblog_print(liblog_get_debug_prefix("ASSERT FAILED!!!:", liblog_get_stripped_filename(file), line, func), "'%s'", strerror(errornumber)); 
	exit(1);
}

/* Print 'VERIFY FAILED: main(), cgi.c:11' and exits */
void liblog_verify(const char* file, int line, const char* func)
{
	liblog_print(liblog_get_debug_prefix("VERIFY FAILED!!!:", liblog_get_stripped_filename(file), line, func), ""); 
	exit(1);
}

void liblog_fenter(const char* file, int line, const char* func) { debug_indent++; liblog_print(LIBLOG_DEBUG_ENTER_PREFIX, "%s() %s:%d", func, liblog_get_stripped_filename(file), line); };
void liblog_fleave(const char* file, int line, const char* func) { liblog_print(LIBLOG_DEBUG_LEAVE_PREFIX, "%s %s:%d", func, liblog_get_stripped_filename(file), line); debug_indent--; };
void liblog_fenter_args(const char* file, int line, const char* func, const char* fmt, ...) 
{ 
	va_list ap;	
	char buf[2048];
	debug_indent++; 
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap); 
	va_end(ap);
	liblog_print(LIBLOG_DEBUG_ENTER_PREFIX, "%s(%s) %s:%d", func, buf, liblog_get_stripped_filename(file), line); 
};

void liblog_fleave_args(const char* file, int line, const char* func, const char* fmt, ...) 
{ 
	va_list ap;	
	char buf[2048];
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap); 
	va_end(ap);
	liblog_print(LIBLOG_DEBUG_LEAVE_PREFIX, "%s(%s) %s:%d", func, buf, liblog_get_stripped_filename(file), line); 
	debug_indent--; 
};

inline int is_printable(unsigned char c)
{
	if (c < 0x20 || c >= 0x7f )
		return 0;
	else 
		return 1;
}

inline char* hexbincolor(unsigned char c)
{
	if (c < 0x20) 
		return BLUE;
	else if ( c >= 0x7f )
		return RED;
	else
		return "";
}


char* liblog_get_hexdump(unsigned char *data, int bytes)
{
	int buf_size = 64*1024;
	char *buffer = malloc(buf_size);
	char *pcur = buffer;
	char *end = buffer+buf_size;
	char *color = NULL;
	int i, j;
	unsigned char c;

	ASSERT(buffer);

	for (i = 0; i < bytes; i++) {
		if (!(i % 8) && i)
			pcur += snprintf(pcur, end-pcur, " ");
		if (!(i % 16) && i) {
			pcur += snprintf(pcur, end-pcur, "  ");
			for (j = 0; j < 16; j++) {
				c = data[i+j-16];
				pcur += snprintf(pcur, end-pcur, "%s%c%s", hexbincolor(c), is_printable(c)? c : '.', RESET);
			}
			pcur += snprintf(pcur, end-pcur, "\n");
		}
		color = hexbincolor(data[i]);

		pcur += snprintf(pcur, end-pcur, "%s%.2x%s ", color, data[i], RESET);
	}
	j = (bytes % 16);
	j = (j != 0 ? j : 16);
	for (i = j; i < 16; i++) {
		if (!(i % 8) && i)
			pcur += snprintf(pcur, end-pcur, " ");
		pcur += snprintf(pcur, end-pcur, "   ");
	}
	printf("   ");
	for (i = bytes - j; i < bytes; i++) {
		c = data[i];
		if ((c < 0x20) || (c >= 0x7f))
			c = '.';
		pcur += snprintf(pcur, end-pcur, "%s%c%s", hexbincolor(c), c, RESET);
	}
	pcur += snprintf(pcur, end-pcur, "\n");
	return buffer;
}


void liblog_hexdump(const char* str, unsigned char *data, int bytes) 
{
	char * hexdump = liblog_get_hexdump(data, bytes);
	ASSERT(hexdump);
	liblog_print("HEXDUMP", "\n"ORANGE"----------- START HEXDUMP %s %d bytes ----------- "RESET"\n%s"ORANGE"===========  END  HEXDUMP %s %d bytes ==========="RESET, str, bytes, hexdump, str, bytes);
	free(hexdump);
}


#else
void liblog_none(const char* str, ...) { return; }

#endif






