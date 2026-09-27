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
