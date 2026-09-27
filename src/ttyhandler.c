/************************************************************************
 *
 *indx#	ttyhandler.c - Routines for doing i/o to a terminal
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
 *doc#	Routines for doing i/o to a terminal
 ************************************************************************/
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <local_iosubs.h>
#include <local_error.h>
#include <local_format.h>
#include <local_string.h>
#include <local_mem.h>
#include <local_lock.h>
#include <local_capparse.h>

#if HANDLER_termio == SYS3_HANDLER
#include <termlocal_iosubs.h>
#elif HANDLER_termio == BSD_LEGACY_HANDLER
#include <sgtty.h>
#endif

struct sysdep
    {
#if HANDLER_termio == SYS3_HANDLER
    struct termio			sd_termio;
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    struct sgttyb			sd_sgttyb;
#endif
    };

typedef struct sysdep *stateptr;

#define IN_TTYHANDLER
#include <ttyhandler.h>

#define CONSIG				SIGHUP

#define IOBUFSIZE(iop)			BUFSIZ

extern char *sys_errlist[];
extern int errno;

#if HANDLER_termio == BSD_LEGACY_HANDLER
void alarmtrap()
    {
    signal( SIGALRM, alarmtrap );
    }
#endif

/**************************************************************************/
function stateptr ttycopystate( stateptr old )
/**************************************************************************/
    {
    stateptr res;
    res = (stateptr)malloc( sizeof(struct sysdep) );
    movebytes( res, old, sizeof(struct sysdep) );
    return res;
    }

/**************************************************************************/
subroutine ttyfree( ttyptr tty )
/**************************************************************************/
    {
    free( tty->ts_name );
    if( tty->ts_pentry != NULL ) then c_p_clean( tty->ts_pentry );
    free( tty->ts_oldstate );
    free( tty->ts_altstate );
    free( tty );
    }

/**************************************************************************/
function stateptr ttyoldstate( ttyptr tty )
/**************************************************************************/
    {
    stateptr res;
    res = (stateptr)malloc( sizeof(struct sysdep) );
#if HANDLER_termio == SYS3_HANDLER
    if( ioctl( IOCHAN(tty->ts_ioin), TCGETA, &(res->sd_termio) ) < 0 )
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    if( ioctl( IOCHAN(tty->ts_ioin), TIOCGETP, &(res->sd_sgttyb) ) < 0 )
#endif
      then
	{
	eformat("ttyoldstate couldn't get terminal info for {s}:  {s}",
	    tty->ts_name, sys_errlist[ errno ] );
	ttyfree( tty );
	return NULL;
	}
    return res;
    }

/**************************************************************************/
function int ttysetstate( ttyptr tty, stateptr newstate )
/**************************************************************************/
    {
    ttyflush(tty);
    ttyclearbuf(tty);
#if HANDLER_termio == BSD_LEGACY_HANDLER
    signal(SIGALRM,alarmtrap);
#endif
    if( tty->ts_curstate & TTY_NOTTTY )
      then
	{
	eformat("ttysetstate applied to non-terminal:  {s}",tty->ts_name);
	return FALSE;
	}
#if HANDLER_termio == SYS3_HANDLER
    if( ioctl(IOCHAN(tty->ts_ioin),TCSETA,&(newstate->sd_termio))<0 )
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    if( ioctl(IOCHAN(tty->ts_ioin),TIOCSETP,&(newstate->sd_sgttyb))<0 )
#endif
      then
	{
#ifndef SPEEDFAILURE
	eformat("ttysetstate couldn't condition {s}:  {s}",
	    tty->ts_name, sys_errlist[ errno ] );
	return FALSE;
#endif
	}
    return TRUE;
    }

/**************************************************************************/
function stateptr ttycbcstate( stateptr res )
/**************************************************************************/
    {
#if HANDLER_termio == SYS3_HANDLER
    res->sd_termio.c_lflag 	&=	~(ICANON|ECHO|ECHOE|ECHOK|ECHONL);
    res->sd_termio.c_iflag	&=	~(INLCR|IGNCR|ICRNL);
    res->sd_termio.c_oflag	&=	~(OPOST);
    res->sd_termio.c_cc[4]	=	0;
    res->sd_termio.c_cc[5] 	=	5;
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    res->sd_sgttyb.sg_flags	&=	~(ECHO|CRMOD);
    res->sd_sgttyb.sg_flags	|=	CBREAK;
#endif
    return res;
    }

