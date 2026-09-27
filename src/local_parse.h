#ifndef LOCAL_parse_INCLUDED
#define LOCAL_parse_INCLUDED
#include <local.h>

extern function char *gline( IOFILE fp );
extern function int sparse( char *res, char *fmt, ... );
extern function int sxparse( char *res, char *fmt, va_list arglist );
extern function int parse( char *fmt, ... );
extern function int xparse( char *fmt, va_list arglist );
extern function int fparse( IOFILE outfile, char *fmt, ... );
extern function int fxparse( IOFILE outfile, char *fmt, va_list arglist );
extern function int gnc( );
extern subroutine setup_parse();
#endif
