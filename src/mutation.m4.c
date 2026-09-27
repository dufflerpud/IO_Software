#include <sys/types.h>
#include <signal.h>

#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <local.h>

#include <local_format.h>
#include <local_settty.h>
#include <local_mem.h>
#include <local_string.h>

#ifndef CTL
#define CTL(x)		((x)&0x1f)
#endif

#define AVAILFOODS	7

#define ITEMWIDTH	3
/* #define FIELDROWS	50 */
/* #define FIELDCOLS	25 */
#define FIELDROWS	50
#define FIELDCOLS	50
#define STACKSIZE	10
#define BYTESIZE	256
#define MAXBUF		30
#define VERCODE		0x5a5a5a5a
#define BCODE		0x55

#define D_MAP		0
#define D_SECTOR	1
#define D_INFO		2

/* #define DEBUG */
#define CHECK
#define ALLOWMUTATIONS





struct instype
    {
    char		*i_name;
    int8		i_prob;
    int8		i_timer;
    } inslist[] =
    {
    	
		
		
		{ "Push-i", 5, 1	 }
		,
    	
		
		
		{ "Push-r", 10, 1	 }
		,
    	
		
		
		{ "Push-s", 15, 1	 }
		,
    	
		
		
		{ "Push-at", 20, 1	 }
		,
    	
		
		
		{ "Pop", 25, 1	 }
		,
    	
		
		
		{ "Pop-at", 30, 1	 }
		,
    	
		
		
		{ "Eat", 35, 3	 }
		,
    	
		
		
		{ "Divide", 40, 10	 }
		,
    	
		
		
		{ "Branch", 45, 1	 }
		,
    	
		
		
		{ "Fbranch", 50, 1	 }
		,
    	
		
		
		{ "Tbranch", 55, 1	 }
		,
    	
		
		
		{ "Move", 60, 7	 }
		,
    	
		
		
		{ "Add", 65, 1	 }
		,
    	
		
		
		{ "Sub", 70, 1	 }
		,
    	
		
		
		{ "Mul", 75, 1	 }
		,
    	
		
		
		{ "Div", 80, 1	 }
		,
    	
		
		
		{ "Mod", 85, 1	 }
		,
    	
		
		
		{ "And", 90, 1	 }
		,
    	
		
		
		{ "Or", 95, 1	 }
		,
    	
		
		
		{ "Xor", 100, 1	 }
		,
    	
		
		
		{ "Neg", 105, 1	 }
		,
    	
		
		
		{ "Not", 110, 1	 }
		,
    	
		
		
		{ "Comp", 115, 1	 }
		,
    	
		
		
		{ "Cmp=", 120, 1	 }
		,
    	
		
		
		{ "Cmp<>", 125, 1	 }
		,
    	
		
		
		{ "Cmp<", 130, 1	 }
		,
    	
		
		
		{ "Cmp>", 135, 1	 }
		,
    	
		
		
		{ "Cmp<=", 140, 1	 }
		,
    	
		
		
		{ "Cmp>=", 145, 1	 }
		,
    	
		
		
		{ NULL, 145, 1	 }
		
    };

#define SIZEPROG	3
char standardprog[SIZEPROG] =
    {
    6 ,
    11 ,
    7 
    };

#define CELL(p)	(*(p))
#define NOCELL	NULL
#define PLACE(x,y)	univ.u_field[x][y]
#define PROG(a,pc)	(CELL(a).a_prog[pc])
#define MPROG(a,pc)	(CELL(a).a_prog[pc] & 0xff)

#define STOPONMUTATIONS	((int32)(-1))
#define FOREVER		((int32)(-2))

typedef struct scell	*cellptr;

/**************************************************************************/
/***									***/
/**************************************************************************/

struct scell
    {
    int32		a_verify;
    int32		a_id;
    cellptr		a_next, a_prev;
    int32		a_x, a_y;
    int32		a_food;
    int32		a_stack[ STACKSIZE ];
    int32		a_mutation;
    int32		a_pc;
    int32		a_size;
    int8		a_sp, a_timer;	/* must change when STACKSIZE>255 */
    int8		a_prog[1];
    };

struct place
    {
    int32		p_food;
    cellptr		p_cell;
    };

struct universe
    {
    int32		u_lastmutation;
    int32		u_generation;
    int32		u_numcells;
    int32		u_alloc;
    struct place	u_field[ FIELDROWS ][ FIELDCOLS ];
    } univ;

cellptr		firstcell = NOCELL;
int			usescurses = TRUE;
int32			autosave = 1000;
int32			autodisplay = 1;
int32			paranoid = FALSE;
int			display = D_MAP;
int32			curcol = FIELDCOLS/2;
int32			currow = FIELDROWS/2;
int8			insmapping[256];

#define MAXDIR		8
int xdir[MAXDIR] = { -1, 00, 01, -1, 01, -1, 00, 01 };
int ydir[MAXDIR] = { -1, -1, -1, 00, 00, 01, 01, 01 };

char muthappenned;
char *filename;

int seed;
int32 maxitempower;

