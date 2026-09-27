/************************************************************************
 *
 *indx#	local_error.h - Include file for error handling
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
 *doc#	Include file for error handling
 ************************************************************************/
#ifndef STRERROR
#include <local.h>

#ifdef HAS_STRERROR
#define STRERROR(x)	strerror(x)
#else
extern const char* const sys_errlist[];
#define STRERROR(x)	sys_errlist[x]
#endif

#ifdef USE_NEW_ERRNO
#include <errno.h>
#else
extern int errno;
#endif

#endif /* STRERROR */

#define ERR_TRAP	0
#define ERR_SAY		1
#define ERR_PASS	2

#define traperrors()	seterrors( ERR_TRAP )
#define sayerrors()	seterrors( ERR_SAY )
#define passerrors()	seterrors( ERR_PASS )

extern int eformat( char *fmt, ... );
extern int exformat( char *fmt, va_list ap );
