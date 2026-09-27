/***************************************************************************
	time.c:  Routines to manipulate time
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_time == LOCAL_HANDLER

#include <sys/types.h>
#include <time.h>
#include <ctype.h>

long timezone = -1;
int usedaylight;
int daylight = FALSE;
char *tzname[2];
char zonename[2][4];

static struct tm beginning =
    {
    0,		/* tm_sec	*/
    0,		/* tm_min	*/
    0,		/* tm_hour	*/
    0,		/* tm_mday	*/
    0,		/* tm_mon	*/
    70,		/* tm_year	*/
    4,		/* tm_wday	*/
    0,		/* tm_yday	*/
    1		/* tm_isdst	*/
    };

char dayspermonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

char *monthnames[] =
    {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL
    };

char *daynames[] =
    {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    NULL
    };

/************************************************************************/
/************************************************************************/
function static int jan1(int year)
    {
    register int day;

    day = year + 4 + ((year + 3) / 4);	/* Julian calendar */
    if( year > 1800 )
      then
	{				/* Apply recent corrections */
	day -= ((year - 1701) / 100);	/* Clavian correction */
	day += ((year - 1601) / 400);	/* Gregorian correction */
	}
    if( year > 1752 ) then day += 3;	/* Adjust for Gregorian calendar */
    return day % 7;
    }

/************************************************************************/
/************************************************************************/
function static int numdays(int year,int month,int day)
    {
    int i, res;

    for( res=i=0; i<month-1; i++ )
	{
	res += dayspermonth[i];
	if( (year%4==0) && i==1 )
	  then
	    {
	    res++;
	    if( year > 1800 )
	      then
		{
		if(year%100==0) then res--;	/* Clavian correction */
		if(year%400==0) then res++;	/* Gregorian correction */
		}
	    }
	}
    res += day;
    return res - 1;
    }

/************************************************************************/
/************************************************************************/
extern IOFILE *printer;
void tzset()
    {
    char *tzn;
    int i, j, negflg;
    extern char *getenv();

    if( timezone >= 0 ) then return;
    if( (tzn = getenv("TZ")) == NULL ) then return;
    for( i=0; isalpha(tzn[i]) && i<3; i++ ) zonename[0][i] = tzn[i];
    zonename[0][i] = 0;
    tzname[0] = zonename[0];
    if( tzn[i]=='-' )
      then { negflg=TRUE; i++; }
      else
        if( tzn[i]=='+' )
	  then { negflg=FALSE; i++; }
	  else negflg = FALSE;
    for( timezone=0; isdigit(tzn[i]); i++ )
	if( negflg )
	  then timezone = 10*timezone - digtobin( tzn[i] );
	  else timezone = 10*timezone + digtobin( tzn[i] );
    if( timezone < 0 ) then timezone += 24;
    if( !tzn[i] )
      then usedaylight = FALSE;
      else
        {
	for( j=0; isalpha(tzn[i]) && j<3; i++,j++ ) zonename[1][j] = tzn[i];
	zonename[1][j] = 0;
	usedaylight = TRUE;
	}
    tzname[1] = zonename[1];
    }

/************************************************************************/
/************************************************************************/
function struct tm *gmtime(time_t *nclock)
    {
    static struct tm t;
    int i;
    time_t clock;

    clock = *nclock;
    t.tm_sec = beginning.tm_sec + clock%60;		clock /= 60;
    t.tm_min = beginning.tm_min + clock%60;		clock /= 60;
    t.tm_hour = beginning.tm_hour + clock%24;		clock /= 24;
    t.tm_year = beginning.tm_year;
    while(TRUE)
	{
	if( t.tm_year%4==0 && (t.tm_year%100!=0 || t.tm_year%400==0) )
	  then i = 366;
	  else i = 365;
	if( i > clock ) then break;
	clock -= i;
	t.tm_year++;
	}
    t.tm_mon = beginning.tm_mon;
    t.tm_yday = clock;
    t.tm_wday = (t.tm_yday + jan1(t.tm_year+1900)) % 7;
    while(TRUE)
	{
	i = dayspermonth[ t.tm_mon ];
	if( t.tm_mon==1 && t.tm_year%4==0 &&
	    (t.tm_year%100!=0 || t.tm_year%400==0) )
	  then i++;
	if( i > clock ) then break;
	clock -= i;
	t.tm_mon++;
	}
    t.tm_mday = beginning.tm_mday + clock;
    return &t;
    }

/************************************************************************/
/************************************************************************/
function struct tm *localtime(time_t *clock)
    {
    struct tm *t;
    time_t offsettime;

    t = gmtime( clock );
    offsettime = *clock;
    tzset();
    if( daylight )
      then offsettime -= 60*60*(timezone+1);
      else offsettime -= 60*60*timezone;
    return gmtime( &offsettime );
    }