/**************************************************************************/
subroutine tstp()	{}
/**************************************************************************/

int stopreq = FALSE;
/**************************************************************************/
subroutine gotasig( int s )
/**************************************************************************/
    {
    signal( SIGINT, gotasig );
    stopreq = TRUE;
    }

/**************************************************************************/
/**************************************************************************/
extern subroutine dodisplay();
extern subroutine saveworld();

/**************************************************************************/
subroutine fatal( char * msg )
/**************************************************************************/
    {
    if( usescurses )
      then
	{
	move( FIELDROWS+1, 0 );
	clrtoeol();
	refresh();
	endwin();
	resetkluge();
	}
    fformat( stderr, "Fatal error:  {s}.\n",msg);
    free( msg );
    exit(1);
    }

#ifdef CHECK
/**************************************************************************/
subroutine check( char *errmsg )
/**************************************************************************/
    {
    cellptr cell;
    int32 x, y;

    for( cell=firstcell; cell!=NOCELL; cell=CELL(cell).a_next )
	if( CELL(cell).a_verify != VERCODE				||
	    CELL(cell).a_prog[ CELL(cell).a_size ] != BCODE	)
	  then fatal(mformat("Id={lz6} perverted, {s}", CELL(cell).a_id, errmsg));
    for( x=0; x<FIELDROWS; x++ )
	for( y=0; y<FIELDCOLS; y++ )
	    if( PLACE(x,y).p_food < 0 || PLACE(x,y).p_food > 100000 )
	      then fatal(mformat("{l},{l} perverted, {s}", x, y, errmsg));
	    else if( (cell=PLACE(x,y).p_cell) != NOCELL )
	      then
		if( CELL(cell).a_x != x || CELL(cell).a_y != y )
		  then fatal(mformat("Mismatched id={l}, x={l}, y={l}, {s}",
		    CELL(cell).a_id, x, y, errmsg));
    free( errmsg );
    }
#endif

/**************************************************************************/
subroutine wstring( char *buf )
/**************************************************************************/
    {
    for( int i=0; buf[i]; i++ ) addch( buf[i] );
    free( buf );
    }

/**************************************************************************/
function int getbuf( char *buf, int buflen )
/**************************************************************************/
    {
    int i, c;
    int startrow, startcol;

    i = 0;
    getyx( stdscr, startrow, startcol );
    while( TRUE )
	{
	buf[i] = 0;
	move( startrow, startcol );
	wstring(mformat("{sl}",buf,buflen-1));
	move( startrow, startcol+i );
	refresh();
	c = getch();
	if( c>=' ' && c<='~' && i<buflen-1 )
	  then buf[i++] = c;
	  else
	    switch( c )
		{
		case '\r':
		case '\n':	move( startrow, startcol );
				for( c=0; c<i; c++ ) addch(' ');
				refresh();
				return i;

		case '\b':	if( i > 0 ) then i--;
				break;

		case CTL('u'):	i = 0;
				break;
		}
	}
    }

/**************************************************************************/
#ifdef STDARGS_PROMPT
function int prompt( int row, int col, char *buf, int buflen, char *pformat, int pargs )
#else
function int prompt( int row, int col, char *buf, int buflen, char *pformat )
#endif
/**************************************************************************/
    {
    char *cp;
    int cplen, res;

    move( row, col );

#ifdef STDARGS_PROMPT
    cp = mxformat( pformat, &pargs );
    cplen = strlen(cp);
    wstring(mformat( "{sf1}", cp ) );
#else
    wstring(mformat( "{s}", pformat ) );
#endif
    res = getbuf( buf, buflen );
    move( row, col );
    while( cplen-- > 0 ) addch(' ');
    move( row, col );
    refresh();
    return res;
    }

#ifndef _B31
#define _B31 (1<<31)
#endif
/**************************************************************************/
function int32 smod( int32 num, int32 denom )
/**************************************************************************/
    {
    num &= ~_B31;
    if( denom > 0 )
      then return num % denom;
    else if( denom < 0 )
      then return denom - (num % (-denom));
      else return num;
    }

/**************************************************************************/
function int32 push(cellptr cell,int32 value)
/**************************************************************************/
    {
    CELL(cell).a_sp = (CELL(cell).a_sp + 1) % STACKSIZE;
    CELL(cell).a_stack[ CELL(cell).a_sp ] = value;
    return value;
    }

/**************************************************************************/
function int32 pop(cellptr cell)
/**************************************************************************/
    {
    int sp;
    int32 ret;
    ret = CELL(cell).a_stack[ CELL(cell).a_sp ];
    if( CELL(cell).a_sp-- == 0 ) then CELL(cell).a_sp = STACKSIZE-1;
    return ret;
    }

/**************************************************************************/
function int choose( int choices )
/**************************************************************************/
    {
    int res;
    res = ((rand() >> 3) & 0x0fff);
    seed++;
    return res % choices;
    }

/**************************************************************************/
function int getatpc( cellptr cell )
/**************************************************************************/
    {
    int val;
    val = MPROG( cell, CELL(cell).a_pc++ );
    if( paranoid && CELL(cell).a_size <= 0 )
      then fatal(mformat("Zero id = {lz6}", CELL(cell).a_id));
    CELL(cell).a_pc %= CELL(cell).a_size;
    return val;
    }

