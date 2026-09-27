#ifndef STRERROR
#include <local.h>

#ifdef HAS_STRERROR
#define STRERROR(x)	strerror(x)
#else
extern const char* const sys_errlist[];
#define STRERROR(x)	sys_errlist[x]
#endif

#ifdef USE_NEW_ERRNO
#include <errno.h>
#else
extern int errno;
#endif

#endif /* STRERROR */

#define ERR_TRAP	0
#define ERR_SAY		1
#define ERR_PASS	2

#define traperrors()	seterrors( ERR_TRAP )
#define sayerrors()	seterrors( ERR_SAY )
#define passerrors()	seterrors( ERR_PASS )

extern int eformat( char *fmt, ... );
extern int exformat( char *fmt, va_list ap );