/************************************************************************/
/************************************************************************/
function time_t gmtotime(struct tm *t)
    {
    time_t res;
    int i;

    if( t==0 ) then return -1;
    res = 0;
    for( i=beginning.tm_year; i<t->tm_year; i++ )
	{
	res += 365;
	if( i%4==0 )
	  then
	    {
	    res++;
	    if( i%100==0 ) then res--;
	    if( i%400==0 ) then res++;
	    }
	}
    for( i=beginning.tm_mon; i<t->tm_mon; i++ )
	{
	res += dayspermonth[i];
	if( t->tm_year%4 == 0 && i==1 )
	  then
	    {
	    res++;
	    if( t->tm_year%100 == 0 ) then res--;
	    if( t->tm_year%400 == 0 ) then res++;
	    }
	}
    res += t->tm_mday - beginning.tm_mday;
    res = 24*res + t->tm_hour - beginning.tm_hour;
    res = 60*res + t->tm_min - beginning.tm_min;
    res = 60*res + t->tm_sec - beginning.tm_sec;
    return res;
    }

/************************************************************************/
/************************************************************************/
function time_t ltotime(struct tm *t)
    {
    time_t offsettime;
    tzset();
    if( (offsettime = gmtotime( t )) < 0 ) then return -1;
    if( daylight )
      then offsettime += 60*60*(timezone+1);
      else offsettime += 60*60*timezone;
    return offsettime;
    }

/************************************************************************/
/************************************************************************/
#define PARSEBUFSIZE			100
function struct tm *parsetime(char *s)
    {
    char *pss[5];
    char pbf[PARSEBUFSIZE];
    int i, j, k, l, state, numstr;
    static struct tm t;
    struct tm *ct;
    long curtime;

    time( &curtime );
    ct = localtime( &curtime );
    setbytes( &t, -1, sizeof(struct tm) );
    for( state=numstr=i=j=0; s[i]; i++ )
	if( s[i]==',' || s[i]<=' ' || s[i]>'~' )
	  then
	    {
	    pbf[j]=0;
	    if( numstr==5 ) then return NULL;
	    if( state!=1 && ++j >= PARSEBUFSIZE-1 ) then return NULL;
	    state = 1;
	    }
	  else
	    {
	    pbf[j] = s[i];
	    if( state != 2 ) then pss[numstr++] = &pbf[j];
	    if( ++j >= PARSEBUFSIZE-1 ) then return NULL;
	    state = 2;
	    }
    pbf[j] = 0;
    if( numstr < 0 ) then return NULL;
    for( i=0; i<numstr; i++ )
	{
	s = pss[i];
	if( strchr(s,'/') != NULL )
	  then
	    {
	    for( j=k=0; isdigit(s[k]); k++ )
		if( (j = j*10 + s[k]-'0') > 12 ) then return NULL;
	    if( --j < 0 ) then return NULL;
	    if( t.tm_mon >= 0 && t.tm_mon != j )
	      then return NULL;
	      else t.tm_mon = j;
	    if( s[k++] != '/' ) then return NULL;
	    l = k;
	    for( j=0; isdigit(s[k]); k++ )
		if( (j = j*10 + s[k]-'0') > 3275 ) then return NULL;
	    if( j > 31 )
	      then k = l-1;
	      else
		{
		if( --j < 0 ) then return NULL;
		if( t.tm_mday >= 0 && t.tm_mday != j )
		  then return NULL;
		  else t.tm_mday = j;
		}
	    if( s[k] != 0 )
	      then
		{
		if( s[k++] != '/' ) then return NULL;
		for( j=0; isdigit(s[k]); k++ )
		    if( (j = j*10 + s[k]-'0') > 3275 ) then return NULL;
		if( j >= 1900 ) then j -= 1900;
		if( t.tm_year >= 0 && t.tm_year != j )
		  then return NULL;
		  else t.tm_year = j;
		if( s[k] != 0 ) then return NULL;
		}
	    }
	else if( strchr(s,'-') != NULL )
	  then
	    {
	    for( j=k=0; isdigit(s[k]); k++ )
		if( (j = j*10 + s[k]-'0') > 31 ) then return NULL;
	    if( --j < 0 ) then return NULL;
	    if( t.tm_mday >= 0 && t.tm_mday != j )
	      then return NULL;
	      else t.tm_mday = j;
	    if( s[k++] != '-' ) then return NULL;
	    for( l=k; isalpha(s[k]); k++ )	;
	    state = s[k];
	    s[k] = 0;
	    if( (j = orig_abbrev( &s[l], monthnames )) < 0 ) then return NULL;
	    s[k] = state;
	    if( t.tm_mon >= 0 && t.tm_mon != j )
	      then return NULL;
	      else t.tm_mon = j;
	    if( s[k] != 0 )
	      then
		{
		if( s[k++] != '-' ) then return NULL;
		for( j=0; isdigit(s[k]); k++ )
		    if( (j = j*10 + s[k]-'0') > 3275 ) then return NULL;
		if( j >= 1900 ) then j -= 1900;
		if( t.tm_year >= 0 && t.tm_year != j )
		  then return NULL;
		  else t.tm_year = j;
		if( s[k] != 0 ) then return NULL;
		}
	    }
	else if( strchr(s,':') != NULL || strchr(s,'.') != NULL )
	  then
	    {
	    for( j=k=0; isdigit(s[k]); k++ )
		if( (j = 10*j + s[k]-'0') > 23 ) then return NULL;
	    if( t.tm_hour >= 0 && t.tm_hour != j )
	      then return NULL;
	      else t.tm_hour = j;
	    if( s[k] != ':' && s[k] != '.' ) then return NULL;
	    k++;
	    for( j=0; isdigit(s[k]); k++ )
		if( (j = 10*j + s[k]-'0') > 59 ) then return NULL;
	    if( t.tm_min >= 0 && t.tm_min != j )
	      then return NULL;
	      else t.tm_min = j;
	    if( s[k] != 0 )
	      then
		{
		if( s[k] != ':' && s[k] !='.' ) then return NULL;
		k++;
		for( j=0; isdigit(s[k]); k++ )
		    if( (j = 10*j + s[k]-'0') > 59 ) then return NULL;
		if( t.tm_sec >= 0 && t.tm_sec != j )
		  then return NULL;
		  else t.tm_sec = j;
		if( s[k] != 0 ) then return NULL;
		}
	    }
	else if( (j=orig_abbrev(s,monthnames)) >= 0 )
	  then
	    if( t.tm_mon >= 0 && t.tm_mon != j )
	      then return NULL;
	      else t.tm_mon = j;
	else if( (j=orig_abbrev(s,daynames)) >= 0 )
	  then
	    if( t.tm_wday >= 0 && t.tm_wday != j )
	      then return NULL;
	      else t.tm_wday = j;
	else
	    {
	    j = 0;
	    for( k=0; isdigit(s[k]); k++ )
	        if( (j = 10*j + s[k]-'0') > 3275 ) then return NULL;
	    if( s[k]!=0 ) then return NULL;
	    if( j>=1 && j<=31 )
	      then
		if( t.tm_mday >= 0 && t.tm_mday != --j )
		  then return NULL;
		  else t.tm_mday = j-1;
	    else if( j >= 1900 )
	      then
		if( t.tm_year >= 0 && t.tm_year != j-1900 )
		  then return NULL;
		  else t.tm_year = j-1900;
	    else return NULL;
	    }
	}
    t.tm_wday = -1;
    if( t.tm_year < 0 )	then t.tm_year	= ct->tm_year;
    if( t.tm_mon  < 0 )	then t.tm_mon	= ct->tm_mon;
    if( t.tm_mday < 0 )	then t.tm_mday	= ct->tm_mday;
    if( t.tm_hour < 0 )	then t.tm_hour	= ct->tm_hour;
    if( t.tm_min  < 0 )	then t.tm_min	= ct->tm_min;
    if( t.tm_sec  < 0 )	then t.tm_sec	= ct->tm_sec;
    t.tm_yday = numdays( t.tm_year + 1900, t.tm_mon+1, t.tm_mday+1 );
    t.tm_wday = (t.tm_yday + jan1( t.tm_year + 1900 ) ) % 7;
    return &t;
    }

