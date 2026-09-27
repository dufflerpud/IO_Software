/************************************************************************
 *
 *indx#	errors.c - Software to track a stack of errors
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
 *doc#	Software to track a stack of errors
 ************************************************************************/
#include <local_format.h>
#include <local_iosubs.h>
#include <local_error.h>

#ifdef GROWINGSTACK
static char *stack;
#else
static char stack[100];
#endif

static char *errmesg;
static int _founderror;
static int sptr;

/**************************************************************************/
/***	Decide from the top of the stack whether the message is an	***/
/***	error not.							***/
/**************************************************************************/
int exformat( char *fmt, va_list arglist );
    {
    if( errmesg != NULL ) then free( errmesg );
    errmesg = mxformat( fmt, arglist );
    switch( stack[sptr] )
	{
	case ERR_PASS:							break;
	case ERR_SAY:	fflformat(stderr,"\r\n{s}\r\n",errmesg);	break;
	case ERR_TRAP:	fflformat(stderr,"\r\nFatal:\r\n{s}\r\n",errmesg);
			exit(1);
	}
    _founderror = TRUE;
    return FALSE;
    }

/**************************************************************************/
/***	Init the stack of errors.					***/
/**************************************************************************/
int eformat( char *fmt, ... )
    {
    XIFY(ap,fmt,int ret=exformat(fmt,ap),return ret);
    }

/**************************************************************************/
/***	Initialize the error stack.					***/
/**************************************************************************/
subroutine initerrors()
    {
#ifdef GROWINGSTACK
    arrayinit( &stack, sizeof(char), 10 );
#endif
    sptr = 0;
    errmesg = NULL;
    seterrors( ERR_TRAP );
    }

/**************************************************************************/
/***	Set error at the top of stack.					***/
/**************************************************************************/
subroutine seterrors( int errtype )
    {
    _founderror = TRUE;
#ifdef GROWINGSTACK
    arrayexist( &stack, sptr );
#endif
    stack[sptr] = errtype;
    }

/**************************************************************************/
/***	Put an error on the stack.					***/
/**************************************************************************/
subroutine pusherrors( int errtype )
    {
    sptr++;
    seterrors( errtype );
    }

/**************************************************************************/
/***	Return true if the top of the stack is an error.		***/
/**************************************************************************/
function int founderror()
    {
    if( sptr > 0 ) then sptr--;
    if( _founderror )
      then
	{
	_founderror = FALSE;
	return TRUE;
	}
    return FALSE;
    }
