/************************************************************************
 *
 *indx#	format.c - Very much like printf and related software
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
 *doc#	Very much like printf and related software, except
 #doc#		printf("text%*.2f",10,floatvar)
 #doc#	becomes
 #doc#		format("text{fr.2}",floatvar,10)
 #doc#	known modifiers are:
 #doc#		l	- left justify
 #doc#		r	- right justify
 #doc#		c	- center spacing (center justify)
 #doc#		.	- decimal places
 #doc#		p	- specify pad character
 #doc#	and object types are:
 #doc#		s	- string
 #doc#		f	- float
 #doc#		i	- int
 #doc#		c	- character
 #doc#		l	- long
 ************************************************************************/
/***************************************************************************
	format.c:  Formatted output routines
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include <local_format.h>
#include <local_string.h>
#include <local_mem.h>
#include <local_error.h>
#include <unistd.h>		/* Definition of write() */

#define BIGBUFSIZE	102400

#define PF_COUNT	0
#define PF_ARRAY	1
#define PF_PUTC		2

int _allowfrees;

/**************************************************************************/
/***	Handle one character (count, add to array or write to file)	***/
/**************************************************************************/
int pf_one( char c, int *pf_count, char **pf_addrp, IOFILE pf_file )
    {
    if( pf_count )				then (*pf_count)++;
    if( pf_addrp && *pf_addrp )			then *(*pf_addrp)++ = c;
    if( pf_file && iooutc(pf_file,c)==IOEOF )	then return IOEOF;
    return 1;
    }

