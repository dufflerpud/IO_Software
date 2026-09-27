#ifndef LOCAL_time_INCLUDED
#define LOCAL_time_INCLUDED
#include <local.h>

#if HANDLER_time == SYSTEM_HANDLER
#include <sys/types.h>
#include <time.h>
#define setup_time()

#elif HANDLER_time == LOCAL_HANDLER
extern subroutine tzset( );
extern function struct tm *gmtime( time_t *nclock );
extern function struct tm *localtime( time_t *clock );
extern function time_t gmtotime( struct tm *t );
extern function time_t ltotime( struct tm *t );
extern function struct tm *parsetime( char *s );
extern function char *asctime( struct tm *t );
extern function char *ctime( time_t *ti );
extern function time_t gtime( char *s );
extern subroutine setup_time();
#endif
#endif
