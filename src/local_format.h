#ifndef LOCAL_FORMAT_DEFINED
#define LOCAL_FORMAT_DEFINED
#include <local.h>
#include <local_iosubs.h>

extern function int cformat( char *fmt, ... );
extern function int cxformat( char *fmt, va_list arglist );
extern function char *mformat( char *fmt, ... );
extern function int fdformat( int fd, char *fmt, ... );
extern function char *mxformat( char *fmt, va_list arglist );
extern function int fdxformat( int fd, char *fmt, va_list arglist );
extern function char *sformat( char *res, char *fmt, ... );
extern function char *sxformat( char *res, char *fmt, va_list arglist );
extern function int format( char *fmt, ... );
extern function int flformat( char *fmt, ... );
extern function int xformat( char *fmt, va_list arglist );
extern function int flxformat( char *fmt, va_list arglist );
extern function int fformat( IOFILE outfile, char *fmt, ... );
extern function int fflformat( IOFILE outfile, char *fmt, ... );
extern function int fxformat( IOFILE outfile, char *fmt, va_list arglist );
extern function int fflxformat( IOFILE outfile, char *fmt, va_list arglist );
extern subroutine filerr( char *fmt, ... );
extern subroutine setup_format();
#endif
