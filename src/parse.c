/***************************************************************************
	parse.c:  Parsing routines
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>
#include <local_string.h>

#if HANDLER_parse == LOCAL_HANDLER

#include <ctype.h>

#define PF_ARRAY	1
#define PF_GETC		2

char *parseaddr;
static int pafnc;
static IOFILE pafil;

/************************************************************************/
/************************************************************************/
function char *gline( IOFILE fp )
    {
    char buf[1000], *tp0, *tp1;

    while( iogets(fp,buf,4000) > 0 )
	{
	if( buf[0]=='#' ) then continue;
	for( tp0=buf; (tp0=rstrchr(tp0,'\\'))!=NULL; iogets(fp,tp0,1000) )
	    {
	    for( tp1=tp0+1; *tp1=='\t'||*tp1==' '; tp1++ );
	    if( *tp1 != '\n' ) then break;
	    }
	for( tp0=buf; *tp0==' '||*tp0=='\t'; tp0++ );
	if( *tp0 != '\n' ) then return string( buf );
	}
    return NULL;
    }

/************************************************************************/
/************************************************************************/
function int sxparse(char *res,char *fmt,va_list ap)
    {
    parseaddr = res;
    pafnc = PF_ARRAY;
    return gf( ap, fmt );
    }

/************************************************************************/
/************************************************************************/
function int sparse(char *res,char *fmt,...)
    {
    XIFY(ap,fmt,int ret=sxparse(res,fmt,ap),return ret);
    }

/************************************************************************/
/************************************************************************/
int xparse(char *fmt,va_list ap)
    {
    al = (char *)arglist;
    pafnc = PF_GETC;
    pafil = stdout;
    return gf( ap, fmt );
    }

/************************************************************************/
/************************************************************************/
function int parse(char *fmt,...)
    {
    XIFY(ap,fmt,int ret=xparse(fmt,ap),return ret);
    }

/************************************************************************/
/************************************************************************/
int fparse(IOFILE outfile,char *fmt,...)
    {
    XIFY(ap,fmt,int ret=fxparse(outfile,fmt,ap),return ret);
    }

/************************************************************************/
/************************************************************************/
int fxparse(IOFILE outfile,char *fmt,va_list ap)
    {
    pafnc = PF_GETC;
    pafil = outfile;
    return gf( ap, fmt );
    }

/************************************************************************/
/************************************************************************/
static _pbak(char c)
    {
    switch( pafnc )
	{
	case PF_ARRAY:	--parseaddr;		break;
	case PF_GETC:	iobackc(pafil,c);	break;
	}
    }

/************************************************************************/
/************************************************************************/
int gnc()
    {
    switch( pafnc )
	{
	case PF_ARRAY:	return *parseaddr++;
	case PF_GETC:	return ioinc(pafil);
	}
    }

#define pa_str		((char *)(result))
#define pa_mstr		(*(char **)(result))
#define pa_char		(*(char *)(result))
#define pa_float	(*(float *)(result))
#define pa_double	(*(double *)(result))
#define pa_int		(*(int *)(result))
#define pa_long		(*(long *)(result))
#define pa_short	(*(short *)(result))

