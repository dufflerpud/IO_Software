#include <local_iosubs.h>
#include <local_format.h>
#include <local_time.h>
#include <local_string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

#define LINKLOCK	"/etc/passwd"
#define LOCKFILE	"/tmp/event-{i}"

subroutine logevent( char *logname, char *fmt, ... )
    {
    char lockname[100];
    IOFILE logfile;
    long curtime;
    va_list ap;

    sformat( lockname, LOCKFILE, getpid() );
    while( link(LINKLOCK,lockname) < 1 ) sleep(1);
    if( (logfile=ioopen(logname,"a")) == NULL )
      then
	{
	perror(logname);
	unlink(lockname);
	return;
	}
    time( &curtime );
    fformat(logfile,"{sm24}: {sl8}({iz3}):  ",
	ctime(&curtime),getlogin(),geteuid());
    va_start( ap, fmt );
    fxformat(logfile,fmt,ap);
    va_end( ap );
    fformat(logfile,"\n");
    ioclose(logfile);
    if( unlink( lockname ) < 0 ) then { perror(lockname); exit(1); }
    }

function int permission( char *filename, char *fmt, ... )
    {
    char action[100], name[100], request[100];
    IOFILE pfile;
    struct passwd *pw;
    int match;
    va_list ap;

    if( (pw=getpwuid(getuid())) == NULL ) then return FALSE;
    va_start( ap, fmt );
    sxformat( request, fmt, ap );
    va_end( ap );
    if( (pfile=ioopen(filename,"r")) == NULL ) then return FALSE;
    while( fscanf(pfile,"%s",action)==1 )
	{
	match = ( strcmp(action,request) == 0 );
	while( fscanf(pfile,"%s",name)==1 && strcmp(name,".")!=0 )
	    {
	    if( match && strcmp(name,pw->pw_name)==0 )
	      then
		{
		ioclose( pfile );
		return TRUE;
		}
	    }
	}
    ioclose( pfile );
    return FALSE;
    }