/**************************************************************************/
function stateptr ttybinstate( stateptr res )
/**************************************************************************/
    {
#if HANDLER_termio == SYS3_HANDLER
    res->sd_termio.c_lflag 	&=	~(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHONL);
    res->sd_termio.c_iflag	&=	~(INLCR|IGNCR|ICRNL);
    res->sd_termio.c_oflag	&=	~(OPOST);
    res->sd_termio.c_cc[4]	=	0;
    res->sd_termio.c_cc[5] 	=	5;
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    res->sd_sgttyb.sg_flags	&=	~(ECHO|CRMOD);
    res->sd_sgttyb.sg_flags	|=	CBREAK;
#endif
    return res;
    }

/**************************************************************************/
function int ttybaud( ttyptr tty, int ib )
/**************************************************************************/
    {
#ifdef B110
    if( ib == 110 )	then ib = B110;		else
#endif
#ifdef B220
    if( ib == 220 )	then ib = B220;		else
#endif
#ifdef B300
    if( ib == 300 )	then ib = B300;		else
#endif
#ifdef B600
    if( ib == 600 )	then ib = B600;		else
#endif
#ifdef B1200
    if( ib == 1200 )	then ib = B1200;	else
#endif
#ifdef B2400
    if( ib == 2400 )	then ib = B2400;	else
#endif
#ifdef B4800
    if( ib == 4800 )	then ib = B4800;	else
#endif
#ifdef B9600
    if( ib == 9600 )	then ib = B9600;	else
#endif
#ifdef B19200
    if( ib == 19200 )	then ib = B19200;	else
#endif
        {
	eformat("Unknown baud rate {i}",ib);
	return FALSE;
	}
#if HANDLER_termio == SYS3_HANDLER
    tty->ts_altstate->sd_termio.c_cflag		&= ~(CBAUD|PARENB);
    tty->ts_altstate->sd_termio.c_cflag		|= ib;
#elif HANDLER_termio == BSD_LEGACY_HANDLER
    tty->ts_altstate->sd_sgttyb.sg_ispeed	= ib;
    tty->ts_altstate->sd_sgttyb.sg_ospeed	= ib;
#endif
    return TRUE;
    }

/**************************************************************************/
function ttyptr ttymake( IOFILE ioin, IOFILE ioout, char *name )
/**************************************************************************/
    {
    int i;
    ttyptr tty;

    tty = (ttyptr)malloc( sizeof(struct ttystruct) );
    tty->ts_ioin = ioin;
    tty->ts_ioout = ioout;
    tty->ts_name = string( name );
    tty->ts_pentry = NULL;
    if( !isatty( IOCHAN(tty->ts_ioin) ) )
      then
	{
	tty->ts_curstate = TTY_NOTTTY;
	return tty;
	}
    tty->ts_curstate = 0;

    tty->ts_oldstate = ttyoldstate( tty );
    tty->ts_altstate = ttybinstate( ttycopystate( tty->ts_oldstate ) );
    return tty;
    }

#ifdef ISIII
    static char *lockmask = "/etc/locks/{s}";
#else
#ifdef ATT3BX
    static char *lockmask = "/usr/spool/locks/LCK..{s}";
#else
    static char *lockmask = "/usr/spool/uucp/LCK..{s}";
#endif
#endif

/**************************************************************************/
function int ttylock( char *resource )
/**************************************************************************/
    {
    return lockresource( lockmask, resource );
    }

/**************************************************************************/
function int ttyunlock( char *resource )
/**************************************************************************/
    {
    return unlockresource( lockmask, resource );
    }

