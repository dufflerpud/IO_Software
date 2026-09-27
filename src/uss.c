/**************************************************************************/
/***	uss.c (Unix spread sheet)					***/
/***	Author:  Christopher M. Caldwell of IO Software, Inc.		***/
/***	Created 25-Jun-86						***/
/**************************************************************************/

#include <curses.h>
#include <local.h>

#include <local_error.h>
#include <local_iosubs.h>
#include <local_format.h>
#include <local_string.h>
#include <local_mem.h>
#include <local_settty.h>

/* #include <local.h> */

#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define VERSION		1.0
#define MAXROW		99
#define MAXCOL		99
#define DEFWIDTH	10
#define DEFPREC		2
#define CTL(x)		( (x) & 037 )
#define DTOR(x)		( (x)*3.1415926/180.0 )
#define RTOD(x)		( (x)*180.0/3.1415926 )
#define S_EVAL		1
#define S_ERROR		2

struct entry
    {
    char	e_row;
    char	e_col;
    char	*e_text;
    char	e_status;
    char	*e_expr;
    double	e_value;
    long	e_updated;
    };

struct entry *table[MAXROW+1][MAXCOL+1];
char **args;
int nargs;
int colwidth[MAXCOL+1];	int colprec[MAXCOL+1];
int mrow = 0;		int mcol = 0;
int disrow = 0;		int discol = 0;
int currow = 0;		int curcol = 0;
int screenrows = 24;	int screencols = 80;
int changed = TRUE;
long turn = 0;
int iter;
FILE *batch;
char *outexpr;
int repflag;
int recalculate = TRUE;
int showlabels = FALSE;
char **labels;
struct entry **lptrs;
static char combuf[ 100 ], repbuf[ 100 ];
static char *command;
static char tok[ 100 ];
static char putback;
static char *cp;

void tstp()	{}
subroutine trapfpe(int t)	{ signal(SIGFPE,trapfpe); }

#define DNOP	1
#define DNOT	2
#define DNEG	3
#define DABS	4
#define DSIN	5
#define DCOS	6
#define DTAN	7
#define DASN	8
#define DACS	9
#define DATN	10
#define DSIND	11
#define DCOSD	12
#define DASND	13
#define DTAND	14
#define DACSD	15
#define DATND	16
#define DSINH	17
#define DCOSH	18
#define DTANH	19
#define DSINHD	20
#define DCOSHD	21
#define DTANHD	22
#define DDTR	23
#define DRTD	24
#define DLOG	25
#define DL10	26
#define DFAC	27
#define DOR	28
#define DXOR	29
#define DAND	30
#define DEQ	31
#define DNE	32
#define DLT	33
#define DLE	34
#define DGT	35
#define DGE	36
#define DADD	37
#define DSUB	38
#define DMUL	39
#define DDIV	40
#define DMOD	41
#define DEXP	42
#define DRSM	43
#define DRML	44
#define DCNT	45
#define DAVG	46
#define DGMN	47
#define DMIN	48
#define DMAX	49
#define DMED	50
#define DSTD	51

static char *monoplist[] =
    {
	"not", "!",   "-",   "+",   "abs",
	"sin", "cos", "tan", "asn", "acs", "atn",
	"sind","cosd","tand","asnd","acsd","atnd",
	"sinh","cosh","tanh",
	"sinhd","coshd","tanhd",
	"dtor","rtod",
	"log", "log10",
	"fac",
	NULL
    };

static char monops[] =
    {
	DNOT,  DNOT,  DNEG,  DNOP,  DABS,
	DSIN,  DCOS,  DTAN,  DASN,  DACS,  DATN,
	DSIND, DCOSD, DTAND, DASND, DACSD, DATND,
	DSINH, DCOSH, DTANH,
	DSINHD,DCOSHD,DTANHD,
	DDTR,  DRTD,
	DLOG,  DL10,
	DFAC
    };

static char *binoplist[] =
    {
	"or",  "||",  "xor", "and", "&&",
	"=",   "==",  "<>",  "!=",  "<",   "<=",  ">",   ">=",
	"+",   "-",   "*",   "/",   "%",   "mod", "^",   "**",
	NULL
    };

static char binprec[] =
    {
	0,     0,     0,     1,     1,
	2,     2,     2,     2,     2,     2,     2,     2,
	3,     3,     4,     4,     4,     4,     5,     5
    };
#define MAXPREC		5

static char binops[] =
    {
	DOR,   DOR,   DXOR,  DAND,  DAND,
	DEQ,   DEQ,   DNE,   DNE,   DLT,   DLE,   DGT,   DGE,
	DADD,  DSUB,  DMUL,  DDIV,  DMOD,  DMOD,  DEXP,  DEXP
    };

static char *sumoplist[] =
    {
	"sum", "mul", "cnt", "avg", "gmn", "min", "max", "med", "std",
	NULL
    };

static char sumops[] =
    {
	DRSM,  DRML,  DCNT,  DAVG,  DGMN,  DMIN,  DMAX,  DMED,  DSTD
    };

function int streq( char *s1, char *s2 )
    {
    while( *s1 && tolower(*s1)==tolower(*s2) ) s1++,s2++;
    return (*s1==*s2);
    }

function int exmat(char *s,char *slist[])
    {
    int i, j;

    for( i=0; slist[i]!=NULL; i++ )
	if( streq( s, slist[i] ) ) then return i;
    return -1;
    }

subroutine gtok()
    {
    int c;
    int tokind;

    if( !putback )
      then
	{
	tokind = 0;
	while( *cp && (*cp<=' ' || *cp>'~') ) cp++;
	if( isalpha(*cp) || *cp=='_' || (cp[0]=='.' && !isdigit(cp[1])) )
	  then while(isalnum(*cp)||*cp=='_'||*cp=='.') tok[tokind++] = *cp++;
	else if( isdigit(*cp) || *cp=='.' )
	  then while( isdigit(*cp) || *cp=='.' ) tok[tokind++] = *cp++;
	else if( *cp>' ' && *cp<='~' )
	  then
	    {
	    c = tok[tokind++] = *cp++;
	    if( ( c=='<' && *cp=='=' )		||
		( c=='<' && *cp=='>' )		||
		( c=='>' && *cp=='=' )		||
		( c=='!' && *cp=='=' )		||
		( c=='=' && *cp=='=' )		||
		( c=='&' && *cp=='&' )		||
		( c=='|' && *cp=='|' )		||
		( c=='*' && *cp=='*' )		)
	      then tok[tokind++] = *cp++;
	    }
	tok[tokind] = 0;
	}
    putback = FALSE;
    }