/************************************************************************/
/************************************************************************/
function char *asctime(struct tm *t)
    {
    static char atime[26], *s;
    if( t == NULL )
      then sformat(atime,"(Null time)\n");
      else
	sformat(atime,"{sm3} {sm3} {ir2} {iz2}:{iz2}:{iz2} {iz4}\n",
	    ( t->tm_wday>=0	? daynames[ t->tm_wday ]	: "???" ),
	    ( t->tm_mon>=0	? monthnames[ t->tm_mon ]	: "???" ),
	    ( t->tm_mday>=0	? t->tm_mday+1			: -1 ),
	    t->tm_hour,
	    t->tm_min,
	    t->tm_sec,
	    ( t->tm_year>=0	? t->tm_year+1900		: -1 )
	    );
    return atime;
    }

/************************************************************************/
/************************************************************************/
function char *ctime(time_t *ti)
    {
    return asctime( localtime( ti ) );
    }

/************************************************************************/
/************************************************************************/
function time_t gtime( char *s )
    {
    return ltotime( parsetime( s ) );
    }

#ifdef MAIN
subroutine main( int argc, char *argv[] )
{
time_t curtime;
if( argc !=2  )
  then
    {
    fformat(stderr,"Usage:  {s} <time>\n",argv[0]);
    exit(1);
    }
time( &curtime );
format("Current time is (GMT):  {s}", asctime( gmtime( &curtime ) ) );
format("Current time is:  {s}", ctime( &curtime ) );
curtime = gtime( argv[1] );
format("{s}",asctime( gmtime( &curtime ) ) );
format("{s}",ctime( &curtime ) );
exit(0);
}
#endif

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_time()
    {
    }
#endif