/************************************************************************/
/************************************************************************/
extern IOFILE printer;
static function int gf( va_list ap, char *fs )
    {
    int matched = 0;
    int pa_size, pa_dec, pa_base, pa_unsigned;
    char *pa_skipover, *pa_allow, *pa_reject;
    char t, buf[512], *result;
    int i, c, cind, sizstr, pbase, nm, negflg, seendig;
    double relval, pow;
    long intval;

    while( c = *fs++ )
	{
	if( c != '{' || (t = *fs++) == '{' )	/* }} */
	  then
	    {
	    if( gnc() != c ) then return matched;
	    }
	  else
	    {
	    pa_size	= 0;
	    pa_dec	= 6;
	    pa_base	= 10;
	    pa_unsigned	= FALSE;
	    pa_skipover	= " \t\r\n";
	    pa_allow	= NULL;
	    pa_reject	= " \t\r\n";
	    switch( t )
		{
		case 's':	result = (char *)NEXT(char *);		break;
		case 'S':	result = (char *)NEXT(char **);		break;
		case 'c':	result = (char *)NEXT(char *);		break;
		case 'i':	result = (char *)NEXT(int *);		break;
		case 'l':	result = (char *)NEXT(long *);		break;
		case 't':	result = (char *)NEXT(short *);		break;
		case 'f':	result = (char *)NEXT(float *);		break;
		case 'd':	result = (char *)NEXT(double *);	break;
		}
							/* { */
	    while( (c = *fs++) != '}' )
		{
		nm = 0;
		pbase = 10;
		if( !isbase(*fs,pbase) )
		  then nm = NEXT(int);
		  else
		    while( TRUE )
			{
			while(isbase(*fs,pbase)) nm = pbase*nm+digtobin(*fs++);
			if( nm<2 || nm>36 || *fs!='_' ) then break;
			pbase = nm;
			fs++;
			}
		switch( c )
		    {
		    case 's':	pa_size = nm;			break;
		    case '.':	pa_dec = nm;			break;
		    case 'b':	pa_base = nm;			break;
		    case 'o':	pa_skipover = (char *)nm;	break;
		    case '-':	pa_reject = (char *)nm;
				pa_allow = NULL;
				break;

		    case '+':	pa_allow = (char *)nm;
				pa_reject = NULL;
				break;
		    }
		}
	    while( (c=gnc()) != IOEOF )
		if( pa_skipover != NULL && strchr(pa_skipover,c)==NULL )
		  then { _pbak(c); break; }
	    switch( t )
		{
		case 's':	i = 0;
				do  {
				    c = gnc();
				    if( c==0 && pafnc==PF_ARRAY )
				      then { _pbak(c); break; }
				    if( pa_reject!=NULL &&
					strchr(pa_reject,c)!=NULL )
				      then { _pbak(c); break; }
				    if( pa_allow!=NULL &&
					strchr(pa_allow,c)==NULL )
				      then { _pbak(c); break; }
				    pa_str[i++] = c;
				    } while( i != pa_size );
				pa_str[i] = 0;
				break;

		case 'S':	sizstr = i = 0;
				pa_mstr = NULL;
				do  {
				    c = gnc();
				    if( c==0 && pafnc==PF_ARRAY )
				      then { _pbak(c); break; }
				    if( pa_reject!=NULL &&
					strchr(pa_reject,c)!=NULL )
				      then { _pbak(c); break; }
				    if( pa_allow!=NULL &&
					strchr(pa_allow,c)==NULL )
				      then { _pbak(c); break; }
				    if( sizstr == i )
				      then
					{
					sizstr = i + 256;
					while( TRUE )
					    {
					    if( pa_mstr==NULL )
					      then pa_mstr=malloc(sizstr);
					      else pa_mstr=realloc(pa_mstr,
						sizstr);
					    if( pa_mstr!=NULL ) then break;
					    sizstr = (sizstr+i)/2;
					    if( sizstr==i ) then break;
					    }
					}
				    if( pa_mstr==NULL ) then break;
				    pa_mstr[i++] = c;
				    } while( i != pa_size );
				if( pa_mstr==NULL ) then return matched;
				if( sizstr == i )
				  then
				    {
				    sizstr++;
				    pa_mstr = realloc( pa_mstr, sizstr );
				    if( pa_mstr==NULL ) then return matched;
				    }
				pa_mstr[i++] = 0;
				if( sizstr > i )
				  then pa_mstr=realloc(pa_mstr,i);
				break;

		case 'c':	pa_char = gnc();
				break;

		case 't':
		case 'i':
		case 'l':	intval = 0;
				seendig = negflg = FALSE;
				i = 0;
				do  {
				    c = gnc();
				    if( c == '-' )
				      then
					if( seendig || negflg )
					  then { _pbak(c); return matched; }
					  else negflg=TRUE;
				    else if( c == '_' )
				      then
					if( intval<2 && intval>36 )
					  then { _pbak(c); return matched; }
					  else
					    {
					    pa_base = intval;
					    intval = 0;
					    seendig = FALSE;
					    }
				    else if( isbase(c,pa_base) )
				      then
					{
					intval *= pa_base;
					c = digtobin(c);
					intval += (negflg?-c:c);
					}
				      else
					{
					_pbak(c);
					if( i==0 ) then return matched;
					break;
					}
				    pa_size--;
				    i++;
				    } while( pa_size != 0 );
				if( t == 'i' )
				  then pa_int = intval;
				else if( t == 'l' )
				  then pa_long = intval;
				  else pa_short = intval;
				break;

		case 'd':
		case 'f':	relval = 0.0;
				pow = 1.0;
				negflg = FALSE;
				i = 0;
				do  {
				    c = gnc();
				    if( c == '-' )
				      then
					if( negflg )
					  then { _pbak(c); break; }
					  else negflg = TRUE;
				    else if( c == '.' )
				      then
					if( pow >= 1.0 )
					  then pow = 0.1;
					  else { _pbak(c); break; }
				    else if( isdigit(c) )
				      then
					{
					c = digtobin( c );
					if( pow >= 1.0 )
					  then relval =
					    relval*10+(negflg?-c:c);
					  else
					    {
					    relval += pow*(negflg?-c:c);
					    pow *= 0.1;
					    }
					}
				      else
					{
					_pbak(c);
					if( i==0 ) then return matched;
					break;
					}
				    pa_size--;
				    i++;
				    } while( pa_size != 0 );
				if( t == 'd' )
				  then pa_double = relval;
				  else pa_float = relval;
				break;
		}
	    matched++;
	    }
	}
    return matched;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_parse()
    {
    }
#endif
