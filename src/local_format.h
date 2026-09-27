/************************************************************************
 *
 *indx#	local_format.h - Include file for replacement for printf
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
 *doc#	Include file for replacement for printf
 ************************************************************************/
#ifndef LOCAL_FORMAT_DEFINED
#define LOCAL_FORMAT_DEFINED
#include <local.h>
#include <local_iosubs.h>

extern function int cformat( char *fmt, ... );
extern function int cxformat( char *fmt, va_list arglist );
extern function char *mformat( char *fmt, ... );
extern function int fdformat( int fd, char *fmt, ... );
extern function char *mxformat( char *fmt, va_list arglist );
extern function int fdxformat( int fd, char *fmt, va_list arglist );
extern function char *sformat( char *res, char *fmt, ... );
extern function char *sxformat( char *res, char *fmt, va_list arglist );
extern function int format( char *fmt, ... );
extern function int flformat( char *fmt, ... );
extern function int xformat( char *fmt, va_list arglist );
extern function int flxformat( char *fmt, va_list arglist );
extern function int fformat( IOFILE outfile, char *fmt, ... );
extern function int fflformat( IOFILE outfile, char *fmt, ... );
extern function int fxformat( IOFILE outfile, char *fmt, va_list arglist );
extern function int fflxformat( IOFILE outfile, char *fmt, va_list arglist );
extern subroutine filerr( char *fmt, ... );
extern subroutine setup_format();
#endif
