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
