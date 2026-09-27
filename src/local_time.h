/************************************************************************
 *
 *indx#	local_time.h - Include file for standalone time routines
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
 *doc#	Include file for standalone time routines
 ************************************************************************/
#ifndef LOCAL_time_INCLUDED
#define LOCAL_time_INCLUDED
#include <local.h>

#if HANDLER_time == SYSTEM_HANDLER
#include <sys/types.h>
#include <time.h>
#define setup_time()

#elif HANDLER_time == LOCAL_HANDLER
extern subroutine tzset( );
extern function struct tm *gmtime( time_t *nclock );
extern function struct tm *localtime( time_t *clock );
extern function time_t gmtotime( struct tm *t );
extern function time_t ltotime( struct tm *t );
extern function struct tm *parsetime( char *s );
extern function char *asctime( struct tm *t );
extern function char *ctime( time_t *ti );
extern function time_t gtime( char *s );
extern subroutine setup_time();
#endif
#endif