/**************************************************************************/
/***	Analyze a format string and print character by character.	***/
/**************************************************************************/
static int pf( char *fs, va_list ap, int *pf_count, char *pf_addr, IOFILE pf_file )
    {
    char *CMC_pf_addr = pf_addr;
    int pf_right, pf_left, pf_center, pf_dec, pf_max;
    int pf_base, pf_iter, pf_pad, pf_unsigned, pf_zeros, pf_free;
    char *pf_except;
    char *pf_string, pf_char;
    int pf_int;
    long pf_long;
    double pf_double;
    double pnum;
    int ind;
    char t, c, buf[512], *cp, *ls;
    int bufcnt;
    int pbase;
    int nm;
    int localfree;

    while( c = *fs++ )
	if( c != '{' || (t = *fs++) == '{' )
	  then
	    {
	    if( pf_one( c, pf_count, &pf_addr, pf_file ) == IOEOF )
	      then return IOEOF;
	    }
	  else
	    {
	    pf_right	= 0;
	    pf_left	= 0;
	    pf_center	= 0;
	    pf_dec	= 6;
	    pf_max	= 0;
	    pf_base	= 10;
	    pf_iter	= 1;
	    pf_pad	= ' ';
	    pf_unsigned	= FALSE;
	    pf_zeros	= 0;
	    pf_free	= FALSE;
	    pf_except	= NULL;
	    localfree	= FALSE;
	    switch( t )
		{
		case 'S':	pf_string	= NEXT(char *);		break;
		case 's':	pf_string	= NEXT(char *);		break;
		case 'c':	pf_char		= NEXT(int);		break;
		case 'i':	pf_int		= NEXT(int);		break;
		case 'l':	pf_long		= NEXT(long);		break;
		case 'f':	pf_double	= NEXT(double);		break;
		case 'p':	pf_int		= NEXT(int);		break;
		case 'n':						break;
		}
	    while( (c = *fs++) != '}' )
		{
		if( isdigit(c) )
		  then
		    {
		    fs--;
		    if( t=='s' || t=='S' )
		      then c = 'l';
		      else
			if( c == '0' )
			  then c = 'z';
			  else c = 'r';
		    }
		nm = 0;
		pbase = 10;
		if( !isbase(*fs,pbase) )
		  then nm = NEXT(int);
		  else
		    {
		    while( TRUE )
			{
			while( isbase(*fs,pbase) )
			    nm = pbase*nm + digtobin(*fs++);
			if( nm<2 || nm>36 || *fs!='_' ) then break;
			pbase = nm;
			fs++;
			}
		    }
		switch( c )
		    {
		    case 'r':	pf_right = nm;		break;
		    case 'l':	pf_left = nm;		break;
		    case 'c':	pf_center = nm;		break;
		    case '.':	pf_dec = nm;		break;
		    case 'm':	pf_max = nm;		break;
		    case 'b':	pf_base = nm;		break;
		    case 'n':	pf_iter = nm;		break;
		    case 'p':	pf_pad = nm;		break;
		    case 'u':	pf_unsigned = nm;	break;
		    case 'z':	pf_zeros = nm;		break;
		    case 'f':	pf_free = nm;		break;
#ifdef WILL_NOT_COMPILE
		    case 'e':	pf_except = (char*)nm;	break;
#endif
		    }
		}
	    bufcnt = 0;
	    if( !_allowfrees ) then pf_free = 0;
	    switch( t )
		{
		case 'S':
		case 's':	if( t == 's' )
				  then
				    if( pf_string == NULL )
				      then bufcnt = strlen(cp="(Null)");
				      else
					{
				        cp = pf_string;
					if( pf_zeros != 0 )
					  then bufcnt = pf_zeros;
					  else bufcnt = strlen( cp );
					}
				  else
				    {
				    SAVE_ARGS_DECL;
				    SAVE_ARGS( ap );
				    pf(pf_string,ap,&bufcnt,NULL,NULL);
				    RESTORE_ARGS( ap );
				    cp = malloc( bufcnt+1 );
				    pf(pf_string,ap,NULL,cp,NULL);
				    if( pf_zeros != 0 ) then bufcnt = pf_zeros;
				    localfree = TRUE;
				    }
				if( !pf_unsigned ) then break;
				ls = malloc( bufcnt*4 + 1 );
				for( ind=nm=0; nm<bufcnt; nm++ )
				    if( (  (cp[nm] > ' '		&&
					    cp[nm] <= '~'		&&
					    cp[nm] != '\\') )		||
					(  (pf_except!= NULL )		&&
					    strchr(pf_except,cp[nm])!=NULL )
					)
				      then ls[ind++] = cp[nm];
				      else
					{
					ls[ind++] = '\\';
					if(cp[nm]=='\\') then ls[ind++]='\\';
					else if(cp[nm]=='\n')then ls[ind++]='n';
					else if(cp[nm]=='\r')then ls[ind++]='r';
					else if(cp[nm]=='\f')then ls[ind++]='f';
					else if(cp[nm]=='\t')then ls[ind++]='t';
					else if(cp[nm]=='\b')then ls[ind++]='b';
					  else
					    {
					    ls[ind++] = ((cp[nm]>>6)&03) + '0';
					    ls[ind++] = ((cp[nm]>>3)&07) + '0';
					    ls[ind++] = (cp[nm]&07) + '0';
					    }
					}
				if( localfree ) then free( cp );
				cp = ls;
				localfree = TRUE;
				bufcnt = ind;
				break;

		case 'c':	cp = buf;
				if( !pf_unsigned			||
				    (pf_char > ' '		&&
					pf_char <= '~'		&&
					pf_char != '\\')		||
				    (pf_except!=NULL		&&
					strchr(pf_except,pf_char)!=NULL) )
				  then
				    {
				    buf[bufcnt++] = pf_char;
				    break;
				    }
				buf[bufcnt++] = '\\';
				if(pf_char=='\\') then buf[bufcnt++]='\\';
				else if(pf_char=='\n') then buf[bufcnt++]='n';
				else if(pf_char=='\r') then buf[bufcnt++]='r';
				else if(pf_char=='\f') then buf[bufcnt++]='f';
				else if(pf_char=='\t') then buf[bufcnt++]='t';
				else if(pf_char=='\b') then buf[bufcnt++]='b';
				  else
				    {
				    buf[bufcnt++] = ((pf_char>>6)&03) + '0';
				    buf[bufcnt++] = ((pf_char>>3)&07) + '0';
				    buf[bufcnt++] = (pf_char&07) + '0';
				    }
				break;

		case 'i':	if( pf_int < 0 )
				  then pf_char = 1;
				  else pf_char = 0;
				if( pf_int >= 0 || !pf_unsigned )
				  then
				    {
				    do  {
					buf[100-(++bufcnt)]
					    = bintodig( abs(pf_int%pf_base) );
					pf_int /= pf_base;
					} while( pf_int != 0 );
				    while( bufcnt < pf_zeros-pf_char )
					buf[100-(++bufcnt)] = '0';
				    if( pf_char ) then buf[100-(++bufcnt)]='-';
				    cp = buf;
				    cp += (100 - bufcnt);
				    }
				  else
				    {
				    c = pf_int & 1;
				    pf_int >>= 1;
				    pf_int &= ~(1<<(8*sizeof(pf_int)-1));
				    c = c + ((pf_int%(pf_base>>1)) << 1);
				    pf_int /= (pf_base>>1);
				    buf[100-(++bufcnt)] = bintodig( c );
				    while( pf_int != 0 )
					{
					buf[100-(++bufcnt)]
					    = bintodig( pf_int%pf_base );
					pf_int /= pf_base;
					}
				    while( bufcnt < pf_zeros )
					buf[100-(++bufcnt)] = '0';
				    cp = buf;
				    cp += (100 - bufcnt);
				    }
				break;

		case 'l':	if( pf_long < 0 )
				  then pf_char = 1;
				  else pf_char = 0;
				if( pf_long >= 0 || !pf_unsigned )
				  then
				    {
				    do  {
					buf[100-(++bufcnt)] = bintodig(
					    abs((int)(pf_long%pf_base)) );
					pf_long /= pf_base;
					} while( pf_long != 0 );
				    while( bufcnt < pf_zeros-pf_char )
					buf[100-(++bufcnt)] = '0';
				    if( pf_char ) then buf[100-(++bufcnt)]='-';
				    cp = buf;
				    cp += (100 - bufcnt);
				    }
				  else
				    {
				    c = pf_long & 1;
				    pf_long >>= 1;
				    pf_long &= ~(1L<<(8*sizeof(pf_long)-1));
				    c += ((pf_long%(pf_base>>1)) << 1);
				    pf_long /= (pf_base>>1);
				    buf[100-(++bufcnt)] = bintodig( c );
				    while( pf_long != 0 )
					{
					buf[100-(++bufcnt)] = bintodig(
					    (int)(pf_long%pf_base) );
					pf_long /= pf_base;
					}
				    while( bufcnt < pf_zeros )
					buf[100-(++bufcnt)] = '0';
				    cp = buf;
				    cp += (100 - bufcnt);
				    }
				break;

		case 'f':	if( pf_double < 0 )
				  then
				    {
				    buf[bufcnt++] = '-';
				    pf_double = -pf_double;
				    }
				pnum = 1.0;
				for( ind=0; ind<pf_dec; ind++ ) pnum /= pf_base;
				pf_double += (pnum / 2.0);
				ind = 0;
				for(pnum=1.0; pnum<=pf_double; pnum*=pf_base)
				    ind--;
				if( pnum != 1.0 )
				  then pnum /= pf_base;
				  else ind--;
				while( bufcnt < pf_zeros + ind )
				    buf[bufcnt++] = '0';
				do  {
				    if( ind++ == 0 ) then buf[bufcnt++]='.';
				    c = (int)(pf_double/pnum);
				    buf[bufcnt++] = bintodig( c );
				    pf_double -= (pnum*c);
				    pnum /= pf_base;
				    } while( ind < pf_dec );
				cp = buf;
				break;

		case 'p':	bufcnt = 0;
				pf_left += pf_int;
				break;

		case 'n':	bufcnt = 0;
				break;
		}
	    if( bufcnt > pf_max && pf_max > 0 ) then bufcnt = pf_max;
	    if( pf_center > 0 )
	      then
		{
		pf_left = ( pf_center - bufcnt ) / 2;
		pf_right = pf_center - pf_left - bufcnt;
		}
	      else
		{
		pf_left -= bufcnt;
		pf_right -= bufcnt;
		}
	    while( pf_iter-- > 0 )
		{
		for( ind=0; ind<pf_right; ind++ )
		    {
		    if( pf_one( pf_pad, pf_count, &pf_addr, pf_file ) == IOEOF )
		      then return IOEOF;
		    }
		for( ind=0; ind<bufcnt; ind++ )
		    {
		    if( pf_one( cp[ind], pf_count, &pf_addr, pf_file ) == IOEOF )
		      then return IOEOF;
		    }
		for( ind=0; ind<pf_left; ind++ )
		    {
		    if( pf_one( pf_pad, pf_count, &pf_addr, pf_file ) == IOEOF )
		      then return IOEOF;
		    }
		}
	    if( localfree ) then free( cp );
	    if( pf_free != 0 ) then free( pf_string );
	    }
    if( pf_addr ) then *pf_addr = 0;
    return 0;
    }