/**************************************************************************/
subroutine killcell( cellptr cell, char *reason )
/**************************************************************************/
    {
    int32 x, y;
    int i;

#ifdef DEBUG
    flformat("{lz6} dies ({s}).\n",CELL(cell).a_id,reason);
#endif

    if( CELL(cell).a_prev == NOCELL )
      then firstcell = CELL(cell).a_next;
      else CELL(CELL(cell).a_prev).a_next = CELL(cell).a_next;
    if( CELL(cell).a_next != NOCELL )
      then CELL(CELL(cell).a_next).a_prev = CELL(cell).a_prev;
    x = CELL(cell).a_x;
    y = CELL(cell).a_y;
    PLACE(x,y).p_food += 2*CELL(cell).a_food/3;
    PLACE(x,y).p_cell = NOCELL;
    univ.u_alloc = univ.u_alloc-sizeof(struct scell)-CELL(cell).a_size;
    if( paranoid )
      then
	if( CELL(cell).a_verify != VERCODE				||
	    CELL(cell).a_prog[ CELL(cell).a_size ] != BCODE	)
	  then fatal(mformat("Attempting to free {lz6} (Verify code={l}, reason={s})\n",
	    CELL(cell).a_id, CELL(cell).a_verify, reason));
    free( cell );
    univ.u_numcells--;
    }

/**************************************************************************/
function cellptr addcell( int32 size )
/**************************************************************************/
    {
    cellptr newcell;
    static int32 nextcellid = 0;
    int32 blocksize;
    int i;

    blocksize = sizeof(struct scell) + size;
    if( (newcell = (cellptr)malloc( (int)blocksize )) == NOCELL )
      then return NOCELL;
    CELL(newcell).a_verify = VERCODE;
    CELL(newcell).a_prog[size] = BCODE;
    univ.u_alloc += blocksize;
    univ.u_numcells++;
    CELL(newcell).a_food = 0;
    for( i=0; i<STACKSIZE; i++ ) CELL(newcell).a_stack[i] = 0;
    CELL(newcell).a_sp = 0;
    CELL(newcell).a_pc = 0;
    CELL(newcell).a_timer = 1;
    CELL(newcell).a_size = size;
    if( firstcell != NOCELL ) then CELL(firstcell).a_prev = newcell;
    CELL(newcell).a_next = firstcell;
    CELL(newcell).a_prev = NOCELL;
    firstcell = newcell;
    CELL(newcell).a_id = nextcellid++;
    return newcell;
    }

/**************************************************************************/
subroutine i_eat( cellptr cell )
/**************************************************************************/
    {
    int32 x, y;
    int i;

    x = CELL(cell).a_x;
    y = CELL(cell).a_y;
    CELL(cell).a_food += PLACE(x,y).p_food;
    PLACE(x,y).p_food = 0;
#ifdef DEBUG
    flformat("{lz6} eats {l}=>{l}.\n",
	CELL(cell).a_id,PLACE(x,y).p_food,CELL(cell).a_food);
#endif
    }