/**************************************************************************/
function ttyptr ttyopen( char *resource )
/**************************************************************************/
    {
    int fd;
    IOFILE ioin, ioout;
    ttyptr res;

    if( !ttylock( resource ) ) then return NULL;

    if( (ioin = ioopen( resource, "r" )) == NULL )
      then
	{
	ttyunlock( resource );
	eformat("Unable to open {s}:  {s}",resource,sys_errlist[errno]);
	return NULL;
	}
    if( (ioout = ioopen( resource, "w" )) == NULL )
      then
	{
	ioclose( ioin );
	ttyunlock( resource );
	eformat("Unable to open {s}:  {s}",resource,sys_errlist[errno]);
	return NULL;
	}

    if( (res = ttymake( ioin, ioout, resource ) ) == NULL )
      then
	{
	ioclose( ioin );
	ioclose( ioout );
	ttyunlock( resource );
	return NULL;
	}

    return res;
    }

/**************************************************************************/
function ttyptr ttyport( char *portname )
/**************************************************************************/
    {
    ttyptr t;
    char *entry, *remfname, *devname, *tempname;
    struct c_pentry *pentry;

    if( (remfname = getenv("HOME")) == NULL ) then remfname = "";
    remfname = mformat("{s}/.remote",remfname);
    if( (entry=gentry(remfname,portname)) == NULL )
      then entry=gentry("/etc/remote",portname);
    free( remfname );
    if( entry != NULL )
      then pentry = parseentry(entry);
      else pentry = NULL;
    if( portname[0] == '/' )
      then devname = string(portname);
      else devname = mformat("/dev/{s}",portname);
    if( (tempname = strsearch(pentry,"dv",NULL)) != NULL )
      then
	{
	free( devname );
	devname = string(tempname);
	}
    if( (t = ttyopen( devname )) == NULL )
      then
	{
	eformat("Unknown device {s}",devname,sys_errlist[errno]);
	free( devname);
	if( entry != NULL ) then free( entry );
	if( pentry != NULL ) then free( pentry );
	return FALSE;
	}
    free( devname );
    t->ts_pentry = pentry;
    ttybaud( t, intsearch(pentry,"br",9600) );
    return t;
    }

/**************************************************************************/
function int ttyrestore( ttyptr tty )
/**************************************************************************/
    {
    if( !(tty->ts_curstate & TTY_NOTTTY)	&&
	!ttysetstate( tty, tty->ts_oldstate )	) then return FALSE;
    tty->ts_curstate &= ~TTY_BINMODE;
    return TRUE;
    }

/**************************************************************************/
subroutine ttyclose( ttyptr tty )
/**************************************************************************/
    {
    IOFILE ioin, ioout;

    ttyrestore( tty );
    ioclose( tty->ts_ioin );
    ioclose( tty->ts_ioout );
    ttyunlock( tty->ts_name );
    ttyfree( tty );
    }

/**************************************************************************/
function int ttybinary( ttyptr tty )
/**************************************************************************/
    {
    if( !ttysetstate( tty, tty->ts_altstate ) ) then return FALSE;
    tty->ts_curstate |= TTY_BINMODE;
    return TRUE;
    }

/**************************************************************************/
static function int ttynuminbuf( ttyptr tty )
/**************************************************************************/
    {
#ifdef FIONREAD
    int32 numchars;
    if( ioctl( IOCHAN(tty->ts_ioin), FIONREAD, &numchars ) >= 0 )
      then return numchars;
#endif
    return 0;
    }

/**************************************************************************/
subroutine ttyformat( ttyptr tty, char *fmt, ... )
/**************************************************************************/
    {
    XIFY( ap, fmt, int ret=fxformat(tty->ts_ioout, fmt, ap ), 1 );
    }

/**************************************************************************/
subroutine ttyflformat( ttyptr tty, char *fmt, va_list ap )
/**************************************************************************/
    {
    fflxformat( tty->ts_ioout, fmt, ap );
    }

/**************************************************************************/
subroutine ttysout( ttyptr tty, char *s )
/**************************************************************************/
    {
    while( *s ) ttyout(tty,*s++);
    }

