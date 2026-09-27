/************************************************************************
 *
 *indx#	event.c - Log an event
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
 *doc#	Log an event
 ************************************************************************/
#include <local_iosubs.h>
#include <local_format.h>
#include <local_time.h>
#include <local_string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

#define LINKLOCK	"/etc/passwd"
#define LOCKFILE	"/tmp/event-{i}"

subroutine logevent( char *logname, char *fmt, ... )
    {
    char lockname[100];
    IOFILE logfile;
    long curtime;
    va_list ap;

    sformat( lockname, LOCKFILE, getpid() );
    while( link(LINKLOCK,lockname) < 1 ) sleep(1);
    if( (logfile=ioopen(logname,"a")) == NULL )
      then
	{
	perror(logname);
	unlink(lockname);
	return;
	}
    time( &curtime );
    fformat(logfile,"{sm24}: {sl8}({iz3}):  ",
	ctime(&curtime),getlogin(),geteuid());
    va_start( ap, fmt );
    fxformat(logfile,fmt,ap);
    va_end( ap );
    fformat(logfile,"\n");
    ioclose(logfile);
    if( unlink( lockname ) < 0 ) then { perror(lockname); exit(1); }
    }

function int permission( char *filename, char *fmt, ... )
    {
    char action[100], name[100], request[100];
    IOFILE pfile;
    struct passwd *pw;
    int match;
    va_list ap;

    if( (pw=getpwuid(getuid())) == NULL ) then return FALSE;
    va_start( ap, fmt );
    sxformat( request, fmt, ap );
    va_end( ap );
    if( (pfile=ioopen(filename,"r")) == NULL ) then return FALSE;
    while( fscanf(pfile,"%s",action)==1 )
	{
	match = ( strcmp(action,request) == 0 );
	while( fscanf(pfile,"%s",name)==1 && strcmp(name,".")!=0 )
	    {
	    if( match && strcmp(name,pw->pw_name)==0 )
	      then
		{
		ioclose( pfile );
		return TRUE;
		}
	    }
	}
    ioclose( pfile );
    return FALSE;
    }