/**************************************************************************/
function cellptr i_divide( cellptr cell )
/**************************************************************************/
    {
    int oldseed;
    cellptr newcell;
    int i, delta;
    int32 pc1, pc2, oldpc, newpc;
    int32 muttype;
    int32 x, y;
    int dir;
    int foundmutation = FALSE;

    dir = smod( pop(cell), (int32)MAXDIR );
    x = (FIELDROWS + CELL( cell ).a_x + xdir[dir]) % FIELDROWS;
    y = (FIELDCOLS + CELL( cell ).a_y + ydir[dir]) % FIELDCOLS;
    if( PLACE( x, y ).p_cell != NOCELL )
      then
	{

#ifdef DEBUG
	flformat("{lz6} failed to spawn.\n",CELL(cell).a_id);
#endif

	return NOCELL;
	}


#ifndef ALLOWMUTATIONS
    pc2 = CELL(cell).a_size;
#else
    oldseed = seed;
    srand( oldseed );

    for( pc1=pc2=0; pc1<CELL(cell).a_size; )
	if( (muttype = choose( 10000 )) < 4 && pc1>0 )
	  then
	    {
	    pc1 += choose( 50 );
	    if( pc1 >= CELL(cell).a_size ) then pc1 = CELL(cell).a_size;
	    }
	else if( muttype < 8 )
	  then
	    {
	    pc1++;
	    pc2++;
	    delta = choose( 50 ) + 1;
	    pc2 += delta;
	    while( delta-- ) choose( BYTESIZE );
	    }
	  else
	    {
	    if( muttype < 40 ) then choose( BYTESIZE );
	    pc1++;
	    pc2++;
	    }
#endif

    if( (newcell = addcell( pc2 )) == NOCELL ) then return NOCELL;

#ifndef ALLOWMUTATIONS
    newpc = CELL(cell).a_pc;
    for( pc1=pc2=0; pc1<CELL(cell).a_size; )
	PROG(newcell,pc2++) = PROG(cell,pc1++);
#else
    newpc = oldpc = CELL(cell).a_pc;
    srand( oldseed );
    for( pc1=pc2=0; pc1<CELL(cell).a_size; )
	{
	if( pc1 <= oldpc ) then newpc = oldpc;
	if( (muttype = choose( 10000 )) < 4 && pc1>0 )
	  then
	    {
#ifdef DEBUG
	    flformat("Clipping.\n");
#endif
	    foundmutation = TRUE;
	    pc1 += choose( 50 );
	    if( pc1 >= CELL(cell).a_size ) then pc1 = CELL(cell).a_size;
	    }
	else if( muttype < 8 )
	  then
	    {
#ifdef DEBUG
	    flformat("Adding.\n");
#endif
	    foundmutation = TRUE;
	    PROG(newcell,pc2++) = PROG(cell,pc1++);
	    delta = choose( 50 ) + 1;
	    while( delta-- ) PROG(newcell,pc2++) = choose( BYTESIZE );
	    }
	else if( muttype < 40 )
	  then
	    {
#ifdef DEBUG
	    flformat("Switching.\n");
#endif
	    foundmutation = TRUE;
	    PROG(newcell,pc2++) = choose( BYTESIZE );
	    pc1++;
	    }
	  else PROG(newcell,pc2++) = PROG(cell,pc1++);
	}
#endif

#ifdef DEBUG
    flformat("{lz6} spawns {lz6}.\n",
	CELL(cell).a_id,CELL(newcell).a_id);
#endif

    CELL(cell).a_food /= 2;
    CELL(newcell).a_food = CELL(cell).a_food;
    for( i=0; i<STACKSIZE; i++ )
	CELL(newcell).a_stack[i] = CELL(cell).a_stack[i];
    CELL(newcell).a_sp = CELL(cell).a_sp;
    CELL(newcell).a_pc = newpc;
    CELL(newcell).a_timer = CELL(cell).a_timer;
    CELL(newcell).a_x = x;
    CELL(newcell).a_y = y;
    PLACE( x, y ).p_cell = newcell;
    if(!foundmutation)
      then CELL(newcell).a_mutation = CELL(cell).a_mutation;
      else
	{
	CELL(newcell).a_mutation = ++univ.u_lastmutation;
	muthappenned = TRUE;
	}
    return newcell;
    }

/**************************************************************************/
subroutine i_pushr( cellptr cell )
/**************************************************************************/
    {
    int32 newac;
    newac = choose(BYTESIZE);
    newac |= (choose(BYTESIZE)<<8);
    newac |= (choose(BYTESIZE)<<16);
    newac |= (choose(BYTESIZE)<<24);
    push( cell, newac );
    }

/**************************************************************************/
subroutine i_pushi( cellptr cell )
/**************************************************************************/
    {
    int32 newac;
    newac = getatpc( cell );
    newac |= (getatpc(cell)<<8);
    newac |= (getatpc(cell)<<16);
    newac |= (getatpc(cell)<<24);
    push( cell, newac );
    }

/**************************************************************************/
subroutine i_branch( cellptr cell )
/**************************************************************************/
    {
    int32 offset;
    CELL(cell).a_pc += pop( cell );
    CELL(cell).a_pc &= ~_B31;
    CELL(cell).a_pc %= CELL(cell).a_size;
    }

/**************************************************************************/
subroutine i_move( cellptr cell )
/**************************************************************************/
    {
    int32 x, y;
    int dir;

    dir = smod( pop(cell), (int32)MAXDIR );
    x = (FIELDROWS + CELL( cell ).a_x + xdir[dir]) % FIELDROWS;
    y = (FIELDCOLS + CELL( cell ).a_y + ydir[dir]) % FIELDCOLS;
    if( PLACE(x,y).p_cell == NOCELL )
      then
	{

#ifdef DEBUG
	flformat("{lz6} moves to {l},{l}.\n", CELL(cell).a_id, x, y );
#endif

	PLACE( CELL(cell).a_x, CELL(cell).a_y ).p_cell = NOCELL;
	PLACE( x, y ).p_cell = cell;
	CELL( cell ).a_x = x;
	CELL( cell ).a_y = y;
	}
      else
	{

#ifdef DEBUG
	flformat("{lz6} attacks {lz6} at {l},{l}.\n",
	    CELL(cell).a_id, CELL(PLACE(x,y).p_cell).a_id, x, y );
#endif

	killcell( PLACE(x,y).p_cell, "Attacked" );
	PLACE( CELL(cell).a_x, CELL(cell).a_y ).p_cell = NOCELL;
	PLACE(x,y).p_cell = cell;
	CELL(cell).a_x = x;
	CELL(cell).a_y = y;
	}
    }