/**************************************************************************/
function int ttyread( ttyptr tty, char *buf, int sizebuf )
/**************************************************************************/
    {
    int numinbuf;
    ttyflush(tty);
    if( !(tty->ts_curstate & TTY_BINMODE) )
      then return ioread( tty->ts_ioin, buf, sizebuf );
    if( (numinbuf=ttynuminbuf(tty)) <= 0 )
      then numinbuf = 0;
      else
	{
	if( numinbuf > sizebuf ) then numinbuf = sizebuf;
	if( (numinbuf=ioread( tty->ts_ioin, buf, numinbuf )) < 0 )
	  then numinbuf = 0;
	}
    if( numinbuf >= sizebuf )
      then return sizebuf;
      else return read( IOCHAN(tty->ts_ioin), buf+numinbuf, sizebuf-numinbuf );
    }

/**************************************************************************/
function int ttyreadnowait( ttyptr tty, char *buf, int sizebuf )
/**************************************************************************/
    {
    int numinbuf;
    ttyflush(tty);
    if( !(tty->ts_curstate & TTY_BINMODE) )
      then return ioread( tty->ts_ioin, buf, sizebuf );

#if HANDLER_termio == BSD_LEGACY_HANDLER
    alarm( 2 );
#endif
    numinbuf = ioread( tty->ts_ioin, buf, sizebuf );
#if HANDLER_termio == BSD_LEGACY_HANDLER
    alarm( 0 );
#endif
    if( numinbuf <= 0 )
      then return IOERROR;
      else return numinbuf;
    }

/**************************************************************************/
function int ttyin( ttyptr tty )
/**************************************************************************/
    {
    char c;
    if( ttyread( tty, &c, 1 ) >= 1 ) then return c & 0xff;
    return IOERROR;
    }

/**************************************************************************/
function int ttyinnowait( ttyptr tty )
/**************************************************************************/
    {
    char c;
    if( ttyreadnowait( tty, &c, 1 ) >= 1 ) then return c & 0xff;
    return IOERROR;
    }

/**************************************************************************/

int escapechar = 033;

static int caughtsig;
static int exitflag;
static IOFILE cmdfile, logfile;
static ttyptr con, rem;

subroutine trapsig(int x)	{ caughtsig=TRUE; }

/**************************************************************************/
subroutine ttyhelp(ttyptr tty)
/**************************************************************************/
    {
    ttyrestore(tty);
    ttyformat(tty,"\nEscaped commands (Escape='{cu1}'):\n",escapechar);
    ttyformat(tty,"    ?         - Help\n");
    ttyformat(tty,"    q         - Quit\n");
    ttyformat(tty,"    <filename - Send file to remote\n");
    ttyformat(tty,"    >filename - Put remote output in file\n");
    ttyformat(tty,"    {{command  - Send command output to remote\n");	/*}*/
    ttyformat(tty,"    }command  - Send remote output to command\n");
    ttyformat(tty,"    !command  - Execute command\n");
    ttyformat(tty,"    schar     - Set new escape character\n");
    ttybinary(tty);
    }

/**************************************************************************/
function int ttycommand( ttyptr tty, char com )
/**************************************************************************/
    {
    char comline[100];
    int i;

    switch( com )
	{
	case 'h':
	case 'H':
	case '?':	ttyhelp(tty);
			break;

	case 003:
	case 004:
	case 'e':
	case 'E':
	case 'q':
	case 'Q':	exitflag=TRUE;
			return TRUE;

	case 's':
	case 'S':	ttyformat(tty,
			    "\r\nEnter new escape character: ");
			ttyread( tty, (char*)&escapechar, 1 );
			ttyformat(tty,"{cu1}\r\n",escapechar);
			ttyflush(tty);
			return FALSE;

	case '!':	ttyrestore(tty);
			ttyformat(tty, "\nShell command: ");
			iogetl( tty->ts_ioin, comline, 100 );
			for( i=0; comline[i]>=' ' || comline[i]=='\t'; i++ );
			comline[i] = 0;
			if( comline[0] != 0 )
			  then system( comline );
			  else
			    if( fork()==0 )
			      then execl("/bin/sh", "sh", 0 );
			      else wait(0);
			ttyformat(tty,"\n[Done]\n");
			ttybinary(tty);
			return FALSE;

	case '{':
	case '<':	ttyrestore(tty);
			if( cmdfile != NULL ) then ioclose(cmdfile);
			ttyformat(tty,
			    "\nInput {s}: ",(com=='{'?"command":"file"));
			iogetl( tty->ts_ioin, comline, 100 );
			for( i=0; comline[i]>=' ' || comline[i]=='\t'; i++ );
			comline[i] = 0;
			if( com=='{' )
			  then cmdfile=iopipe(comline,"r");
			  else cmdfile=ioopen(comline,"r");
			if( cmdfile==NULL ) then perror(comline);
			ttybinary(tty);
			return FALSE;

	case '}':
	case '>':	ttyrestore(tty);
			if( logfile != NULL )
			  then
			    {
			    ioclose(logfile);
			    ttyformat(tty,"\n[End log]\n");
			    logfile=NULL;
			    }
			  else
			    {
			    ttyformat(tty,
				"\nOutput {s}: ",
				(com=='}'?"command":"file"));
			    iogetl( tty->ts_ioin, comline, 100 );
			    for(i=0;comline[i]>=' '||comline[i]=='\t';i++);
			    comline[i] = 0;
			    if( com=='}' )
			      then logfile=iopipe(comline,"w");
			      else logfile=ioopen(comline,"w");
			    if( logfile==NULL ) then perror(comline);
			    }
			ttybinary(tty);
			return TRUE;
	
	default:	ttyformat(tty,"\007");
			ttyflush(tty);
			return FALSE;
	}
    }

