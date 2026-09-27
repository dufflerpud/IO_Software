/************************************************************************
 *
 *indx#	iosubs.c - Software to do buffered i/o (replacement for stdio)
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
 *doc#	Software to do buffered i/o (replacement for stdio)
 ************************************************************************/
/***************************************************************************
	iosubs.c (I/O routines to replace stdio)
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#define _INIO_
#include <local.h>

#if HANDLER_IOSUBS == LOCAL_HANDLER
#ifdef STANDALONE

#ifdef UNIXEMULATOR
static iostr Istdin	= { read,  0,        0, NULL, 0, 0, 0, NULL, 0 };
static iostr Istdout	= { write, IO_WRITE, 1, NULL, 0, 0, 0, NULL, 0 };
static iostr Istderr	= { write, IO_WRITE, 2, NULL, 0, 0, 0, NULL, 0 };
IOFILE stdin		= &Istdin;
IOFILE stdout		= &Istdout;
IOFILE stderr		= &Istderr;
#else
IOFILE stdin, stdout, stderr;
#endif

/**************************************************************************/
/***	This routine gets one character from the buffer.  If necessary	***/
/***	it calls the function in iofunc to get a new buffer.		***/
/**************************************************************************/
function int ioinc(IOFILE p)
    {
    int res;
    if( p == NULL ) then return IOERROR;
    if( p->io_pbufsize != 0 )
      then
	{
	res = p->io_buf[ --(p->io_pbufsize) ] & 0xff;
	if( p->io_pbufsize != 0 ) then realloc( p->io_pbuf, p->io_pbufsize );
	return res;
	}
    if( p->io_buf == NULL )
      then
	{
	if( p->io_bufsize == 0 ) then p->io_bufsize = BUFSIZE;
	p->io_buf = malloc( p->io_bufsize );
	p->io_status |= IO_MALBUF;
	}
    if( p->io_status & IO_WRITE ) then return IOERROR;
    if( p->io_ind >= p->io_nread )
      then
	{
	p->io_nread = (*(p->io_func))( p->io_file, p->io_buf, p->io_bufsize );
	p->io_ind = 0;
	}
    if( p->io_ind >= p->io_nread ) then return IOERROR;
    return p->io_buf[ p->io_ind++ ] & 0xff;
    }

/**************************************************************************/
/***	This routine write out one buffer.				***/
/**************************************************************************/
function int ioflush(IOFILE p)
    {
    int res;
    if( p == NULL ) then return IOERROR;
    if( p->io_buf==NULL || p->io_ind==0 ) then return 0;
    res = (*(p->io_func))( p->io_file, p->io_buf, p->io_ind );
    p->io_ind = 0;
    return res;
    }

/**************************************************************************/
/***	This routine puts one character into the buffer.  If necessary	***/
/***	it calls the function in iofunc to dump the buffer.		***/
/**************************************************************************/
function int iooutc(IOFILE p,char c)
    {
    if( p == NULL ) then return IOERROR;
    if( p->io_buf == NULL )
      then
	{
	if( p->io_bufsize == 0 ) then p->io_bufsize = BUFSIZE;
	p->io_buf = malloc( p->io_bufsize );
	p->io_status |= IO_MALBUF;
	}
    p->io_buf[ p->io_ind++ ] = c;
    if( p->io_ind >= p->io_bufsize ) then return ioflush(p);
    return IOSUCCESS;
    }

/**************************************************************************/
/***	This routine moves to a new position in the file.  If output,	***/
/***	the buffer is flushed.  If input, the data is discarded.	***/
/**************************************************************************/
function long ioseek( IOFILE p, long offset, int where )
    {
    if( p == NULL ) then return IOERROR;
    if( p->io_status & IO_WRITE )
      then ioflush(p);
      else
	{
	p->io_pbufsize = 0;
	if( p->io_pbuf != NULL ) then p->io_pbuf = realloc( p->io_pbuf, 1 );
	}
    p->io_ind = p->io_nread = 0;
    return lseek( p->io_file, offset, where );
    }

/**************************************************************************/
/***	This routine puts a character back into the buffer.		***/
/**************************************************************************/
function iobackc( IOFILE p, char c )
    {
    if( p==NULL || (p->io_status&IO_WRITE) ) then return IOERROR;
    if( p->io_ind != 0 )
      then p->io_buf[ --(p->io_ind) ] = c;
      else
	{
	if( p->io_pbuf == NULL )
	  then p->io_pbuf = malloc( p->io_pbufsize+1 );
	  else p->io_pbuf = realloc( p->io_pbuf, p->io_pbufsize+1 );
	p->io_pbuf[ p->io_pbufsize++ ] = c;
	}
    }

