/************************************************************************
 *
 *indx#	mem.c - Software for memory allocation
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
 *doc#	Software for memory allocation
 ************************************************************************/
/***************************************************************************
	mem.c:  Memory handling routines
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#ifndef STANDALONE
#include <stdlib.h>
subroutine initmalloc()	{}
#else

#include "local_iosubs.h"

struct allocstruct
    {
    struct allocstruct	*a_next;
    unsigned		a_size;
    };

typedef struct allocstruct astr;
typedef astr *astrp;

static astr header;
static int memused, chunksused;

/************************************************************************/
/************************************************************************/
subroutine initmalloc( astrp baseaddr, unsigned availmemory )
    {
    astrp	p;

    p = baseaddr;
    header.a_next = p;
    header.a_size = 0;
    p->a_next = NULL;
    p->a_size = availmemory;
    memused = chunksused = 0;
    }

/************************************************************************/
/************************************************************************/
function unsigned memfree()
    {
    astrp p;
    unsigned result = 0;

    for( p=header.a_next; p!=NULL; p=p->a_next )
	result += p->a_size;
    return result;
    }

/************************************************************************/
/************************************************************************/
function char *malloc( unsigned nbytes )
    {
    astrp p1, p2;
    char *retval;

    nbytes = nbytes + sizeof(long)-1 - (nbytes-1)%sizeof(long) + sizeof(astr);

    retval = NULL;
    for( p1 = &header,p2=header.a_next; p2!=NULL; p1=p2,p2=p2->a_next )
	if(p2->a_size>=nbytes) then break;
    if( p2 != NULL )
      then
	{
	if( p2->a_size < nbytes+sizeof(astr) )
	  then
	    {
	    p1->a_next = p2->a_next;
	    retval = (char *)(p2+1);
	    memused += p1->a_size;
	    }
	  else
	    {
	    p1->a_next = (astrp)((char *)p2+nbytes);
	    p1 = p1->a_next;
	    p1->a_next = p2->a_next;
	    p1->a_size = p2->a_size - nbytes;
	    p2->a_next = p1;
	    p2->a_size = nbytes;
	    memused += p2->a_size;
	    retval = (char *)(p2+1);
	    }
	chunksused++;
	}
    return retval;
    }

/************************************************************************/
/************************************************************************/
function char *calloc( nelem, elemsize ) int nelem, int elemsize )
    {
    char *retval;
    int i;

    nelem *= elemsize;
    retval = malloc( nelem );
    if( retval != NULL ) then for( i=0; i<nelem; i++ ) retval[i] = 0;
    return retval;
    }

/************************************************************************/
/************************************************************************/
function char *realloc( char *p1, unsigned newsize )
    {
    char *p2, *retval;
    int oldsize, i;

    if( p1 == NULL )
      then retval = malloc( newsize );
      else
	{
	oldsize = ((astrp)p1-1)->a_size - sizeof(astr);
	if( newsize <= oldsize )
	  then retval = p1;
	  else
	    {
	    p2 = malloc( newsize );
	    if( p2 != NULL ) then for( i=0; i<oldsize; i++ ) p2[i] = p1[i];
	    free( p1 );
	    retval = p2;
	    }
	}
    return retval;
    }

/************************************************************************/
/************************************************************************/
subroutine free( astrp freep )
    {
    astrp	pfreep, indp, pindp, prevp, postp;
    int		inserted;

    freep--;
    chunksused--;
    memused -= freep->a_size;
    postp = (astrp)((char *)freep+freep->a_size);
    inserted = FALSE;
    for(pindp = &header,indp=header.a_next;
	indp!=NULL;
	pindp=indp,indp=indp->a_next )
	if( (astrp)((char *)indp+indp->a_size) == freep )
	  then
	    if( !inserted )
	      then
		{
		prevp = indp;
		memused -= indp->a_size;
		indp->a_size += freep->a_size;
		inserted = TRUE;
		}
	      else
		{
		memused -= indp->a_size;
		indp->a_size += freep->a_size;
		pfreep->a_next = freep->a_next;
		break;
		}
	else if( indp==postp )
	  then
	    if( !inserted )
	      then
		{
		memused -= indp->a_size;
		pfreep = pindp;
		pindp->a_next = freep;
		freep->a_next = indp->a_next;
		freep->a_size += indp->a_size;
		inserted = TRUE;
		}
	      else
		{
		memused -= prevp->a_size;
		prevp->a_size += indp->a_size;
		pindp->a_next = indp->a_next;
		break;
		}
    if( !inserted )
      then
	{
	freep->a_next = NULL;
	pindp->a_next = freep;
	}
    }
