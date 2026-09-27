/***************************************************************************
	sleep.c:  Routines to delay execution
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_sleep == LOCAL_HANDLER

#ifdef STANDALONE

/************************************************************************/
/************************************************************************/
function unsigned int sleep( int seconds )
    {
    long time1, time2;
    time( &time1 );
    do  {
	time( &time2 );
	} while( time2 == time1 );
    do  {
	time( &time1 );
	} while( time1 < time2+seconds );
    return 0;
    }

#endif

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_sleep()
    {
    }
#endif
