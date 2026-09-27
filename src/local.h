/************************************************************************
 *
 *indx#	local.h - One place to set configuration variables
 *@HDR@	$Id$
 *@HDR@
 *@HDR@	Copyright (c) 2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
 *@HDR@
 *@HDR@	Permission is hereby granted, free of charge, to any person
 *@HDR@	obtaining a copy of this software and associated documentation
 *@HDR@	files (the "Software"), to deal in the Software without
 *@HDR@	restriction, including without limitation the rights to use,
 *@HDR@	copy, modify, merge, publish, distribute, sublicense, and/or
 *@HDR@	sell copies of the Software, and to permit persons to whom
 *@HDR@	the Software is furnished to do so, subject to the following
 *@HDR@	conditions:
 *@HDR@	
 *@HDR@	The above copyright notice and this permission notice shall be
 *@HDR@	included in all copies or substantial portions of the Software.
 *@HDR@	
 *@HDR@	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 *@HDR@	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *@HDR@	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 *@HDR@	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *@HDR@	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *@HDR@	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *@HDR@	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 *@HDR@	OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *hist#	2026-09-27 - Christopher.M.Caldwell0@gmail.com - Created
 ************************************************************************
 *doc#	One place to set configuration variables
 ************************************************************************/
#ifndef LOCAL_DEFINED
#define LOCAL_DEFINED

#define int8		char
#define int16		short
#define int32		long
#define int64		long long
#define uint8		unsigned int8
#define uint16		unsigned int16
#define uint32		unsigned int32
#define uint64		unsigned int64

#undef STANDALONE
#define BYTE_SWAPPED

#define USE_NEW_ERRNO

#undef DEFINE_STRCHR_AS_INDEX
#undef DEFINE_INDEX_AS_STRCHR
#define NEED_STRING
#define NEED_LCASE

#define SYSTEM_HANDLER		1
#define LOCAL_HANDLER		2
#define STDARG_HANDLER		3
#define VARARGS_HANDLER		4
#define SYS3_HANDLER		5
#define BSD_LEGACY_HANDLER	6

#define HANDLER_getcwd		SYSTEM_HANDLER
#define HANDLER_getenv		SYSTEM_HANDLER
#define HANDLER_mem		SYSTEM_HANDLER
#define HANDLER_parse		SYSTEM_HANDLER
#define HANDLER_settty		SYSTEM_HANDLER
#define HANDLER_sleep		SYSTEM_HANDLER
#define HANDLER_string		SYSTEM_HANDLER
#define HANDLER_system		SYSTEM_HANDLER
#define HANDLER_tempfile	SYSTEM_HANDLER
#define HANDLER_time		SYSTEM_HANDLER
#define HANDLER_args		STDARG_HANDLER

#if HANDLER_args == STDARG_HANDLER
#include <stdarg.h>
#define NEXT(mode)	va_arg(ap,mode)
#define SAVE_ARGS(x)
#define RESTORE_ARGS(x)
#define SAVE_ARGS_DECL
#define XIFY(ap,fmt,stmt1, stmt2 )	va_list ap; va_start(ap,fmt); stmt1; va_end(ap); stmt2;

#elif HANDLER_args == VARARGS_HANDLER
#include <vararg.h>
#define NEXT(mode)	((mode *)(al += sizeof(mode)))[-1]
#define SAVE_ARGS(x)
#define RESTORE_ARGS(x)
#define SAVE_ARGS_DECL
#endif

#define HAS_MEMSET

#define HAS_STRERROR

/*	Back in 1985, I thought these made C more readable.  I still
	do, but I don't use them any more because of the funny looks
	I get on the rare occasions they let me out.

	"function" and "subroutine" definitely make functions and
	functions returning nothing easier to search for if you don't
	happen to remember what you called something. */

#define then
#define function
#define subroutine void

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

#ifndef _B0
#define _B0		0x00000001
#define _B1		0x00000002
#define _B2		0x00000004
#define _B3		0x00000008
#define _B4		0x00000010
#define _B5		0x00000020
#define _B6		0x00000040
#define _B7		0x00000080
#define _B8		0x00000100
#define _B9		0x00000200
#define _B10		0x00000400
#define _B11		0x00000800
#define _B12		0x00001000
#define _B13		0x00002000
#define _B14		0x00004000
#define _B15		0x00008000
#define _B16		0x00010000
#define _B17		0x00020000
#define _B18		0x00040000
#define _B19		0x00080000
#define _B20		0x00100000
#define _B21		0x00200000
#define _B22		0x00400000
#define _B23		0x00800000
#define _B24		0x01000000
#define _B25		0x02000000
#define _B26		0x04000000
#define _B27		0x08000000
#define _B28		0x10000000
#define _B29		0x20000000
#define _B30		0x40000000
#define _B31		0x80000000
#endif


#ifdef notdef
#if HANDLER_FORMAT == LOCAL_HANDLER
#include <local_format.h>
#endif

#if HANDLER_getcwd == LOCAL_HANDLER
#include <local_getcwd.h>
#endif

#if HANDLER_getenv == LOCAL_HANDLER
#include <local_getenv.h>
#endif

/* iosubs.c: */
#if HANDLER_IOSUBS == LOCAL_HANDLER
#include <local_iosubs.h>
#endif

#if HANDLER_mem == LOCAL_HANDLER
#include <local_mem.h>
#endif

#if HANDLER_parse == LOCAL_HANDLER
#include <local_parse.h>
#endif

#if HANDLER_settty == LOCAL_HANDLER
#include <local_settty.h>
#endif

#if HANDLER_sleep == LOCAL_HANDLER
#include <local_sleep.h>
#endif

#if HANDLER_string == LOCAL_HANDLER
#include <local_string.h>
#endif

#if HANDLER_system == LOCAL_HANDLER
#include <local_system.h>
#endif

#if HANDLER_tempfile == LOCAL_HANDLER
#include <local_tempfile.h>
#endif

#if HANDLER_time == LOCAL_HANDLER
#include <local_time.h>
#endif
#endif

#endif
