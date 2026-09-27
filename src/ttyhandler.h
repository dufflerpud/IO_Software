#include <local_iosubs.h>
#include <local_capparse.h>

struct ttystruct
    {
    char				*ts_name;
    IOFILE				ts_ioin, ts_ioout;
    struct c_pentry			*ts_pentry;
    char				ts_curstate;
#ifdef IN_TTYHANDLER
    stateptr				ts_oldstate;
    stateptr				ts_altstate;
#else
    char				*ts_oldstate;
    char				*ts_altstate;
#endif
    };

typedef struct ttystruct *ttyptr;

#ifndef IN_TTYHANDLER
extern subroutine ttyfree( ttyptr tty );
extern subroutine ttyclose( ttyptr tty );
extern subroutine ttyrestore( ttyptr tty );
extern subroutine ttyformat( ttyptr tty, char *fmt, int args );
extern subroutine ttyflformat( ttyptr tty, char *fmt, int args );
extern subroutine ttysout( ttyptr tty, char *s );
extern subroutine ttyhelp(ttyptr tty);
extern subroutine ttyconnect( ttyptr v_con, ttyptr v_rem );
extern function int ttybinary( ttyptr tty );
extern function int ttysetstate( ttyptr tty, stateptr newstate );
extern function stateptr ttycopystate( stateptr old );
extern function stateptr ttyoldstate( ttyptr tty );
extern function stateptr ttycbcstate( stateptr res );
extern function stateptr ttybinstate( stateptr res );
extern function int ttybaud( ttyptr tty, int ib );
extern function ttyptr ttymake( IOFILE ioin, IOFILE ioout, char *name );
extern function int ttylock( char *resource );
extern function int ttyunlock( char *resource );
extern function ttyptr ttyopen( char *resource );
extern function ttyptr ttyport( char *portname );
extern static function int ttynuminbuf( ttyptr tty );
extern function int ttyread( ttyptr tty, char *buf, int sizebuf );
extern function int ttyreadnowait( ttyptr tty, char *buf, int sizebuf );
extern function int ttyin( ttyptr tty );
extern function int ttyinnowait( ttyptr tty );
extern function int ttycommand( ttyptr tty, char com );
extern function int ttyexpect( ttyptr tty, char *toput, char *toget );
#endif

#define TTY_BINMODE			_B0
#define TTY_NOTTTY			_B1

#define ttyreadnowait			ttynwread
#define ttyclearbuf(tty)		ioseek((tty)->ts_ioin,0L,0)
#define ttyflush(tty)			ioflush((tty)->ts_ioout)
#define ttywrite(tty,buf,num)		iowrite((tty)->ts_ioout,(buf),(num))
#define ttyfwrite(tty,buf,num)		(ttywrite((tty),(buf),(num)), ttyflush(tty))
#define ttyout(tty,c)			iooutc((tty)->ts_ioout,(c))
#define ttyfout(tty,c)			(ttyout((tty),(c)),ttyflush(tty))
#define ttyfsout(tty,s)			(ttysout((tty),(s)),ttyflush(tty))
#define ttyxformat(tty,fmt,args)	fxformat((tty)->ts_ioout,(fmt),(args))
#define ttyflxformat(tty,fmt,args)	fflxformat((tty)->ts_ioout,(fmt),(args))
