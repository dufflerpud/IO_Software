#ifndef LOCAL_mem_INCLUDED
#define LOCAL_mem_INCLUDED
#include <local.h>

#if HANDLER_mem == SYSTEM_HANDLER
#include <stdlib.h>
#else
extern function char *malloc( unsigned nbytes );
extern function char *calloc( nelem, elemsize );
extern function char *realloc( char *p1, unsigned newsize );
extern subroutine free( void * freep );
#endif

#ifdef HAS_MEMSET
#define setbytes(d,v,sz)	memset(d,v,sz)
#define movebytes(d,s,sz)	memcpy(d,s,sz)
#else
extern subroutine setbytes( char *s, int value, int size );
extern subroutine movebytes( char *s1, char *s2, int size );
#define memset(d,v,sz)		setbytes(d,v,sz)
#define memcpy(d,s,sz)		movebytes(d,s,sz)
#endif

extern subroutine initmalloc( void * baseaddr, unsigned availmemory );
extern function unsigned memfree( );
extern function int16 checksum( char *buf, int size );
extern function int16 swapbytes( int16 arg );
extern subroutine swab( char *sfrom, char *sto, int size );
extern subroutine arrayinit( void **var, int malinc, int objsize );
extern subroutine arrayexist( void **var, int elem );
extern function int arraynext( void **var );
extern subroutine arraydone( void **var );
extern function int arraylen( void **var );
extern subroutine arraysetlen( void **var, int elem );
extern function int bitsin( char *s, int slen, int stype );
extern subroutine setup_mem();
#endif /* LOCAL_mem_INCLUDED */