extern function int ev( int r, int c, double *value, int prec );
function int ltorc( char *tk, int crow, int ccol, int *rpt, int *cpt )
    {
    int nr=0;	int nc=0;
    int onr=0;	int onc=0;
    char *tp, *s;
    int i, sflag;

    if( tk[0]=='.' )
      then
	{
	if( tk[1]==0 ) then { *rpt=crow; *cpt=ccol; return TRUE; }
	nr=crow; nc=ccol;
	s = tk+1;
	}
      else
	{
	s = tk;
	if( (tp=strchr(s,'.')) != NULL ) then *tp = 0;
	if( (i=orig_abbrev(s,labels)) >= 0 && lptrs[i]!=NULL )
	  then
	    {
	    if( tp==NULL )
	      then
		{
		*rpt=lptrs[i]->e_row;
		*cpt=lptrs[i]->e_col;
		return TRUE;
		}
	    nr = lptrs[i]->e_row;
	    nc = lptrs[i]->e_col;
	    s = tp+1;
	    }
	if( tp!=NULL ) then *tp = '.';
	}
    if( *s++!='r' ) then return FALSE;
    if( *s=='_' )
      then { sflag=TRUE; s++; }
      else sflag=FALSE;
    while(isdigit(*s)) onr = 10*onr + (sflag?-digtobin(*s++):digtobin(*s++));
    if( *s++!='c' ) then return FALSE;
    if( *s=='_' )
      then { sflag=TRUE; s++; }
      else sflag=FALSE;
    while(isdigit(*s)) onc = 10*onc + (sflag?-digtobin(*s++):digtobin(*s++));
    if( *s != 0 ) then return FALSE;
    if( nr+onr < 0 || nr+onr > MAXROW ) then return FALSE;
    if( nc+onc < 0 || nc+onc > MAXCOL ) then return FALSE;
    *rpt = nr + onr;
    *cpt = nc + onc;
    return TRUE;
    }

function int eval( int r, int c, char *s, double *value )
    {
    char *savecp, saveputback;
    int res;
    struct entry *p;

    savecp=cp;			cp=s;
    saveputback=putback;	putback=FALSE;
    if( (p=table[r][c]) != NULL )
      then
	if( p->e_status & S_EVAL )
	  then return FALSE;
	  else p->e_status |= S_EVAL;
    res = ev( r, c, value, 0 );
    if( p != NULL ) then p->e_status &= ~S_EVAL;
    cp=savecp;			putback=saveputback;
    return res;
    }

function int exrange( double *res, int opcode, int r1, int c1, int r2, int c2 )
    {
    int r, c;
    int temp;
    int retval;
    struct entry *p;
    double sumval=0.0;
    double mulval=1.0;
    double minval=0.0;
    double maxval=0.0;
    int count = 0;

    if( opcode==DRML )
      then *res = 1.0;
      else *res = 0.0;
    if( r1 > r2 ) then { temp=r1; r1=r2; r2=temp; }
    if( c1 > c2 ) then { temp=c1; c1=c2; c2=temp; }
    for( r=r1; r<=r2; r++ )
	for( c=c1; c<=c2; c++ )
	    {
	    if( (p=table[r][c])!=NULL && p->e_text==NULL )
	      then
		{
		if( p->e_expr != NULL )
		  then
		    {
		    if( p->e_updated >= turn )
		      then
		        retval = ( ( p->e_status & S_ERROR ) == 0 );
		      else
			if( (retval=eval(r,c,p->e_expr,&p->e_value)) )
			  then p->e_status &= ~S_ERROR;
			  else p->e_status |= S_ERROR;
		    if( !retval ) then return FALSE;
		    }
		sumval += p->e_value;
		mulval *= p->e_value;
		if( count==0 || p->e_value < minval ) then minval = p->e_value;
		if( count==0 || p->e_value > maxval ) then maxval = p->e_value;
		count++;
		p->e_updated = turn;
		}
	    }
    switch( opcode )
	{
	case DRSM:	*res = sumval;			break;
	case DRML:	*res = mulval;			break;
	case DCNT:	*res = count;			break;
	case DMIN:	*res = minval;			break;
	case DMAX:	*res = maxval;			break;
	case DMED:	*res = (minval+maxval)/2.0;	break;
	case DAVG:	if( count <= 0 )
			  then *res=0.0;
			  else *res=sumval/count;
			break;
	case DGMN:	if( count <= 0 )
			  then *res=1.0;
			  else *res=pow(mulval,1.0/count);
			break;
	case DSTD:	*res = 0.0;
			if( count <= 0 ) then break;
			sumval /= count;
			for( r=r1; r<=r2; r++ )
			    for( c=c1; c<=c2; c++ )
				if( (p=table[r][c])!=NULL && p->e_text==NULL )
				  then
				    {
				    mulval = p->e_value - sumval;
				    *res += (mulval*mulval);
				    }
			*res = pow( *res/count, 0.5 );
			break;
	}
    return TRUE;
    }

