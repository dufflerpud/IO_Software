/************************************************************************
 *
 *indx#	hashtable.c - Software for creating and maintaining a hash table
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
 *doc#	Software for creating and maintaining a hash table
 ************************************************************************/
#include <local_format.h>
#include <local_mem.h>
#include <local_string.h>
#include <local_iosubs.h>

#define HASHSIZE	1001

typedef struct hashitem *hashptr;

struct hashitem
    {
    hashptr		h_next;
    char		h_name[1];
    };

hashptr hashtable[HASHSIZE];

#define HNULL		((hashptr)0)

subroutine inithash()
    {
    int i;
    for( i=0; i<HASHSIZE; i++ ) hashtable[i] = HNULL;
    }

function int hashnum( char *cp )
    {
    int32 v = 0;
    int i;

    for( i=0; cp[i]; i++ ) v ^= (cp[i]<<((i%4)*7));
    v %= HASHSIZE;
    return v;
    }

function hashptr addtotable( char *cp )
    {
    hashptr hp;

    hp = (hashptr)(&hashtable[ hashnum(cp) ]);
    while( hp->h_next != HNULL )
	if( strcmp( hp->h_next->h_name, cp ) == 0 )
	  then return hp->h_next;
	  else hp = hp -> h_next;

    hp->h_next = (hashptr)malloc( sizeof(struct hashitem) + strlen(cp) );
    hp->h_next->h_next = HNULL;
    strcpy( hp->h_next->h_name, cp );
    return hp->h_next;
    }

int maxwid = 0;

subroutine dumptable()
    {
    int i;
    hashptr hp;
    int curpos = 0;

    for( i=0; i<HASHSIZE; i++ )
	for( hp=hashtable[i]; hp!=HNULL; hp=hp->h_next )
	    {
	    if( curpos == 0 )
	      then format("extern int "), curpos+=11;
	      else format(","), curpos++;
	    format("{Sr}","ot_{s}()",maxwid,hp->h_name);
	    curpos += maxwid;
	    if( curpos > 77-maxwid )
	      then
		{
		format(";\n");
		curpos = 0;
		}
	    }
    if( curpos > 0 ) then format(";\n");
    format("\n");
    }