/**************************************************************************/
subroutine exins( cellptr cell, int ins )
/**************************************************************************/
    {
    int32 n1, n2;
    switch( ins )
	{
	case 0 :	i_pushi( cell );			break;
	case 1 :	i_pushr( cell );			break;
	case 2 :	push( cell, push(cell, pop(cell) ) );
			break;
	case 3 :	n1 = smod( pop( cell ), (int32)STACKSIZE );
			push( cell, CELL(cell).a_stack[n1] );
			break;
	case 4 :	pop(cell);				break;
	case 5 :	n1 = smod( pop( cell ), (int32)STACKSIZE );
			CELL(cell).a_stack[n1] = pop(cell);
			break;
	case 6 :	i_eat( cell );			break;
	case 7 :	i_divide( cell );			break;
	case 8 :	i_branch( cell );			break;
	case 10 :	if( pop(cell) )
			  then i_branch( cell );
			  else pop(cell);
			break;
	case 9 :	if( !pop(cell) )
			  then i_branch( cell );
			  else pop(cell);
			break;
	case 11 :	i_move( cell );			break;
	case 12 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1+n2);
			break;
	case 13 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1-n2);
			break;
	case 14 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1*n2);
			break;
	case 15 :	n1=pop(cell);
			n2=pop(cell);
			if( n2 != 0 )
			  then push(cell,n1/n2);
			  else push(cell,0);
			break;
	case 16 :	n1=pop(cell);
			n2=pop(cell);
			if( n2 != 0 )
			  then push(cell,n1%n2);
			  else push(cell,0);
			break;
	case 17 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1&n2);
			break;
	case 18 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1|n2);
			break;
	case 19 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,n1^n2);
			break;
	case 20 :	push(cell, -pop(cell) );
			break;
	case 21 :	push(cell, (int32)(!pop(cell)) );
			break;
	case 22 :	push(cell, ~pop(cell) );
			break;
	case 23 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1==n2));
			break;
	case 24 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1!=n2));
			break;
	case 25 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1<n2));
			break;
	case 26 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1>n2));
			break;
	case 27 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1<=n2));
			break;
	case 28 :	n1=pop(cell);
			n2=pop(cell);
			push(cell,(int32)(n1>=n2));
			break;
	default:	fatal(mformat("{lz6} can't execute {i}:{s}",
			    CELL(cell).a_id,ins,
			    inslist[ins].i_name));
	}
    }

/**************************************************************************/
function int cyclecell( cellptr cell )
/**************************************************************************/
    {
    int32 n1, n2;
    int ins;

    static int32 lastid = 0;
    static int lastins = 0;

    if( paranoid )
      then
	if( CELL(cell).a_x < 0		||
	    CELL(cell).a_x >= FIELDROWS	||
	    CELL(cell).a_y < 0		||
	    CELL(cell).a_y >= FIELDCOLS	)
	  then fatal(mformat("Animal {lz6} out of bounds, x={l}, y={l}",
	    CELL(cell).a_id, CELL(cell).a_x, CELL(cell).a_y));
    ins = insmapping[ getatpc( cell ) ] & 0xff;
#ifdef CHECK
    if( paranoid )
      then check(mformat( "Verify failure, Id={l}, ins={i}:{s}, Lid={l}. Lins={i}:{s}",
	CELL(cell).a_id, ins, inslist[ins].i_name,
	lastid, lastins, inslist[lastins].i_name ) );
    lastid = CELL(cell).a_id;
    lastins = ins;
#endif
    if( CELL(cell).a_food <= 0 )
      then killcell( cell, "Starvation" );
      else
	{
	CELL(cell).a_timer = inslist[ins].i_timer;
	CELL(cell).a_food--;
	exins( cell, ins );
	}
    }

/**************************************************************************/
subroutine makeresources()
/**************************************************************************/
    {
    int32 x, y;
    int i;
    for( x=0; x<FIELDROWS; x++ )
	for( y=0; y<FIELDCOLS; y++ )
	    {
	    PLACE(x,y).p_food = AVAILFOODS;
	    PLACE(x,y).p_cell = NOCELL;
	    }
    }

/**************************************************************************/
subroutine addresources()
/**************************************************************************/
    {
    int32 x, y;
    int i;
    for( x=0; x<FIELDROWS; x++ )
	for( y=0; y<FIELDCOLS; y++ )
	    /*
	    if( choose( 100 ) < 5 )
	      then PLACE(x,y).p_food += AVAILFOODS;
	    */
	    PLACE(x,y).p_food++;
    }

/**************************************************************************/
subroutine makecells()
/**************************************************************************/
    {
    int32 x, y;
    int32 i;
    int j;
    cellptr cell;
    for( i=0; i<SIZEPROG; i++ )
	{
	cell = addcell( (int32)SIZEPROG );
#ifdef DEBUG
	flformat("{lz6} created.\n",CELL(cell).a_id);
#endif
	CELL(cell).a_food = AVAILFOODS;
	movebytes( CELL(cell).a_prog, standardprog, (int)SIZEPROG );
	CELL(cell).a_pc = i;
	do  {
	    x = choose( FIELDROWS );
	    y = choose( FIELDCOLS );
	    } while( PLACE(x,y).p_cell != NOCELL );
	PLACE(x,y).p_cell = cell;
	CELL(cell).a_x = x;
	CELL(cell).a_y = y;
	CELL(cell).a_mutation = univ.u_lastmutation;
	}
    }

