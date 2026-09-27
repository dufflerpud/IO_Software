/************************************************************************
 *
 *indx#	capparse.c - Software to parse characteristics of /etc/remote
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
 *doc#	Software to parse characteristics of /etc/remote
 ************************************************************************/
/***************************************************************************
	capparse.c:  Routines to handle terminal dependent characteristics
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <ctype.h>
#include <local_string.h>
#include <stdlib.h>
#include <local_iosubs.h>
#include <local_parse.h>
#include <local_format.h>
#include <local_capparse.h>

#define UNKNOWN			(-1000)

function char *gentry( char *fname, char *name )
    {
    IOFILE fp;
    char *cp, *tp0, *tp1;
    int i, nlen;

    if( (fp=ioopen(fname,"r")) == NULL ) then return NULL;
    nlen = strlen(name);
    for( ; (tp1=cp=(char *)gline(fp))!=NULL; free(cp) )
	while( TRUE )
	    {
	    for( tp0=tp1; isalnum(*tp1); tp1++ );
	    if( tp1-tp0 == nlen )
	      then
		{
		for(i=0;i<nlen;i++) if(tp0[i]!=name[i]) then break;
		if( i==nlen ) then { ioclose(fp); return cp; }
		}
	    if( *tp1 != '|' ) then break;
	    tp1++;
	    }
    ioclose(fp);
    return NULL;
    }

subroutine c_p_clean( struct c_pentry *c_p )
    {
    int numclean, i;

    if( c_p == NULL ) then return;
    numclean = c_p[0].c_value.intvalue;
    for( i=0; i<numclean; i++ )
	{
	free( c_p[i].c_name );
	if( c_p[i].c_ecode == 2 ) then free( c_p[i].c_value.strvalue );
	}
    free( c_p );
    }

function struct c_pentry *c_psearch( struct c_pentry *c_p, char *s )
    {
    int numdatum, i;

    if( c_p != NULL )
      then
	{
	numdatum = c_p[0].c_value.intvalue;
	for( i=0; i<numdatum; i++ )
	    if( strcmp( c_p[i].c_name, s ) == 0 )
	      then return &c_p[i];
	}
    return NULL;
    }

function char *strsearch( struct c_pentry *c_p, char *s, char *def )
    {
    struct c_pentry *tp;

    if( (tp=c_psearch(c_p,s)) == NULL )
      then return def;
      else return tp->c_value.strvalue;
    }

function int intsearch( struct c_pentry *c_p, char *s, int def )
    {
    struct c_pentry *tp;

    if( (tp=c_psearch(c_p,s)) == NULL )
      then return def;
      else return tp->c_value.intvalue;
    }

function struct c_pentry *parseentry( char *s )
    {
    int i, j, k;
    int len;
    struct c_pentry *c_pdata = NULL;
    int cd;
    char *tp, *xxxp;

    c_pdata = (struct c_pentry *)malloc( sizeof(struct c_pentry) );
    c_pdata[0].c_name = (char *)string("NumDatum");
    c_pdata[0].c_ecode = 1;
    c_pdata[0].c_value.intvalue = 1;
    while( *s!=':' && *s!='\n' ) s++;
    if( *s++ != ':' ) then { c_p_clean(c_pdata); return NULL; }
    while( (*s != '\n') && (*s!=0) )
	{
	if( !isalnum(*s) )
	  then while( *s != ':' && *s != '\n' )	s++;
	  else
	    {
	    cd = c_pdata[0].c_value.intvalue++;
	    c_pdata = (struct c_pentry *)realloc(
			(char *)c_pdata,
			sizeof(struct c_pentry)*(cd+1) );
	    c_pdata[cd].c_ecode = 0;
	    for( len=0; isalnum(s[len]); len++ ) ;
	    c_pdata[cd].c_name = (char *)malloc( len+1 );
	    for( i=0; i<len; i++ ) c_pdata[cd].c_name[i] = *s++;
	    c_pdata[cd].c_name[i] = 0;
	    switch( *s++ )
		{
		case '=':	c_pdata[cd].c_ecode = 2;
				for(i=0;
				    s[i-1]=='\\' || s[i-1]=='^' ||
					(s[i]!='\n' && s[i]!=':');
				    i++ )
				    ;
				tp = c_pdata[cd].c_value.strvalue =
				    (char *)malloc(i+1);
				j = 0;
				while( j < i )
				    if( *s == '^' )
				      then s++, j+=2, *tp++ = (*s++ & ~0140);
				    else if( *s != '\\' )
				      then j++, *tp++ = *s++;
				      else
				        {
				        s++,j++;
					if( !isdigit(*s) )
					  then
					    {
					    switch( *s )
						{
						case 'b': *tp++=0010; break;
						case 't': *tp++=0011; break;
						case 'n': *tp++=0012; break;
						case 'f': *tp++=0014; break;
						case 'r': *tp++=0015; break;
						case 'E': *tp++=0033; break;
						case 'd': *tp++=0177; break;
						default:  *tp++= *s;  break;
						}
					    s++,j++;
					    }
					  else
					    {
					    k = 0;
					    if( *s != '0' )
					      then
						for( ; isbase(*s,10); s++,j++)
						    k = 10*k + digtobin(*s);
					    else if( j++,lcase(*++s)!='x' )
					      then
					        for(; isbase(*s,8); s++,j++)
						    k = 010*k + digtobin(*s);
					      else
						for(j++,s++;
						    isbase(*s,16);
						    s++,j++)
						    k = 0x10*k + digtobin(*s);
					    *tp++ = k;
					    }
					}
				*tp = 0;
				break;

		case '#':	if( s[0] != '0' )
				  then
				    for( i=j=0; isbase(s[i],10); i++ )
					j = j*10 + digtobin(s[i]);
				  else
				    if( s[1] != 'x' )
				      then
					for( i=1,j=0; isbase(s[i],010); i++ )
					    j = j*010 + digtobin(s[i]);
				      else
					for( i=2,j=0; isbase(s[i],0x10); i++ )
					    j = j*0x10 + digtobin(s[i]);
				c_pdata[cd].c_ecode = 1;
				c_pdata[cd].c_value.intvalue = j;
				s += i;
				break;

		case '@':	c_pdata[cd].c_ecode = 1;
				c_pdata[cd].c_value.intvalue = 0;
				break;

		case ':':	c_pdata[cd].c_ecode = 1;
				c_pdata[cd].c_value.intvalue = 1;
				s--;
				break;
		}
	    }
	if( *s == ':' )
	  then s++;
	  else
	    {
	    fformat(stderr,"Failed with '{c}' or {i} after {s}.\r\n",
		*s,*s,c_pdata[cd].c_name);
	    c_p_clean(c_pdata);
	    return NULL;
	    }
	}
    return c_pdata;
    }
