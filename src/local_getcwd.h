#ifndef LOCAL_getcwd_INCLUDED
#define LOCAL_getcwd_INCLUDED
#include <local.h>

extern function char *strcwd( );
extern function char *getcwd( char *bufp, size_t size );
extern subroutine setup_getcwd();
#endif