function int exop( double *res, int opcode, double v1, double v2 )
    {
    switch( opcode )
	{
	case DNOP:	*res = v1;					break;
#ifdef SIGTSTP
	case DNOT:	*res = (v1==0.0);				break;
#endif
	case DNEG:	*res = -v1;					break;
	case DABS:	*res = ( v1 >= 0 ? v1 : -v1 );			break;
	case DSIN:	*res = sin(v1);					break;
	case DCOS:	*res = cos(v1);					break;
	case DTAN:	*res = tan(v1);					break;
	case DASN:	*res = asin(v1);				break;
	case DACS:	*res = acos(v1);				break;
	case DATN:	*res = atan(v1);				break;
	case DSIND:	*res = sin(DTOR(v1));				break;
	case DCOSD:	*res = cos(DTOR(v1));				break;
	case DTAND:	*res = tan(DTOR(v1));				break;
	case DASND:	*res = RTOD(asin(v1));				break;
	case DACSD:	*res = RTOD(acos(v1));				break;
	case DATND:	*res = RTOD(atan(v1));				break;
	case DSINH:	*res = sinh(v1);				break;
	case DCOSH:	*res = cosh(v1);				break;
	case DTANH:	*res = tanh(v1);				break;
	case DSINHD:	*res = sinh(DTOR(v1));				break;
	case DCOSHD:	*res = cosh(DTOR(v1));				break;
	case DTANHD:	*res = tanh(DTOR(v1));				break;
	case DDTR:	*res = DTOR(v1);				break;
	case DRTD:	*res = RTOD(v1);				break;
	case DLOG:	*res = log(v1);					break;
	case DL10:	*res = log10(v1);				break;
	case DFAC:	for( *res=1.0; v1>0.0; v1-=1.0 ) *res*=v1;	break;
#ifdef SIGTSTP
	case DOR:	*res = ((v1!=0.0) | (v2!=0.0));			break;
	case DXOR:	*res = ((v1!=0.0) ^ (v2!=0.0));			break;
	case DAND:	*res = ((v1!=0.0) & (v2!=0.0));			break;
#endif
	case DEQ:	*res = (v1 == v2);				break;
	case DNE:	*res = (v1 != v2);				break;
	case DLT:	*res = (v1 < v2);				break;
	case DLE:	*res = (v1 <= v2);				break;
	case DGT:	*res = (v1 > v2);				break;
	case DGE:	*res = (v1 >= v2);				break;
	case DADD:	*res = (v1 + v2);				break;
	case DSUB:	*res = (v1 - v2);				break;
	case DMUL:	*res = (v1 * v2);				break;
	case DDIV:	*res = (v1 / v2);				break;
	/*
	case DMOD:	*res = fmod(v1,v2);				break;
	*/
	case DMOD:	*res = ((int)v1 % (int)v2);			break;
	case DEXP:	*res = pow(v1,v2);				break;
	}
    return TRUE;
    }

function int ev( int r, int c, double *value, int prec )
    {
    double v1;
    int res, i, j, pcount;
    int r1, c1;
    int r2, c2;
    char *tp;
    FILE *comfile;
    struct entry *p;

    if( prec <= MAXPREC )
      then
	{
	if( !ev( r, c, value, prec+1 ) ) then return FALSE;
	while( TRUE )
	    {
	    gtok();
	    if( !*tok || (i=exmat(tok,binoplist))<0 || binprec[i]!=prec )
	      then break;
	    if( !ev( r, c, &v1, prec+1 ) ) then return FALSE;
	    if( !exop(value,binops[i],*value,v1) ) then return FALSE;
	    }
	putback = TRUE;
	return TRUE;
	}
      else
	{
	gtok();
	if( (i=exmat(tok,monoplist)) >= 0 )
	  then
	    {
	    res = ev( r, c, value, MAXPREC );
	    return res && ( monops[i]==DNOP || exop(value,monops[i],*value,(double)0) );
	    }
	else if( (i=exmat(tok,sumoplist)) >= 0 )
	  then
	    {
	    gtok();
	    if( !ltorc(tok,r,c,&r1,&c1) ) then return FALSE;
	    gtok();
	    if( !ltorc(tok,r,c,&r2,&c2) ) then return FALSE;
	    return exrange(value,sumops[i],r1,c1,r2,c2);
	    }
	else if( streq(tok,"sh") )
	  then
	    {
	    gtok();
	    if( *tok!='(' ) then return FALSE;
	    for( pcount=1,tp=cp; *cp && pcount>0; cp++ )
		if( *cp=='(' ) then pcount++;
		else if( *cp==')' ) then pcount--;
	    if( pcount > 0 ) then return FALSE;
	    *--cp = 0;
	    if( (comfile=popen(tp,"r")) == NULL )
	      then
	        {
		return FALSE;
		/* *cp++ = ')'; */
		}
	    i = fscanf(comfile,"%lf",value);
	    ioclose( comfile );
	    wait(0);
	    *cp++ = ')';
	    return( i >= 1 );
	    }
	else if( *tok=='(' )
	  then
	    {
	    res = ev( r, c, value, 0 );
	    gtok();
	    return res && *tok==')';
	    }
	else if( isalpha(*tok) || *tok=='_' || (tok[0]=='.'&&!isdigit(tok[1])) )
	  then
	    if( !ltorc(tok,r,c,&i,&j) )
	      then return FALSE;
	      else
		{
		if( (p = table[i][j]) == NULL )
		  then return FALSE;
		else if( p->e_expr != NULL )
		  then
		    {
		    if( p->e_updated >= turn )
		      then res = ( (p->e_status & S_ERROR) == 0 );
		      else
			{
			res = eval(i,j,p->e_expr,&p->e_value);
			if( !res )
			  then p->e_status |= S_ERROR;
			  else p->e_status &= ~S_ERROR;
			}
		    *value = p->e_value;
		    p->e_updated = turn;
		    return res;
		    }
		else if( p->e_text == NULL )
		  then
		    {
		    *value = p->e_value;
		    return TRUE;
		    }
		  else return FALSE;
		}
	  else
	    {
	    if( isdigit(*tok) || *tok=='.' )
	      then return ( sscanf(tok,"%lf",value) >= 1 );
	    }
	}
    return FALSE;
    }

subroutine evalall()
    {
    int r, c;
    struct entry *p;

    turn++;
    for( r=0; r<=mrow; r++ )
	for( c=0; c<=mcol; c++ )
	    if( (p=table[r][c])!=NULL && p->e_expr!=NULL )
	      then
		{
		if( eval( r, c, p->e_expr, &p->e_value ) )
		  then p->e_status &= ~S_ERROR;
		  else p->e_status |= S_ERROR;
		p->e_updated = turn;
		}
    changed = TRUE;
    }

function struct entry *uentry()
    {
    struct entry *p;

    if( (p=table[currow][curcol]) == NULL )
      then table[currow][curcol] = p =
		(struct entry *)malloc(sizeof(struct entry));
      else
	if( p->e_text != NULL )
	  then free( p->e_text );
	  else
	    if( p->e_expr != NULL )
	      then free( p->e_expr );

    p->e_row = currow;
    p->e_col = curcol;
    p->e_text = NULL;
    p->e_expr = NULL;
    p->e_value = 0.0;
    p->e_status = 0;
    p->e_updated = 0;
    if( currow > mrow ) then mrow = currow;
    if( curcol > mcol ) then mcol = curcol;
    return p;
    }

