/***************************************************************************
	getenv.c:  Get current environment
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_getcwd == LOCAL_HANDLER
extern char **environ;

/************************************************************************/
/************************************************************************/
char *getenv(char *s)
    {
    int i, j;
    if( environ==NULL ) then return NULL;
    for( i=0; environ[i]!=NULL; i++ )
	{
	for( j=0; environ[i][j]==s[j]; j++ )	;
	if( s[j]==0 && environ[i][j]=='=' ) then return &(environ[i][j+1]);
	}
    return NULL;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_getenv()
    {
    }
#endif