/**************************************************************************/
/***	This routine sets up a buffer mechanism on the specified chan.	***/
/**************************************************************************/
function IOFILE ioassociate( int chan, int (*func)(), int status )
    {
    IOFILE p;

    if( (p = (IOFILE)malloc(sizeof(iostr))) == NULL ) return NULL;
    p->io_file		= chan;
    p->io_status	= status | IO_MALBLK;
    p->io_func		= func;
    p->io_buf		= NULL;
    p->io_bufsize	= BUFSIZE;
    p->io_ind		= 0;
    p->io_nread		= 0;
    p->io_pbuf		= NULL;
    p->io_pbufsize	= 0;
    return p;
    }

/**************************************************************************/
/***	This routine frees memory associated with a file.		***/
/**************************************************************************/
function int iodisassociate( IOFILE p )
    {
    if( p==NULL ) then return IOERROR;
    if( p->io_status & IO_WRITE ) then ioflush(p);
    if( (p->io_status&IO_MALBUF) && p->io_buf!=NULL )
      then
        {
	free( p->io_buf );
	p->io_status &= ~IO_MALBUF;
	p->io_buf = NULL;
	p->io_bufsize = 0;
	}
    if( p->io_pbuf != NULL )
      then
        {
	free( p->io_pbuf );
	p->io_pbuf = NULL;
	p->io_pbufsize = 0;
	}
    p->io_ind = p->io_nread = 0;
    if( p->io_status & IO_MALBLK ) then free( p );
    return IOSUCCESS;
    }

/**************************************************************************/
/***	This routine opens a file similar to openf.			***/
/**************************************************************************/
function IOFILE ioopen(char *filename,char *mode)
    {
    int filenum;
    IOFILE p;

    switch( mode[0] )
	{
	case 'r':	if( (filenum=open(filename,0)) < 0 )
			  then return NULL;
			return ioassociate( filenum, read, 0 );

	case 'w':	if( (filenum=creat(filename,0)) < 0 )
			  then return NULL;
			return ioassociate( filenum, write, IO_WRITE );

	case 'a':	if( (filenum=open(filename,1)) < 0 )
			  then
			    if( (filenum=creat(filename,0)) < 0 )
			      then return NULL;
			if( lseek( filenum, 0L, 2 ) < 0 )
			  then { close(filenum); return NULL; }
			return ioassociate( filenum, write, IO_WRITE );
	}
    }

/**************************************************************************/
/***	This routine closes a file (flushing buffers when necessary).	***/
/**************************************************************************/
function int ioclose(IOFILE p)
    {
    int filenum;
    if( p == NULL ) then return IOERROR;
    filenum = p->io_file;
    iodisassociate(p);
    return close(filenum);
    }

/**************************************************************************/
/***	This routine spawns off another process to execute a command.	***/
/**************************************************************************/
function IOFILE iopipe( char *com, char *mode )
    {
    int pipechans[2];
    IOFILE result;

    if( pipe( pipechans ) < 0 ) then return NULL;
    if( fork() )
      then
	if( mode[0]=='r' )
	  then
	    {
	    close( pipechans[1] );
	    return ioassociate( pipechans[0], read, 0 );
	    }
	  else
	    {
	    close( pipechans[0] );
	    return ioassociate( pipechans[1], write, IO_WRITE );
	    }
      else
	{
	if( mode[0]=='r' )
	  then
	    {
	    close( pipechans[0] );
	    close( 1 );
	    dup( pipechans[1] );
	    close( pipechans[1] );
	    }
	  else
	    {
	    close( pipechans[1] );
	    close( 0 );
	    dup( pipechans[0] );
	    close( pipechans[0] );
	    }
	execl("/bin/sh","sh","-c",com,0);
	exit(1);
	}
    }

/**************************************************************************/
/**************************************************************************/
function int canseek( int chan )
    {
    struct stat statbuf;
    if( lseek(chan,0L,1) < 0 )		then return FALSE;
    if( fstat(chan,&statbuf) < 0 )	then return FALSE;
    if( statbuf.st_mode != S_IFREG )	then return FALSE;
    return TRUE;
    }

/**************************************************************************/
/**************************************************************************/
function int iocanseek( IOFILE iof )
    {
    return canseek( IOCHAN( iof ) );
    }

#define MSBUFSIZE	1024

/**************************************************************************/
/**************************************************************************/
function int makeseek( int chan )
    {
    char buf[ MSBUFSIZE ];
    int nread, inchan, outchan;

    if( canseek( chan ) ) then return chan;

    sformat(buf,"/tmp/MS.{i}",getpid());
    if( (outchan = creat( buf, 0666 )) < 0 ) then filerr( buf );
    if( (inchan = open( buf, 0 )) < 0 ) then filerr( buf );
    if( unlink( buf ) < 0 ) then filerr( buf );
    while( (nread=read( chan, buf, MSBUFSIZE )) >= 0 )
	if( nread > 0 ) then write( outchan, buf, nread );
    close( chan );
    close( outchan );
    return inchan;
    }

