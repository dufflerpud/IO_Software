#ifndef LOCAL_tempfile_INCLUDED
#define LOCAL_tempfile_INCLUDED
#include <local.h>

extern function char *mktemp( char *template );
extern function char *tmpnam( char *s );
extern subroutine setup_tempfile();
#endif
