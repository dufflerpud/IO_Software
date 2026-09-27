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