/**************************************************************************/
function char *genins( cellptr cell, int32 inspc )
/**************************************************************************/
    {
    int val, ins;
    int32 newac;

    val = MPROG(cell,inspc);
    ins = insmapping[ val ] & 0xff;
    switch( ins )
	{
	case 0 :
	    newac =  (MPROG(cell,(inspc+1)%CELL(cell).a_size)    );
	    newac |= (MPROG(cell,(inspc+2)%CELL(cell).a_size)<<8 );
	    newac |= (MPROG(cell,(inspc+3)%CELL(cell).a_size)<<16);
	    newac |= (MPROG(cell,(inspc+4)%CELL(cell).a_size)<<24);
	    return mformat("({iz2b16u1})  {s} {lz8b16u1}",
		val,inslist[ins].i_name,newac);

	default:
	    return mformat("({iz2b16u1})  {s}",val,inslist[ins].i_name);
	}
    }

/**************************************************************************/
subroutine mapdisplay()
/**************************************************************************/
    {
    int32 row, col;
    int32 bestv, leastv, mutv, count;
    int i;
    cellptr cell, bestp;

    if( usescurses )
      then
	{
	erase();
	move( 0, 0 );
	}
    wstring(mformat("Generation {l}, Mutation {lb36}, Allocated {l}:\n",
	univ.u_generation,univ.u_lastmutation,univ.u_alloc) );
    for( row=0; row<FIELDROWS; row++ )
	{
	for( col=0; col<FIELDCOLS; col++ )
	    if( PLACE(row,col).p_cell == NOCELL )
	      then wstring(mformat(" {cn}",'-',ITEMWIDTH-1));
	      else wstring(mformat(" {lb36z}",
		CELL(PLACE(row,col).p_cell).a_mutation%maxitempower,
		ITEMWIDTH-1));
	wstring(mformat("\n"));
	}
    
    if( !usescurses )
      then
	{
#ifdef PLACEFOODS
	wstring(mformat("\nFood suplies:\n"));
	for( row=0; row<FIELDROWS; row++ )
	    {
	    for( col=0; col<FIELDCOLS; col++ )
		wstring(mformat("{lr}",PLACE(row,col).p_food,ITEMWIDTH));
	    wstring(mformat("\n"));
	    }
	wstring(mformat("\n"));
#endif
#ifdef CELLFOODS
	wstring(mformat("\nAnimal foods:\n"));
	for( row=0; row<FIELDROWS; row++ )
	    {
	    for( col=0; col<FIELDCOLS; col++ )
		wstring(mformat("{lr}",
		    CELL(PLACE(row,col).p_cell).a_food,ITEMWIDTH));
	    wstring(mformat("\n"));
	    }
	wstring(mformat("\n"));
#endif
	wstring(mformat("\nMutations:\n"));
	leastv = -1;
	while( TRUE )
	    {
	    bestv = ~_B31;
	    count = 0;
	    for(cell=firstcell;cell!=NULL;cell=CELL(cell).a_next)
		{
		mutv = CELL(cell).a_mutation;
		if( mutv == bestv )
		  then count++;
		else if( mutv < bestv && mutv > leastv )
		  then
		    {
		    bestv = CELL(cell).a_mutation;
		    bestp = cell;
		    count = 1;
		    }
		}
	    leastv = bestv;
	    if( count <= 0 ) then break;
	    wstring(mformat("\nMutation {lb36}, {l} occurances:\n",bestv,count));
	    for( i=0; i<CELL(bestp).a_size; i++ )
		wstring(mformat("{iz4b16}:  {sf1}\n", i, genins( bestp, (int32)i ) ) );
	    }
	}
    }

/**************************************************************************/
subroutine evolve( int32 numgenerations )
/**************************************************************************/
    {
    cellptr cell;
    int32 gencounter;

    stopreq=FALSE;
    gencounter = numgenerations;
    muthappenned = FALSE;
    while( !stopreq && firstcell!=NOCELL )
        {
	if( autosave>0 && univ.u_generation%autosave == 0 ) then saveworld();
	if( autodisplay>0 && univ.u_generation%autodisplay == 0 )
	  then
	    {
	    dodisplay();
	    if( usescurses ) then refresh();
	    }
	addresources();
	for( cell=firstcell; cell!=NULL; cell=CELL(cell).a_next )
	    if( --CELL(cell).a_timer == 0 )
	      then cyclecell( cell );
	univ.u_generation++;
	if( numgenerations > 0 && --gencounter <= 0 ) then break;
	if( numgenerations == STOPONMUTATIONS && muthappenned ) then break;
	}
#ifdef DEBUG
    if( firstcell == NOCELL )
      then flformat("All cells are dead.\n");
      else flformat("Something went wrong.\n");
#endif
    }

