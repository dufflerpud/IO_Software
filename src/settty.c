#include <local.h>

#if HANDLER_settty == SYSTEM_HANDLER

#include <curses.h>

#ifdef SOL
static struct tchars ot, nt;
static struct ltchars olt, nlt;
static int setup = 0;
#endif

/************************************************************************/
/************************************************************************/
subroutine setkluge()
    {
#ifndef SOL
    crmode();
#else
    int i;

    if( !setup )
      then
	{
	ioctl( 0, TIOCGETC, &ot );
	movebytes( (char *)&nt, (char *)&ot, sizeof(ot) );
	nt.t_intrc = nt.t_quitc = nt.t_brkc = -1;
	ioctl( 0, TIOCGLTC, &olt );
	movebytes( (char *)&nlt, (char *)&olt, sizeof(olt) );
	nlt.t_suspc = nlt.t_dsuspc = nlt.t_flushc = nlt.t_lnextc = -1;
	setup = 1;
	}
    ioctl( 0, TIOCSETC, &nt );
    ioctl( 0, TIOCSLTC, &nlt );
#endif
    }

/************************************************************************/
/************************************************************************/
subroutine resetkluge()
    {
#ifdef SOL
    ioctl( 0, TIOCSETC, &ot );
    ioctl( 0, TIOCSLTC, &olt );
#endif
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_settty()
    {
    }
#endif
