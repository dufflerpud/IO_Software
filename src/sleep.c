/************************************************************************
 *
 *indx#	sleep.c - Standalone sleep routine
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
 *doc#	Standalone sleep routine
 ************************************************************************/
/***************************************************************************
	sleep.c:  Routines to delay execution
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_sleep == LOCAL_HANDLER

#ifdef STANDALONE

/************************************************************************/
/************************************************************************/
function unsigned int sleep( int seconds )
    {
    long time1, time2;
    time( &time1 );
    do  {
	time( &time2 );
	} while( time2 == time1 );
    do  {
	time( &time1 );
	} while( time1 < time2+seconds );
    return 0;
    }

#endif

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_sleep()
    {
    }
#endif