/**************************************************************************/
subroutine ttyconnect( ttyptr v_con, ttyptr v_rem )
/**************************************************************************/
    {
    int i;
    int escaped;
    int thekid;
    int nread;
    char buf[BUFSIZ];

    escaped = FALSE;
    caughtsig = FALSE;
    exitflag = FALSE;
    cmdfile = logfile = NULL;
    con = v_con;
    rem = v_rem;

    signal( CONSIG, trapsig );
    ttyformat(con,"[For help, type {cu1}?]\n",escapechar);
    ttybinary( con );
    ttybinary( rem );
    while( !exitflag )
	{
	if( (thekid = fork()) == 0 )
	  then
	    {
	    if( cmdfile != NULL ) then ioclose( cmdfile );
	    while( !caughtsig )
		{
		nread=ttyread(rem,buf,1);
		if( nread > 0 )
		  then
		    {
		    if( (nread = ttynuminbuf(rem)) > 0 )
		      then
			if( (nread=ttyread(rem,buf+1,nread)) < 0 )
			  then nread = 0;
		    nread++;
		    ttyfwrite( con, buf, nread );
		    if(logfile!=NULL) then iowrite(logfile,buf,nread);
		    }
		}
	    if( logfile != NULL ) then ioclose( logfile );
	    exit(0);
	    }
	  else
	    {
	    while( TRUE )
		{
		if( cmdfile!=NULL )
		  then
		    if( (i=ioinc(cmdfile)) != IOEOF )
		      then ttyout( rem, i );
		      else
			{
			ttyflush(rem);
			ioclose( cmdfile );
			cmdfile = NULL;
			}
		  else
		    if( (i=ttyin(con)) >= 0 )
		      then
			if( !escaped )
			  then
			    if( i != escapechar )
			      then ttyfout( rem, i );
			      else escaped = TRUE;
			  else
			    {
			    escaped = FALSE;
			    if( i == escapechar )
			      then ttyfout( rem, i );
			      else
				if( ttycommand( con, i ) )
				  then
				    {
				    kill( thekid, CONSIG );
				    wait(0);
				    break;
				    }
			    }
		}
	    }
	}
    if( cmdfile != NULL ) then ioclose( cmdfile );
    if( logfile != NULL ) then ioclose( logfile );
    ttyrestore( con );
    ttyrestore( rem );
    ttyformat(con,"\n[Done]\n");
    }

/**************************************************************************/
function int ttyexpect( ttyptr tty, char *toput, char *toget )
/**************************************************************************/
    {
    int i, k, timer, len;
    int c;

    len = strlen( toget );

    for( k=0; k<4; k++ )
	{
	ttyfsout( tty, toput );
	sleep(1);
	for( i=timer=0; timer<10; timer++ )
	    if( (c=ttyinnowait(tty)) < 0 || (c&0177)!=toget[i] )
	      then i=0;
	      else
		if( ++i == len )
		  then return TRUE;
	}
    return FALSE;
    }