/**************************************************************************/
/**************************************************************************/
function IOFILE iomakeseek( IOFILE iof )
    {
    char buf[ MSBUFSIZE ];
    int nread, outchan;
    IOFILE inchan;

    if( iocanseek( iof ) ) then return iof;

    sformat(buf,"/tmp/MS.{i}",getpid());
    if( (outchan = creat( buf, 0666 )) < 0 ) then filerr( buf );
    if( (inchan = ioopen( buf, "r" )) == NULL ) then filerr( buf );
    if( unlink( buf ) < 0 ) then filerr( buf );
    while( (nread=ioread( iof, buf, MSBUFSIZE )) >= 0 )
	if( nread > 0 ) then write( outchan, buf, nread );
    ioclose( iof );
    close( outchan );
    return inchan;
    }
#endif

/**************************************************************************/
/***	This routine outputs a "word" (an int).				***/
/**************************************************************************/
function int iooutw(IOFILE p,int w)
    {
    return iowrite( p, &w, sizeof(w) );
    }

/**************************************************************************/
/***	This routine inputs a "word" (an int).				***/
/**************************************************************************/
function int ioinw(IOFILE p)
    {
    int res, w;
    if( (res=ioread( p, &w, sizeof(w) )) < 0 )
      then return res;
      else return w;
    }

/**************************************************************************/
/***	This routine outputs sizebuf bytes.				***/
/**************************************************************************/
function int iowrite( IOFILE p, char *buf, int sizebuf )
    {
    int res = 0;
    while( sizebuf-- > 0 && res >= 0 ) res = iooutc(p,*buf++);
    return res;
    }

/**************************************************************************/
/***	This routine inputs sizebuf bytes.				***/
/**************************************************************************/
function int ioread( IOFILE p, char *buf, int sizebuf )
    {
    int numread = 0;
    for( numread=0; sizebuf-- > 0; numread++ )
	if( (*buf++ = ioinc(p)) < 0 ) then return numread;
    return numread;
    }

/**************************************************************************/
/***	This routine inputs up to sizebuf bytes or a carriage return.	***/
/**************************************************************************/
function int iogets( IOFILE p, char *buf, int sizebuf )
    {
    int numread = 0;
    while( --sizebuf > 0 )
	if( (*buf = ioinc(p)) < 0 )
	  then break;
	  else
	    {
	    numread++;
	    if( *buf++ == '\n' ) then break;
	    }
    if( *buf!='\n' && numread==0 ) then return IOERROR;
    *buf = 0;
    return numread;
    }

/**************************************************************************/
/***	This routine inputs up to carriage return and returns them in	***/
/***	a malloced array.						***/
/**************************************************************************/
function int miogets( IOFILE p, char **s )
    {
    int c, numread = 0;
    strinit( s );
    while( (c=ioinc(p)) >= 0 )
	{
	stradd( s, c );
	numread++;
	if( c == '\n' ) then break;
	}
    strdone( s );
    if( numread==0 ) then return IOERROR;
    return numread;
    }

/**************************************************************************/
/***	This routine inputs up to sizebuf bytes or a carriage return.	***/
/**************************************************************************/
function int iogetl( IOFILE p, char *buf, int sizebuf )
    {
    int numread = 0;
    while( --sizebuf > 0 )
	if( (*buf=ioinc(p)) < 0 || *buf=='\n' )
	  then break;
	  else buf++, numread++;
    if( *buf!='\n' && numread==0 ) then return IOERROR;
    *buf = 0;
    return numread;
    }

/**************************************************************************/
/***	This routine inputs up to carriage return and returns them in	***/
/***	a malloced array.						***/
/**************************************************************************/
function int miogetl( IOFILE p, char **s )
    {
    int c, numread;
    strinit( s );
    for( numread=0; (c=ioinc(p))>=0; numread++ )
	if( c == '\n' )
	  then break;
	  else stradd( s, c );
    strdone( s );
    if( c<0 && numread==0 ) then return IOERROR;
    return numread;
    }

/**************************************************************************/
/***	This routine outputs a string.					***/
/**************************************************************************/
function int ioputs( IOFILE p, char *s )
    {
    while( *s ) if( iooutc( p, *s++ ) < 0 ) then return IOERROR;
    }

/**************************************************************************/
/***	This routine outputs a string (and adds a end of line).		***/
/**************************************************************************/
function int ioputl( IOFILE p, char *s )
    {
    while( *s ) if( iooutc( p, *s++ ) < 0 ) then return IOERROR;
    if( iooutc( p, '\n' ) < 0 ) then return IOERROR;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_iosubs()
    {
    }
#endif
