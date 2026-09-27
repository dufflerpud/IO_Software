/************************************************************************
 *
 *indx#	local_mem.h - Include file for memory management
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
 *doc#	Include file for memory management
 ************************************************************************/
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