#if HANDLER_args == STDARG_HANDLER
subroutine wformat( char *fmt, ... )
#elif HANDLER_args == VARARGS_HANDLER
subroutine wformat( char *fmt, int arglist )
#endif
    {
    char megabuf[1000];
    int i;

#if HANDLER_args == STDARG_HANDLER
    va_list ap;
    va_start( ap, fmt );
    sxformat( megabuf, fmt, ap );
#else
    sxformat( megabuf, fmt, &arglist );
#endif
    for( i=0; megabuf[i]; i++ ) addch( megabuf[i] );
    }

#if HANDLER_args == STDARG_HANDLER
subroutine errmsg(char *fmt,...)
#elif HANDLER_args == VARARGS_HANDLER
subroutine errmsg(char *fmt, int arglist )
#endif
    {
#if HANDLER_args == STDARG_HANDLER
    va_list ap;
    va_start( ap, fmt );
    char *megabuf = mxformat( fmt, ap );
#elif HANDLER_ARGS == VARARGS_HANDLER
    char *megabuf = mxformat( fmt, &arglist );
#endif
    move( 1, 0 );
    wformat("{s}.\n",megabuf);
    free( megabuf );
    if( batch != NULL )
      then
	{
	ioclose( batch );
	batch = NULL;
	}
    }

function int uss_gettext()
    {
    int c, numchars;
    int sc, sr;

    if( batch!=NULL || repflag ) then return FALSE;

    sr = currow - disrow + 2;
    for( c=discol,sc=0; c<curcol; sc+=colwidth[c++] )	;
    wformat("Input text (Type <CR> when done):");
    numchars = 1;
    while( TRUE )
	{
	move( sr, sc );
	command[numchars] = 0;
	wformat("{s}",command+1);
	refresh();
	c = getch();
	if( c>=' ' && c<='~' )
	  then
	    if( sc+numchars < 79 )
	     then command[numchars++] = c;
	     else iooutc(stdout,7);
	  else
	    switch( c )
		{
		case CTL('C'):
		case CTL('D'):	move( 1, 0 );
				clrtoeol();
				return TRUE;

		case CTL('H'):
		case 0177:	if( numchars > 1 )
				  then command[numchars--] = 0;
				  else
				    {
				    move( 1, 0 );
				    clrtoeol();
				    return TRUE;
				    }
				break;

		case CTL('U'):	command[numchars=1] = 0;
				break;

		case CTL('M'):
		case CTL('J'):	move( 1, 0 );
				clrtoeol();
				return FALSE;

		default:	iooutc( stdout, 7 );
				break;
		}
	}
    }

#if HANDLER_args == STDARG_HANDLER
function int prompt(char *fmt,...)
#elif HANDLER_args == VARARGS_HANDLER
function int prompt(char *fmt,int arglist)
#endif
    {
    int c, numchars;

    if( batch!=NULL || repflag ) then return FALSE;

    char promptbuf[1000];

#if HANDLER_args == STDARG_HANDLER
    va_list ap;
    va_start( ap, fmt );
    sxformat( promptbuf, fmt, ap );
#elif HANDLER_args == VARARGS_HANDLER
    sxformat( promptbuf, fmt, &arglist );
#endif

    numchars = 1;
    while( TRUE )
	{
	move( 1, 0 );
	command[numchars] = 0;
	wformat("{s}: {s}",promptbuf,command+1);
	clrtoeol();
	refresh();
	c = getch();
	if( c>=' ' && c<='~' )
	  then
	    if( numchars < 60 )
	     then command[numchars++] = c;
	     else iooutc(stdout,7);
	  else
	    switch( c )
		{
		case CTL('C'):
		case CTL('D'):	move( 1, 0 );
				clrtoeol();
				return TRUE;

		case CTL('H'):
		case 0177:	if( numchars > 1 )
				  then command[numchars--] = 0;
				  else
				    {
				    move( 1, 0 );
				    clrtoeol();
				    return TRUE;
				    }
				break;

		case CTL('U'):	command[numchars=1] = 0;
				break;

		case CTL('M'):
		case CTL('J'):	move( 1, 0 );
				clrtoeol();
				return FALSE;

		default:	iooutc( stdout, 7 );
				break;
		}
	}
    }

subroutine findnewmaxes()
    {
    int r, c;

    mrow = mcol = 0;
    for( r=0; r<=MAXROW; r++ )
	for( c=0; c<=MAXCOL; c++ )
	    if( table[r][c] != NULL )
	      then
		{
		if( r > mrow ) then mrow = r;
		if( c > mcol ) then mcol = c;
		}
    }

subroutine blank( int br, int bc )
    {
    struct entry *p;
    int i;

    if( (p=table[br][bc]) == NULL ) then return;
    if( p->e_expr != NULL ) then free( p->e_expr );
    else if( p->e_text != NULL ) then free( p->e_text );
    for( i=0; labels[i]!=NULL; i++ )
	if( lptrs[i]==p )
	  then
	    {
	    free( labels[i] );
	    labels[i] = string(".");
	    lptrs[i]=NULL;
	    }
    free( p );
    table[br][bc] = NULL;
    findnewmaxes();
    }