/**************************************************************************/
/***	Count characters that would be printed according to format	***/
/**************************************************************************/
int cxformat(char *fmt,va_list arglist)
    {
    int pf_count = 0;
    _allowfrees = TRUE;
    pf( fmt, arglist, &pf_count, NULL, NULL );
    return pf_count;
    }

/**************************************************************************/
/***	Count characters that would be printed according to format	***/
/**************************************************************************/
int cformat(char *fmt,...)
    {
    XIFY(ap,fmt,int ret=cxformat(fmt,ap),return ret);
    }

/**************************************************************************/
/***	Allocate an array and print into the array			***/
/**************************************************************************/
char *mxformat(char *fmt,va_list arglist)
    {
    int pf_count = 0;
    _allowfrees = TRUE;
    char *res = malloc( BIGBUFSIZE );
    pf( fmt, arglist, &pf_count, res, NULL );
    return realloc( res, pf_count+1 );
    }

/**************************************************************************/
/***	Allocate an array and print into the array			***/
/**************************************************************************/
char *mformat(char *fmt,...)
    {
    XIFY(ap,fmt,char *ret=mxformat(fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write to the specified file descriptor				***/
/**************************************************************************/
int fdxformat(int fd,char *fmt,va_list arglist)
    {
    int ires, pf_count = 0;
    _allowfrees = TRUE;
    char *res = malloc( BIGBUFSIZE );
    pf( fmt, arglist, &pf_count, res, NULL );
    ires = write( fd, res, pf_count );
    free( res );
    return ires;
    }

/**************************************************************************/
/***	Write to the specified file descriptor				***/
/**************************************************************************/
int fdformat(int fd,char *fmt,...)
    {
    XIFY(ap,fmt,int ret=fdxformat(fd,fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write to the specified array.					***/
/**************************************************************************/
char *sxformat(char *res,char *fmt,va_list arglist)
    {
    int pf_count = 0;
    _allowfrees = true;
    if( res )
      then
	pf( fmt, arglist, &pf_count, res, NULL );
      else
	{
	res = malloc( BIGBUFSIZE );
	pf( fmt, arglist, &pf_count, NULL, NULL );
	res = realloc( res, pf_count+1 );
	}
    return res;
    }

/**************************************************************************/
/***	Write to the specified array.					***/
/**************************************************************************/
char *sformat(char *res,char *fmt,...)
    {
    XIFY(ap,fmt,char *ret=sxformat(res,fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write the data to stdout					***/
/**************************************************************************/
int xformat(char *fmt,va_list arglist)
    {
    _allowfrees = TRUE;
    return pf( fmt, arglist, NULL, NULL, stdout );
    }

/**************************************************************************/
/***	Write the data to stdout					***/
/**************************************************************************/
int format(char *fmt,...)
    {
    XIFY(ap,fmt,int ret=xformat(fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write the data to stdout and flush				***/
/**************************************************************************/
int flxformat(char *fmt,va_list arglist)
    {
    _allowfrees = TRUE;
    int res = pf( fmt, arglist, NULL, NULL, stdout );
    ioflush( stdout );
    return res;
    }

/**************************************************************************/
/***	Write the data to stdout and flush				***/
/**************************************************************************/
int flformat(char *fmt,...)
    {
    XIFY(ap,fmt,int ret=flxformat(fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write the data to stdio channel					***/
/**************************************************************************/
int fxformat(IOFILE outfile,char *fmt,va_list arglist)
    {
    _allowfrees = TRUE;
    return pf( fmt, arglist, NULL, NULL, outfile );
    }


/**************************************************************************/
/***	Write the data to stdio channel					***/
/**************************************************************************/
int fformat(IOFILE outfile,char *fmt,...)
    {
    XIFY(ap,fmt,int ret=fxformat(outfile,fmt,ap),return ret);
    }

/**************************************************************************/
/***	Write the data to stdio channel	and flush			***/
/**************************************************************************/
int fflxformat(IOFILE outfile,char *fmt,va_list arglist)
    {
    _allowfrees = TRUE;
    int res = pf( fmt, arglist, NULL, NULL, outfile );
    ioflush(outfile);
    return res;
    }

/**************************************************************************/
/***	Write the data to stdio channel and flush			***/
/**************************************************************************/
int fflformat(IOFILE outfile,char *fmt,...)
    {
    XIFY(ap,fmt,int ret=fflxformat(outfile,fmt,ap),return ret);
    }

#ifdef HAS_STRERROR
#define STRERROR(x)	strerror(x)
#else
extern char		*sys_errlist[];
#define STRERROR(x)	sys_errlist[x]
#endif

#ifndef USE_NEW_ERRNO
#else
extern int errno;
extern int sys_nerr;
#endif

/**************************************************************************/
/***	Write a specified error message with the previous error code.	***/
/**************************************************************************/
subroutine filerr( char *fmt, ... )
    {
    va_list ap;
    va_start( ap, fmt );
    fxformat( stderr, fmt, ap );
    va_end( ap );
#ifdef USE_NEW_ERRNO
    fformat( stderr, ":  {s}.\n", STRERROR(errno) );
#else
    if( errno <= sys_nerr )
      then fformat( stderr, ":  {s}.\n", STRERROR(errno) );
      else fformat( stderr, ":  Error #{i}.\n", errno );
#endif
    exit(1);
    }

/**************************************************************************/
/***	Not much to setup.						***/
/**************************************************************************/
subroutine setup_format()
    {
    }