/**************************************************************************/
subroutine sectordisplay()
/**************************************************************************/
    {
    int i;
    cellptr cell;

    erase();
    move( 0, 0 );
    wstring(mformat("({l},{l}) has {l} food",
	currow,curcol,PLACE(currow,curcol).p_food));
    cell = PLACE(currow,curcol).p_cell;
    if( cell == NOCELL ) then return;
    wstring(mformat(" and cell {lz6}, mutation {lz2b36} with {l} food, timer {i}:\n",
	CELL(cell).a_id, CELL(cell).a_mutation,
	CELL(cell).a_food, CELL(cell).a_timer ) );
    
    move( 2, 0 );	wstring(mformat("Program:"));
    for( i=0; i<CELL(cell).a_size; i++ )
	{
	move( 3+i, 0 );
	wstring(mformat("{c}{iz4b16}:  {sf1}\n", (i==CELL(cell).a_pc?'>':' '),
	    i, genins( cell, (int32)i ) ) );
	}
    move( 2, 40 );	wstring(mformat("Stack/Memory:"));
    for( i=0; i<STACKSIZE; i++ )
	{
	move( 3+i, 40 );
	wstring(mformat("{c}{iz4b16}:  {lz8b16u1}", (i==CELL(cell).a_sp?'>':' '),
	    i,CELL(cell).a_stack[i]));
	}
    }

/**************************************************************************/
subroutine infodisplay()
/**************************************************************************/
    {
    erase();
    move( 0, 0 );
    wstring(mformat("Autodisplay = {l}.\n",autodisplay));
    wstring(mformat("Autosave    = {l}.\n",autosave));
    wstring(mformat("Paranoid    = {i}.\n",paranoid));
    }

/**************************************************************************/
subroutine dodisplay()
/**************************************************************************/
    {
    switch( display )
	{
	case D_MAP:	mapdisplay();		break;
	case D_SECTOR:	sectordisplay();	break;
	case D_INFO:	infodisplay();		break;
	default:	fatal(mformat("Attempting to display {i}",display));
	}
    }

/**************************************************************************/
subroutine makeworld()
/**************************************************************************/
    {
    int infile;
    int32 i;
    int32 size;
    cellptr cell;

    while( firstcell!=NOCELL ) killcell( firstcell, "God's orders" );
    univ.u_lastmutation = 0;
    univ.u_generation = 0;
    univ.u_numcells = 0;
    univ.u_alloc = 0;

    if( (infile = open(filename,0)) < 0 )
      then
	{
	makeresources();
	makecells();
	}
      else
	{
	if(read(infile,&univ,sizeof(univ))<sizeof(univ))
	  then fatal(mformat("Found EOF reading field in {s}",filename));
	for( i=0; i<univ.u_numcells; i++ )
	    {
	    if( read(infile,&size,sizeof(size)) < sizeof(size) )
	      then fatal(mformat("Found EOF reading a size from {s}",filename));
	    cell = (cellptr)malloc( (int)size );
	    if( cell == NOCELL )
	      then fatal(mformat("Couldn't allocate memory for new cell"));
	    if( read(infile,cell,(int)size) < size )
	      then fatal(mformat("Found EOF reading an cell from {s}",filename));
	    CELL(cell).a_next = firstcell;
	    if( firstcell != NOCELL )
	      then CELL(firstcell).a_prev = cell;
	    firstcell = cell;
	    CELL(cell).a_prev = NOCELL;
	    PLACE( CELL(cell).a_x, CELL(cell).a_y ).p_cell = cell;
	    }
	close( infile );
	}
    }

/**************************************************************************/
subroutine saveworld()
/**************************************************************************/
    {
    int outfile;
    int32 size, i;
    cellptr cell;

#ifdef CHECK
    check(mformat("Pre saveworld"));
#endif
    if( usescurses )
      then
	{
	move( FIELDROWS+1, 0 );
	wstring(mformat("[Saving database]"));
	clrtoeol();
	refresh();
	}
    if( (outfile=creat(filename,0666)) < 0 )
      then fatal(mformat("Failed to create {s}",filename));
    write( outfile, &univ, sizeof(univ) );
    for( cell=firstcell; cell!=NOCELL; cell=CELL(cell).a_next )
	{
	size = sizeof(struct scell) + CELL(cell).a_size;
	write( outfile, &size, sizeof(size) );
	write( outfile, cell, (int)size );
	}
    close( outfile );
    if( usescurses )
      then
	{
	move( FIELDROWS+1, 0 );
	clrtoeol();
	refresh();
	}
    }

