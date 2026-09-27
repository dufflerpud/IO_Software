/***************************************************************************
	getcwd.c:  Get current working directory
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#include <local.h>

#if HANDLER_getcwd == LOCAL_HANDLER
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/* #define DEBUG */

#ifndef BIGNAMESIZ
#define BIGNAMESIZ		1024
#endif

#define ROOTINODE		4

char *string();

/************************************************************************/
/************************************************************************/
char *strcwd()
    {
    DIR *opendir(), *dirfile;
    struct dirent *readdir(), *dp;

    struct stat rootstat, *statp;
    int i, len, nlen, dir, numdirs, foundflag;
    char *curdir;

    if( stat( "/", &rootstat ) < 0 ) then return NULL;
    statp = (struct stat *)malloc(1);
    curdir = malloc(2);
    curdir[0] = '.';
    curdir[1] = 0;
    len = 1;
    numdirs = 0;
    dirfile = NULL;
    while( TRUE )
	{
	if( numdirs == 0 )
	  then statp = (struct stat *)malloc(sizeof(struct stat));
	  else statp = (struct stat *)realloc(statp,
	       (numdirs+1)*sizeof(struct stat));
	if( stat(curdir,&statp[numdirs]) < 0 ) then goto cwdfail;
	if( statp[numdirs].st_ino == rootstat.st_ino	&&
	    statp[numdirs].st_dev == rootstat.st_dev	) then break;
	curdir = realloc( curdir, len+4 );
	curdir[len++] = '/';
	curdir[len++] = '.';
	curdir[len++] = '.';
	curdir[len] = 0;
	numdirs++;
	}
    strcpy( curdir, "/" );
    len = 1;
    for( dir=numdirs-1; dir>=0; dir-- )
	{
#ifdef DEBUG
	printf("Dir#%d, curdir=%s.\n",dir,curdir);
#endif
	if( (dirfile=opendir(curdir)) == NULL ) then goto cwdfail;
	if( len<=1 )
	  then nlen = len;
	  else
	    {
	    nlen = len+1;
	    curdir = realloc( curdir, nlen+1 );
	    curdir[len] = '/';
	    curdir[nlen] = 0;
	    }
	foundflag = FALSE;
	while( (dp=readdir(dirfile)) != NULL )
	    {
	    if( dp->d_ino == 0 ) then continue;
	    if( dp->d_name[0]=='.' )
	      then
	        if( dp->d_name[1]==0 )
	          then continue;
		  else
		    if( dp->d_name[1]=='.' && dp->d_name[2]==0 )
		      then continue;
#ifdef DEBUG
	    printf("Checking %s, cino=%d, ino=%d.\n",dp->d_name,statp[dir].st_ino,dp->d_ino);
#endif
	    if( statp[dir].st_ino>ROOTINODE && statp[dir].st_ino!=dp->d_ino )
	      then continue;
	    foundflag = TRUE;
	    curdir = realloc( curdir, nlen+BIGNAMESIZ+2 );
	    for( i=0; i<BIGNAMESIZ && dp->d_name[i]; i++ )
		curdir[nlen+i]=dp->d_name[i];
	    curdir[nlen+i] = 0;
	    if( statp[dir].st_ino <= ROOTINODE )
	      then
		{
		if( stat(curdir,&rootstat) < 0 )
		  then goto cwdfail;
		  else
		    if( rootstat.st_ino!=statp[dir].st_ino	||
			rootstat.st_dev!=statp[dir].st_dev	)
		      then continue;
		}
	    len = nlen + i;
	    break;
	    }
	if( !foundflag )
	    {
	    fprintf(stderr,
    "File system inconsistancy:  strcwd can't find inode %d in %s.\n",
    statp[dir].st_ino,curdir);
	    goto cwdfail;
	    }
	closedir( dirfile );
	}
    free( statp );
    return realloc( curdir, len+1 );

cwdfail:
    if( dirfile != NULL  ) then closedir( dirfile );
    free( statp );
    free( curdir );
    return NULL;
    }

/************************************************************************/
/************************************************************************/
char *getcwd( char *bufp, size_t size )
    {
    char *res = strcwd();
    if( res && bufp )
        {
	if( size )
	  then strncpy( bufp, res, size );
	  else strcpy( bufp, res );
	free( res );
	res = bufp;
	}

    return res;
    }

/************************************************************************/
/*	Not much to setup.						*/
/************************************************************************/
subroutine setup_getcwd()
    {
    }
#endif