#endif	/* Standalone */

/************************************************************************/
/************************************************************************/
subroutine setbytes( char *s, int value, int size )
    {
    while( size-- > 0 ) *s++ = value;
    }

/************************************************************************/
/************************************************************************/
subroutine movebytes( char *s1, char *s2, int size )
    {
    while( size-- > 0 ) *s1++ = *s2++;
    }

/************************************************************************/
/************************************************************************/
function int16 checksum( char *buf, int size )
    {
    int16 temp;
    int16 result = 0;
    while( size-- > 0 )
	{
	temp = result;
	*((char *)&result+0) = *((char *)&temp+1);
	*((char *)&result+1) = *((char *)&temp+0);
	result = result + *buf++;
	}
    return result;
    }

/************************************************************************/
/************************************************************************/
function int16 swapbytes( int16 arg )
    {
    int16 result;
    *((int8*)&result+0) = *((int8*)&arg+1);
    *((int8*)&result+1) = *((int8*)&arg+0);
    return result;
    }

/************************************************************************/
/************************************************************************/
subroutine swab(char *sfrom,char *sto,int size)
    {
    while( size-- > 0 )
	{
	*sto++ = sfrom[1];
	if( size-- > 0 ) then *sto++ = sfrom[0];
	sfrom += 2;
	}
    }

struct sbuild
    {
    int		slen;
    int		smalinc;
    int		sobjsize;
    int		sused;
    };

/************************************************************************/
/************************************************************************/
subroutine arrayinit( struct sbuild **var, int malinc, int objsize )
    {
    struct sbuild *t;

    (*var) = (struct sbuild *)malloc( sizeof(struct sbuild) );
    (*var) -> slen = 0;
    (*var) -> smalinc = malinc;
    (*var) -> sobjsize = objsize;
    (*var) -> sused = 0;
    (*var)++;
    }

/************************************************************************/
/************************************************************************/
subroutine arrayexist( struct sbuild **var, int elem )
    {
    struct sbuild *t;
    t = *var - 1;
    if( elem > t->sused ) then t->sused = elem;
    if( t -> slen > elem ) then return;
    t -> slen = elem + t->smalinc;
    *var = (struct sbuild *)realloc( (char *)t, t->slen*t->sobjsize );
    }

/************************************************************************/
/************************************************************************/
function int arraynext( struct sbuild **var )
    {
    int ind;
    struct sbuild *t;
    t = *var - 1;
    ind = t -> sused + 1;
    arrayexist( var, ind );
    return ind;
    }

/************************************************************************/
/************************************************************************/
subroutine arraydone( struct sbuild **var )
    {
    char *newstr;
    struct sbuild *t;

    t = *var - 1;
    newstr = malloc( (t->sused+1) * t->sobjsize );
    movebytes( newstr, (char*)*var, (t->sused+1) * t->sobjsize );
    free( t );
    *var = (struct sbuild *)newstr;
    }

/************************************************************************/
/************************************************************************/
function int arraylen( struct sbuild **var )
    {
    struct sbuild *t;

    t = *var - 1;
    return t->sused + 1;
    }

/************************************************************************/
/************************************************************************/
subroutine arraysetlen( struct sbuild **var, int elem )
    {
    struct sbuild *t;

    if( elem >= 0 )
      then
	{
	arrayexist( var, elem );
	t = *var - 1;
	t -> sused = elem;
	}
    }

/************************************************************************/
/************************************************************************/
function int bitsin( char *s, int slen, int stype )
    {
    int i;
    int res = 0;
    switch( stype )
	{
	default:
	case 1:	for(i=0;i<slen;i++) res += ((((int8*)s)[i/8]>>(i%8)) & 1);
		break;

	case 2:	for(i=0;i<slen;i++) res += ((((int16*)s)[i/16]>>(i%16)) & 1);
		break;

	case 4:	for(i=0;i<slen;i++) res += ((((int32*)s)[i/32]>>(i%32)) & 1);
		break;
	}
    return res;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_mem()
    {
    }
