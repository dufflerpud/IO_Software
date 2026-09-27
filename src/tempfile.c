/************************************************************************
 *
 *indx#	tempfile.c - Software for creating files in /tmp
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
 *doc#	Software for creating files in /tmp
 ************************************************************************/
/***************************************************************************
	tempfile.c:  Routines to handle temporary files
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_tempfile == LOCAL_HANDLER

#include <sys/types.h>

/************************************************************************/
/************************************************************************/
char *mktemp(char *template)
    {
    char *result;
    result = template;
    while( *result++ ) ;
    result -= 6;
    sformat( result, "{iz6}", getpid() );
    }

/************************************************************************/
/************************************************************************/
IOFILE tmpfile()
    {
    IOFILE result;
    char *name;
    name = mktemp("/bin/TMF.XXXXXX");
    result = ioopen(name,"u");
    unlink( name );
    }

#ifndef L_tmpnam
#define L_tmpnam		14
#endif

/************************************************************************/
/************************************************************************/
char *tmpnam(char *s)
    {
    static time_t tmpindex = 0;
    static char localname[L_tmpnam];
    char *result;
    int p;

    if( tmpindex == 0 ) then time(&tmpindex);
    if( s==NULL ) then s=localname;
    result = s;
    *s++ = '%';
    for( p=getpid(); p!=0; p/=36 ) *s++ = bintodig(p%36);
    *s++ = '|';
    for( p=tmpindex++; p!=0; p/=36 ) *s++ = bintodig(p%36);
    *s++ = '_';
    *s++ = 0;
    return result;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_tempfile()
    {
    }
#endif