function int copyblock( int r1,int c1, int r2,int c2, int r3,int c3 )
    {
    int rfrom, rf, rto, rt, rinc;
    int cfrom, cf, cto, ct, cinc;
    int r4, c4;
    struct entry *n, *p;

    if( r1==r3 && c1==c3 )
      then
	{
	errmsg("Copy would overwrite data");
        return FALSE;
	}
    r4 = r3 + r2 - r1;
    c4 = c3 + c2 - c1;
    if( r3 < 0 || c3 < 0 || r4 > MAXROW || c4 > MAXCOL )
      then
	{
	errmsg("Copy would go off page");
	return FALSE;
	}
    if( r1 <= r3 )
      then {rinc=(-1); rfrom=r2; rto=r4; }
      else {rinc=1; rfrom=r1; rto=r3; }
    if( c1 <= c3 )
      then {cinc=(-1); cfrom=c2; cto=c4; }
      else {cinc=1; cfrom=c1; cto=c3; }
    for( rf=rfrom,rt=rto; rf>=r1 && rf<=r2; rf+=rinc,rt+=rinc )
	for( cf=cfrom,ct=cto; cf>=c1 && cf<=c2; cf+=cinc,ct+=cinc )
	    if( table[rt][ct]!=NULL )
	      then
		{
		errmsg("Copy would overwrite data");
		return FALSE;
		}
    for( rf=rfrom,rt=rto; rf>=r1 && rf<=r2; rf+=rinc,rt+=rinc )
	for( cf=cfrom,ct=cto; cf>=c1 && cf<=c2; cf+=cinc,ct+=cinc )
	    if( (p = table[rf][cf]) != NULL )
	      then
		{
		n = (struct entry *)malloc(sizeof(struct entry));
		table[rt][ct] = n;
		n->e_row = rt;
		n->e_col = ct;
		n->e_status = p->e_status;
		n->e_value = p->e_value;
		n->e_updated = p->e_updated;
		if( p->e_text != NULL )
		  then n->e_text = string( p->e_text );
		  else n->e_text = NULL;
		if( p->e_expr != NULL )
		  then n->e_expr = string( p->e_expr );
		  else n->e_expr = NULL;
		}
    findnewmaxes();
    return TRUE;
    }

function int moveblock( int r1,int c1, int r2,int c2, int r3,int c3 )
    {
    int rfrom, rf, rto, rt, rinc;
    int cfrom, cf, cto, ct, cinc;
    int r4, c4;
    struct entry *n, *p;

    if( r1==r3 && c1==c3 ) then return TRUE;
    r4 = r3 + r2 - r1;
    c4 = c3 + c2 - c1;
    if( r3 < 0 || c3 < 0 || r4 > MAXROW || c4 > MAXCOL )
      then
	{
	errmsg("Move would go off page");
	return FALSE;
	}
    if( r1 <= r3 )
      then {rinc=(-1); rfrom=r2; rto=r4; }
      else {rinc=1; rfrom=r1; rto=r3; }
    if( c1 <= c3 )
      then {cinc=(-1); cfrom=c2; cto=c4; }
      else {cinc=1; cfrom=c1; cto=c3; }
    for( rf=rfrom,rt=rto; rf>=r1 && rf<=r2; rf+=rinc,rt+=rinc )
	for( cf=cfrom,ct=cto; cf>=c1 && cf<=c2; cf+=cinc,ct+=cinc )
	    if( table[rt][ct]!=NULL &&
		( rt<r1 || rt>r2 || ct<c1 || ct>c2 ) )
	      then
		{
		errmsg("Move would overwrite data");
		return FALSE;
		}
    for( rf=rfrom,rt=rto; rf>=r1 && rf<=r2; rf+=rinc,rt+=rinc )
	for( cf=cfrom,ct=cto; cf>=c1 && cf<=c2; cf+=cinc,ct+=cinc )
	    if( (p = table[rf][cf]) != NULL )
	      then
		{
		table[rt][ct] = p;
		p->e_row = rt;
		p->e_col = ct;
		table[rf][cf] = NULL;
		}
    findnewmaxes();
    return TRUE;
    }

subroutine finish()	{ clear(); refresh(); resetkluge(); endwin(); exit(0); }

subroutine saveall( FILE *outfile )
    {
    int r, c, i;
    struct entry *p;

    fformat(outfile,"#{s} saved file.\n",args[0]);
    for( c=0; c<=mcol; c++ )
	{
	fformat(outfile,"@r0c{i}\n",c);
	fformat(outfile,"w{i}\n",colwidth[c]);
	fformat(outfile,"p{i}\n",colprec[c]);
	for( r=0; r<=mrow; r++ )
	    if( (p=table[r][c]) != NULL )
	      then
		{
		if( r>0 )
		  then
		    fformat(outfile,"@r{i}c{i}\n",r,c);
		if( p->e_text != NULL )
		  then fformat(outfile,"\"{s}\n",p->e_text);
		else if( p->e_expr != NULL )
		  then fformat(outfile,"e{s}\n",p->e_expr);
		  else fformat(outfile,"={f.}\n",p->e_value,colprec[c]+3);
		}
	}
    for( i=0; labels[i]!=NULL; i++ )
	{
	fformat(outfile,"@r{i}c{i}\n",lptrs[i]->e_row,lptrs[i]->e_col);
	fformat(outfile,"m{s}\n",labels[i]);
	}
    fformat(outfile,"@r0c0\n");
    ioclose( outfile );
    }

subroutine marklabel()
    {
    int i, j, k;
    char *s;

    if( isdigit(command[1]) )
      then j = 1;
      else
	for(j=1;isalnum(command[j])||command[j]=='_';j++);
    if( command[1]==0 || command[j]!=0 )
      then { errmsg("Illegal character '{c}' in label.", command[j] ); return; }
    if(table[currow][curcol]==NULL) then { errmsg("Nothing to mark"); return; }
    k = -1;
    for( j=0; labels[j]!=NULL; j++ )
	if( strcmp(labels[j],command+1) == 0 )
	  then break;
	  else
	    if( k<0 && lptrs[j]==NULL ) then k=j;
    if(labels[j]!=NULL) then { errmsg("{s} already exists",command+1); return; }
    s = command+1;
    if( *s++=='r' )
      then
	{
	i = 0;
	while(isdigit(*s)) i=10*i+digtobin(*s++);
	if( i >= 0 && i <= MAXROW )
	  then
	    if( *s++=='c' )
	      then
		{
		i = 0;
		while(isdigit(*s)) i=10*i+digtobin(*s++);
		if( i >= 0 && i <= MAXCOL )
		if(*s==0) then { errmsg("Cannot make {s}",command+1); return; }
		}
	}
    if( k >= 0 )
      then
	{
	free( labels[k] );
	j = k;
	}
      else
	{
	labels = (char **)realloc( (char *)labels,
	    (j+2) * sizeof(char *) );
	lptrs = (struct entry **)realloc( (char *)lptrs,
	    (j+1) * sizeof(struct entry *) );
	labels[j+1] = NULL;
	}
    labels[j] = string( command+1 );
    lptrs[j] = table[currow][curcol];
    }