/**************************************************************************/
subroutine docommands()
/**************************************************************************/
    {
    int c = 0;
    int32 atemp;
    char buf[MAXBUF];
    int32 gentodo;

    while( TRUE )
	{
	if( display!=D_MAP || strchr("hjklHJKL",c)==NULL ) then dodisplay();
	if( usescurses )
	  then
	    {
	    move( FIELDROWS+1, 0 );
	    wstring(mformat("Enter command:  "));
	    if(display==D_MAP)
	      then move((int)currow+1,(int)curcol*ITEMWIDTH+1);
	    refresh();
	    c = getch();
	    move( FIELDROWS+1, 0 );
	    clrtoeol();
	    refresh();
	    }
	switch( c )
	    {
	    case 'q':	return;

	    case 'd':	prompt( FIELDROWS+1, 0, buf, MAXBUF,
			    "Display (m,s,i):  ");
			switch( buf[0] )
			    {
			    case 'm':	display = D_MAP;	break;
			    case 's':	display = D_SECTOR;	break;
			    case 'i':	display = D_INFO;	break;
			    }
			break;

	    case 'D':	prompt(FIELDROWS+1,0,buf,MAXBUF,
			    "Autodisplay interval:  ");
			if( sscanf(buf,"%ld",&atemp)==1 && atemp>=0 )
			  then autodisplay = atemp;
			break;

	    case 's':	saveworld();					return;

	    case 'S':	prompt(FIELDROWS+1,0,buf,MAXBUF,
			    "Autosave interval:  ");
			if( sscanf(buf,"%ld",&atemp)==1 && atemp>=0 )
			  then autosave = atemp;
			break;

	    case 'h':	curcol = (curcol+FIELDCOLS-1) % FIELDCOLS;	break;
	    case 'j':	currow = (currow+1) % FIELDROWS;		break;
	    case 'k':	currow = (currow+FIELDROWS-1) % FIELDROWS;	break;
	    case 'l':	curcol = (curcol+1) % FIELDCOLS;		break;

	    case 'H':	curcol = 0;					break;
	    case 'J':	currow = FIELDROWS-1;				break;
	    case 'K':	currow = 0;					break;
	    case 'L':	curcol = FIELDCOLS-1;				break;

	    case '\r':
	    case '\n':	evolve( (int32)1 );				break;
	    case 'g':	evolve( FOREVER );				break;
	    case 'm':	evolve( STOPONMUTATIONS );			break;
	    case 'n':	prompt(FIELDROWS+1,0,buf,MAXBUF,
			    "Generations to evolve:  ");
			if( sscanf(buf,"%ld",&gentodo)==1 && gentodo>0 )
			  then evolve( gentodo );
			break;

	    case 'r':	makeworld();				break;

	    case 'P':	paranoid = !paranoid;			break;
	    }
	}
    }

/**************************************************************************/
subroutine main( int argc, char *argv[] )
/**************************************************************************/
    {
    time_t curtime = 0;
    int i, j;
    int32 whentostop = 0;
    int autoexit = FALSE;
    int usereport = FALSE;

    filename = mformat( "{s}.save", argv[0] );

    for( i=1; i<argc; i++ )
	if( strcmp( argv[i], "-e" ) == 0 )
	  then autoexit = TRUE;
	else if( strcmp( argv[i], "-g" ) == 0 )
	  then whentostop = FOREVER;
	else if( strcmp( argv[i], "-m" ) == 0 )
	  then whentostop = STOPONMUTATIONS;
	else if(strncmp(argv[i],"-n",2)==0 )
	  then
	    {
	    if( sscanf(argv[i]+2,"%ld",&whentostop)!=1	||
		whentostop <= 0				)
	      then
		{
		fflformat(stderr,"-n requires positive integer\n");
		exit(1);
		}
	    }
	else if( strcmp(argv[i], "-P" ) == 0 )
	  then paranoid = TRUE;
	else if(strncmp(argv[i],"-S",2)==0 )
	  then
	    {
	    if( sscanf(argv[i]+2,"%ld",&autosave)!=1	||
		autosave < 0				)
	      then
		{
		fflformat(stderr,"-S requires non-negative integer\n");
		exit(1);
		}
	    }
	else if(strncmp(argv[i],"-R",2)==0 )
	  then
	    if( sscanf(argv[i]+2,"%ld",&autodisplay)==1	&&
		autodisplay >= 0			)
	      then usereport = TRUE;
	      else
		{
		fflformat(stderr,"-R requires non-negative integer\n");
		exit(1);
		}
	else if(strncmp(argv[i],"-D",2)==0 )
	  then
	    {
	    if( sscanf(argv[i]+2,"%ld",&autodisplay)==1	&&
		autodisplay >= 0			)
	      then usereport = FALSE;
	      else
		{
		fflformat(stderr,"-D requires non-negative integer\n");
		exit(1);
		}
	    }
	  else filename = argv[i];

    if( autoexit ) then usescurses = !usereport;

    for( maxitempower=i=1; i<ITEMWIDTH; i++ ) maxitempower *= 36;
    for( i=j=0; i<29 ; i++ )
	while( j < 256*inslist[i].i_prob/145 )
	    insmapping[j++] = i;
    for( i=0; i<SIZEPROG; i++ )
	standardprog[i] = 256*(inslist[standardprog[i]&0xff].i_prob-1)/145;

    signal( SIGINT, gotasig );
    /* time( &curtime ); */
    seed = (int)curtime;
    srand( seed );

    if( usescurses )
      then
	{
	initscr();
	noecho();
	setkluge();
	}

    makeworld();
    if( whentostop != 0 ) then evolve( whentostop );
    if( autoexit )
      then saveworld();
      else docommands();

    if( usescurses )
      then
	{
	resetkluge();
	endwin();
	}
    exit(0);
    }
