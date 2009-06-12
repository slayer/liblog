/*
 * =====================================================================================
 * 
 *       Filename:  log.h
 * 
 *    Description:  logging stuff
 * 
 *        Version:  1.0
 *        Created:  13.09.2007 16:31:31 EEST
 *       Revision:  $Id: log.h 710 2008-07-31 21:32:55Z vlad $
 *       Compiler:  gcc
 * 
 *         Author:  Vladislav Moskovets, mail: devvlad dot gmail dot com
 * 
 * =====================================================================================
 */

#ifdef LIBLOG_ENABLED

#include <errno.h>

#ifndef LIBLOG_LOG_STDFILE
// #define LIBLOG_LOG_STDFILE stderr  // Uncomment it to use STDERR
// #define LIBLOG_LOG_STDFILE stdout  // Uncomment it to use STDOUT
#endif

#ifndef LIBLOG_LOG_FILE
#define LIBLOG_LOG_FILE "/dev/console"
#endif

void liblog_print(const char* prefix, const char* str, ...);
void liblog_fenter(const char* file, int line, const char* func);
void liblog_fleave(const char* file, int line, const char* func);
void liblog_assert(const char* file, int line, const char* func, const char *expr_str);
void liblog_verify(const char* file, int line, const char* func);
void liblog_assert_perror(const char* file, int line, const char* func, int errornumber);
void liblog_verify_perror(const char* file, int line, const char* func, int errornumber);

const char* liblog_get_debug_prefix(const char *prefix, const char* file, int line, const char* func);
void liblog_fenter_args(const char* file, int line, const char* func, const char* fmt, ...);
void liblog_fleave_args(const char* file, int line, const char* func, const char* fmt, ...);
void liblog_set_file(FILE*f);
void liblog_done();


#define LOG_INIT				((void) 0)
#define LOG_SET_FILE(f)			(liblog_set_file(f))
#define LOG_DONE				liblog_done()
#define RESET_ERRNO				(((errno)=(0)))
#ifndef _WIN32
#define ASSERT(expr)			((void)((expr) ? (0) : (liblog_assert(__FILE__, __LINE__, __FUNCTION__, "Expression: '"#expr"'"))))
#define VERIFY(expr)			((void)((expr) ? (0) : (liblog_assert(__FILE__, __LINE__, __FUNCTION__))))
#endif
#define ASSERT_PERROR			((void)((!(errno)) ? (0) : (liblog_assert_perror(__FILE__, __LINE__, __FUNCTION__, errno))));RESET_ERRNO
#define VERIFY_PERROR			((void)((!(errno)) ? (0) : (liblog_assert_perror(__FILE__, __LINE__, __FUNCTION__, errno))))
#define FENTER					(liblog_fenter(__FILE__, __LINE__, __FUNCTION__));RESET_ERRNO
#define FENTERA(fmt, ...)		(liblog_fenter_args(__FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__));RESET_ERRNO
#define FLEAVE					(liblog_fleave(__FILE__, __LINE__, __FUNCTION__));RESET_ERRNO
#define FLEAVEA(fmt, ...)		(liblog_fleave_args(__FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__));RESET_ERRNO
#define DBG(str, ...)			(liblog_print(((const char*) liblog_get_debug_prefix("| DEBUG:", __FILE__, __LINE__, __FUNCTION__)), str, __VA_ARGS__));
#define WARN(str, ...)			{ liblog_print("", "\n\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");\
		liblog_print(((const char*) liblog_get_debug_prefix("WARNING:", __FILE__, __LINE__, __FUNCTION__)), str, __VA_ARGS__);\
		liblog_print("", "\n\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");};
#define ERR(str, ...)			(liblog_print(((const char*) liblog_get_debug_prefix("ERROR:", __FILE__, __LINE__, __FUNCTION__)), str, __VA_ARGS__));

#else

#define LOG_INIT				((void) 0)
#define LOG_DONE				((void) 0)
#define LOG_SET_FILE(f)			((void) 0)
#define RESET_ERRNO				((void) (0))
#ifndef _WIN32
#define ASSERT(expr)			((void) (0))
#define VERIFY(expr)			((void) (expr))
#endif
#define ASSERT_PERROR			((void) (0))
#define VERIFY_PERROR			((void) (expr))
#define FENTER					((void)	(0))
#define FENTERA(fmt, ...)		((void) (0))
#define FLEAVE					((void)	(0))
#define FLEAVEA(fmt, ...)		((void)	(0))
#define DBG(str, ...)			((void) (0))
#define WARN(str, ...)			((void) (0))
#define ERR(str, ...)			((void) (0))
#endif