subroutine center()
    {
    int i;
    int curwid;

    if( (disrow = currow - (screenrows-2) / 2) < 0 ) then disrow = 0;
    i = 1;
    for( curwid=colwidth[curcol]; curwid<screencols && i<MAXCOL; i++ )
	{
	if( curcol-i >= 0 )		then curwid += colwidth[curcol-i];
	if( curcol+i <= MAXCOL )	then curwid += colwidth[curcol+i];
	}
    if( (discol = curcol - i + 1) < 0 ) then discol = 0;
    changed = TRUE;
    }

subroutine execcommand()
    {
    int i, j;
    int r, c;
    int r1, c1, r2, c2, r3, c3;
    char *s;
    char ful[100], flr[100], tul[100];
    struct entry *p;
    double newvalue;
    FILE *outfile;

    switch( command[0] )
	{
	case '#':	prompt("Comment");			break;

#ifdef SIGSTOP
	case CTL('Z'):	clear();
			refresh();
			resetkluge();
			kill( getpid(), SIGSTOP );
			setkluge();
			clear();
			changed = TRUE;
			break;
#endif

	case '!':	if( prompt("Shell command") ) then break;
			move( 0, 0 );
			clear();
			refresh();
			resetkluge();
			if( (s = getenv("SHELL")) == NULL ) then s = "/bin/sh";
			if( command[1]==0 )
			  then system( s );
			  else
			    if( fork()==0 )
			      then execl( s, s, "-c", command+1, 0 );
			      else wait(0);
			format("\n[Type return when ready]\n");
			setkluge();
			while( (i=getch())!='\n' && i!='\r' )
			    {
			    iooutc(stdout,7);
			    ioflush(stdout);
			    }
			clear();
			changed = TRUE;
			strcpy(repbuf,command);
			break;

	case CTL('C'):
	case 'Q':	finish();

	case 'H':	curcol = 0;				break;
	case 'G':
	case 'J':	currow = mrow;				break;
	case 'K':	currow = 0;				break;
	case '$':
	case 'L':	curcol = mcol;				break;

	case 'h':
	case CTL('H'):	while( iter-->0 && curcol>0 ) curcol--;
			break;

	case 'j':
	case CTL('J'):	while( iter-->0 && currow<MAXROW ) currow++;
			break;

	case 'k':
	case CTL('K'):	while( iter-->0 && currow>0 ) currow--;
			break;

	case 'l':
	case CTL('L'):	while( iter-->0 && curcol<MAXCOL ) curcol++;
			break;

	case 'c':	center();				break;

	case 'w':	if( prompt("Width") ) then break;
			if( sscanf(command+1,"%d",&i) < 1	||
			    i < 1				||
			    i > 60				)
			  then
			    {
			    errmsg("Illegal width {s}",command+1);
			    break;
			    }
			colwidth[curcol] = i;
			if( colprec[curcol] > i-2 ) then colprec[curcol] = 0;
			changed = TRUE;
			strcpy(repbuf,command);
			break;

	case 'p':	if( prompt("Precision") ) then break;
			if( sscanf(command+1,"%d",&i) < 1	||
			    i < 0				||
			    (i>0 && i>colwidth[curcol]-2)	)
			  then
			    {
			    errmsg("Illegal precision {s}",command+1);
			    break;
			    }
			colprec[curcol] = i;
			changed = TRUE;
			strcpy(repbuf,command);
			break;

	case 'm':	if( prompt("Mark label") ) then break;
			marklabel();
			if( showlabels ) then changed = TRUE;
			break;

	case 'u':	if( prompt("Unmark label") ) then break;
			for( i=0; labels[i]!=NULL; i++ )
			    if( lptrs[i]!=NULL && streq(labels[i],command+1) )
			      then break;
			if( labels[i] )
			  then
			    {
			    free( labels[i] );
			    labels[i] = string(".");
			    lptrs[i] = NULL;
			    }
			  else errmsg("Unknown label {s}",command+1);
			changed = TRUE;
			break;

	case '@':	if( prompt("Where") ) then break;
			if( !ltorc(command+1,currow,curcol,&currow,&curcol) )
			  then errmsg("Unknown label {s}",command+1);
			break;

	case '"':	changed = TRUE;
			if( uss_gettext() ) then break;
			if( command[1] == 0 )
			  then blank(currow,curcol);
			  else
			    {
			    s = command+1;
			    i = strlen(s);
			    c = curcol;
			    for( i=strlen(s); i>0; i-=c1 )
				{
				if( (c1=i) > colwidth[curcol] )
				  then c1 = colwidth[curcol];
				p = uentry();
				p->e_text = malloc( c1+1 );
				for( j=0; j<c1; j++ ) p->e_text[j] = *s++;
				p->e_text[j] = 0;
				if( ++curcol > MAXCOL ) then break;
				}
			    curcol = c;
			    }
			strcpy(repbuf,command);
			break;

	case '=':	if( prompt("Value") ) then break;
			if( command[1] == 0 )
			  then blank(currow,curcol);
			  else
			    if( !eval( currow, curcol, command+1, &newvalue ) )
			      then errmsg("Illegal expression '{s}'",command+1);
			      else
				{
				p = uentry();
				p->e_value = newvalue;
				}
			changed = TRUE;
			strcpy(repbuf,command);
			break;

	case 't':	if( prompt("Expression") ) then break;
			if( eval( currow, curcol, command+1, &newvalue ) )
			  then errmsg("Value:  {f}",newvalue);
			  else errmsg("Illegal expression '{s}'",command+1);
			break;

	case 'e':	if( prompt("Expression") ) then break;
			if( command[1] == 0 )
			  then blank(currow,curcol);
			  else
			    if( batch==NULL &&
				!eval( currow, curcol, command+1, &newvalue ) )
			      then
				{
				errmsg("Illegal expression '{s}'",command+1);
				break;
				}
			      else
				{
				p = uentry();
				p->e_expr = string( command+1 );
				if( batch==NULL )
				  then p->e_value = newvalue;
				  else p->e_value = 0.0;
				}
			changed = TRUE;
			strcpy(repbuf,command);
			break;

	case 'M':	if( prompt("From-UL from-LR to-UL") ) then break;
			if( sscanf(command+1,"%s%s%s",ful,flr,tul) < 3	||
			    !ltorc(ful,currow,curcol,&r1,&c1)		||
			    !ltorc(flr,currow,curcol,&r2,&c2)		||
			    r1>r2					||
			    c1>c2					||
			    !ltorc(tul,currow,curcol,&r3,&c3)		)
			  then
			    {
			    errmsg("Illegal corner");
			    break;
			    }
			if( moveblock( r1,c1, r2,c2, r3,c3 ) )
			  then
			    {
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case '^':	if( moveblock( currow,0, mrow,mcol, currow-1,0 ) )
			  then
			    {
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case 'V':
	case 'v':	if( moveblock( currow,0, mrow,mcol, currow+1,0 ) )
			  then
			    {
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case '<':	if( moveblock( 0,curcol, mrow,mcol, 0,curcol-1 ) )
			  then
			    {
			    for( c1=curcol; c1<=MAXCOL; c1++ )
				{
				colwidth[c1-1] = colwidth[c1];
				colprec[c1-1] = colprec[c1];
				}
			    colwidth[MAXCOL] = DEFWIDTH;
			    colprec[MAXCOL] = DEFPREC;
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case '>':	if( moveblock( 0,curcol, mrow,mcol, 0,curcol+1 ) )
			  then
			    {
			    for( c1=MAXCOL-1; c1>=curcol; c1-- )
				{
				colwidth[c1+1] = colwidth[c1];
				colprec[c1+1] = colprec[c1];
				}
			    colwidth[curcol] = DEFWIDTH;
			    colprec[curcol] = DEFPREC;
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case 'C':	if( prompt("From-UL from-LR to-UL") ) then break;
			if( sscanf(command+1,"%s%s%s",ful,flr,tul) < 3	||
			    !ltorc(ful,currow,curcol,&r1,&c1)		||
			    !ltorc(flr,currow,curcol,&r2,&c2)		||
			    r1>r2					||
			    c1>c2					||
			    !ltorc(tul,currow,curcol,&r3,&c3)		)
			  then
			    {
			    errmsg("Illegal corner");
			    break;
			    }
			if( copyblock( r1,c1, r2,c2, r3,c3 ) )
			  then
			    {
			    changed = TRUE;
			    strcpy(repbuf,command);
			    }
			break;

	case 'R':	recalculate = !recalculate;
			break;

	case 'r':	evalall();
			break;

	case CTL('R'):	if( !recalculate ) then evalall();
			clear();
			changed = TRUE;
			break;

	case '/':	clear();
			refresh();
			format("Recalculate={i}, Show labels={i} ",
			    recalculate, showlabels );
			format("mrow={i}, mcol={i}.\r\n\n", mrow, mcol );
			for( i=0; labels[i]; i++ )
			    if( lptrs[i] != NULL )
			      then format("{sl20}r{ir2p48}c{ir2p48}\r\n",
				labels[i],lptrs[i]->e_row,lptrs[i]->e_col);
			format("\n[Type any character to continue]");
			ioflush(stdout);
			getch();
			clear();
			changed = TRUE;
			break;

	case '?':	showlabels = !showlabels;
			changed = TRUE;
			break;

	case 'g':	if( prompt("Get from") ) then break;
			if( (batch=ioopen(command+1,"r"))==NULL )
			  then
			    {
			    errmsg("Cannot open {s}: {s}",
			        command+1,STRERROR(errno));
			    break;
			    }
			strcpy(repbuf,command);
			break;

	case 's':	if( prompt("Save to") ) then break;
			if( (outfile=ioopen(command+1,"w")) == NULL )
			  then errmsg("Cannot open {s}: {s}",
			      command+1,STRERROR(errno));
			  else saveall(outfile);
			break;

	case CTL('D'):
	case 'Z':
	case 'S':	if( nargs == 2 )
			  then
			    {
			    if( (outfile=ioopen(args[1],"w")) == NULL )
			      then
				{
			        errmsg("Cannot open {s}: {s}",
				  args[1],STRERROR(errno));
				break;
				}
			    }
			  else
			    {
			    if( prompt("Save to") ) then break;
			    if( (outfile=ioopen(command+1,"w")) == NULL )
			      then
				{
			        errmsg("Cannot open {s}: {s}",
				  command+1,STRERROR(errno));
				break;
				}
			    }
			saveall( outfile );
			finish();

	case 'o':	if( prompt("Output to [file] [UL-corner LR-corner]") )
			  then break;
			r1 = c1 = 0;
			r2 = mrow;	c2 = mcol;
			i = sscanf(command+1,"%s%s%s",tul,ful,flr);
			if( i<=0 ) then strcpy(tul,"");
			else if( i==2 )
			  then
			    {
			    strcpy(flr,ful);
			    strcpy(ful,tul);
			    strcpy(tul,"");
			    i = 3;
			    }
			if( (i>1 && !ltorc(ful,currow,curcol,&r1,&c1))	||
			    (i>2 && !ltorc(flr,currow,curcol,&r2,&c2))	||
			    r1>r2					||
			    c1>c2					)
			  then { errmsg("Illegal argument"); break; }
			if( *tul )
			  then
			    {
			    if( (outfile=ioopen(tul,"w"))==NULL )
			      then
				{
				errmsg("Cannot open {s}: {s}",
				    tul,STRERROR(errno));
				break;
				}
			    }
			  else
			    if( (outfile=popen("/usr/bin/lpr","w"))== NULL )
			      then
				{
				errmsg("/usr/bin/lpr: {s}",STRERROR(errno));
				break;
				}
			if( !recalculate ) then evalall();
			for( r=r1; r<=r2; r++ )
			    {
			    for( c=c1; c<=c2; c++ )
				{
				if( (p = table[r][c]) == NULL )
				  then
				    for(i=0;i<colwidth[c];i++)
					iooutc(outfile,' ');
				  else
				    if( p->e_text != NULL )
				      then fformat(outfile,"{slm}",
					p->e_text,colwidth[c],colwidth[c]);
				      else fformat(outfile,"{fr.}",
					p->e_value,colwidth[c],colprec[c]);
				}
			    iooutc( outfile, '\n' );
			    }
			ioclose( outfile );
			if( !*tul ) then wait(0);
			break;

	default:	iooutc( stdout, 7 );
			break;
	}
    }

subroutine update()
    {
    int r, c;
    int i;
    int sr, sc;
    struct entry *p;

    if( currow<disrow || curcol<discol || currow>=disrow+screenrows-2 )
      then center();
      else
	{
	for( c=discol,sc=0; c<=curcol; sc+=colwidth[c++] )
	    if( sc+colwidth[c] >= screencols ) then break;
	if( c <= curcol ) then center();
	}
    move( 0, 0 );
    wformat("@r{ir2p48}c{ir2p48}: ",currow,curcol);
    if( (p = table[currow][curcol]) == NULL ) then wformat("(Blank)\n");
      else if( p->e_text != NULL ) then wformat("\"{s}\"\n",p->e_text);
      else if( p->e_expr != NULL ) then wformat("{s}\n",p->e_expr);
      else wformat("{f.}\n",p->e_value,colprec[curcol]);
    if( changed )
      then
	{
	move( 2, 0 );
	clrtobot();
	for( sr=2,r=disrow; sr<screenrows && r<=mrow; sr++,r++ )
	    for( sc=0,c=discol; sc+colwidth[c]<screencols && c<=mcol;
		sc+=colwidth[c++] )
		{
		if( (p = table[r][c]) != NULL )
		  then
		    {
		    move( sr, sc );
		    if( !showlabels )
		      then i = -1;
		      else
			{
			for( i=0; labels[i]!=NULL; i++ )
			    if( lptrs[i] == p ) then break;
			if( labels[i] == NULL ) then i = -1;
			}
		    if( i > -1 )
		      then wformat("{slm}",labels[i],colwidth[c],colwidth[c]);
		    else if( p->e_status & S_ERROR )
		      then wformat("{cn}",'?',colwidth[c]);
		    else if( p->e_text != NULL )
		      then wformat("{slm}",p->e_text,colwidth[c],colwidth[c]);
		      else wformat("{fr.}",p->e_value,colwidth[c],colprec[c]);
		    }
		}
	}
    sr = currow - disrow + 2;
    for( c=discol,sc=0; c<curcol; sc+=colwidth[c++] )	;
    move( sr, sc );
    changed = FALSE;
    }

subroutine main( int argc, char *argv[] )
{
int r, c;
int i;
double outvalue;

signal( SIGFPE, trapfpe );
args = argv;
nargs = argc;
if( nargs < 1 || nargs > 3 )
  then
    {
    fformat(stderr,"Usage:  {s} [file [expression]]\n",args[0]);
    exit(1);
    }

if( nargs < 2 )
  then batch = NULL;
  else batch = ioopen(args[1],"r");
if( nargs < 3 )
  then outexpr = NULL;
  else outexpr = argv[2];

for( c=0; c<MAXCOL; c++ )
    {
    colwidth[c] = DEFWIDTH;
    colprec[c] = DEFPREC;
    for( r=0; r<MAXROW; r++ ) table[r][c] = NULL;
    }

labels = (char **)malloc( sizeof(char *) );
lptrs = (struct entry **)malloc(1);
labels[ 0 ] = NULL;

if( outexpr == NULL )
  then
    {
    initscr();
    setkluge();
    clear();
    move( 1, 0 );
    if( nargs==1 )
      then wformat("[{s}, V{f.2}]",argv[0],VERSION);
    else if( batch==NULL )
      then wformat("[{s}, V{f.2}, New file {s}]",argv[0],VERSION,args[1]);
      else wformat("[{s}, V{f.2}, Old file {s}]",argv[0],VERSION,args[1]);
    }

while( TRUE )
    {
    if( batch != NULL )
      then
	if( fgets( combuf, 100, batch ) != NULL )
	  then
	    {
	    i = strlen( combuf ) - 1;
	    if( i>=0 && combuf[i]=='\n' ) then combuf[i] = 0;
	    if( sscanf(combuf,"%d",&iter) < 1 ) then iter = 0;
	    for( command=combuf; isdigit(*command); command++ );
	    }
	  else
	    {
	    ioclose( batch );
	    batch = NULL;
	    if( !recalculate ) then evalall();
	    if( outexpr!=NULL )
	      then
		if( eval(0,0,outexpr,&outvalue) )
		  then
		    {
		    format("{f.10}\n",outvalue);
		    exit(0);
		    }
		  else
		    {
		    fformat(stderr,"Error in calculation {s}.\n",outexpr);
		    exit(1);
		    }
	    }
    if( batch == NULL )
      then
	{
	if( recalculate && changed ) then evalall();
	update();
	refresh();
	iter = 0;
	command = combuf;
	while( isdigit( *command=getch() ) )
	    {
	    iter = 10*iter + digtobin(*command);
	    move( 1, 0 );
	    wformat("Repeat:  {i}",iter);
	    clrtoeol();
	    refresh();
	    }
	move( 1, 0 );
	clrtoeol();
	command[1] = 0;
	}
    if( iter <= 0 ) then iter = 1;
    if( command[0]!='a' && command[0]!='.' )
      then repflag = FALSE;
      else
	{
	strcpy( command, repbuf );
	repflag = TRUE;
	}
    execcommand();
    }
}

#ifdef ULTRIX11
static struct tchars ot, nt;
static struct ltchars olt, nlt;
#endif
static int setup = 0;

subroutine setkluge()
    {
    int i;

    if( !setup )
      then
	{
#ifdef ULTRIX11
	ioctl( 0, TIOCGETC, &ot );
	for( i=0; i<sizeof(ot); i++ ) ((char *)&nt)[i]=((char *)&ot)[i];
	nt.t_intrc = nt.t_quitc = nt.t_brkc = -1;
	ioctl( 0, TIOCGLTC, &olt );
	for( i=0; i<sizeof(olt); i++ ) ((char *)&nlt)[i]=((char *)&olt)[i];
	nlt.t_suspc = nlt.t_dsuspc = nlt.t_flushc = nlt.t_lnextc = -1;
#endif
	setup = 1;
	}
    crmode();
    noecho();
    nonl();
#ifdef ULTRIX11
    ioctl( 0, TIOCSETC, &nt );
    ioctl( 0, TIOCSLTC, &nlt );
#endif
    }

subroutine resetkluge()
    {
    nocrmode();
    echo();
    nl();
#ifdef ULTRIX11
    ioctl( 0, TIOCSETC, &ot );
    ioctl( 0, TIOCSLTC, &olt );
#endif
    }
