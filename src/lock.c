#include <unistd.h>
#include <local_iosubs.h>
#include <local_string.h>
#include <local_format.h>
#include <local_mem.h>
#include <local_error.h>
#include <local_lock.h>
#include <fcntl.h>

extern char *sys_errlist[];
extern int errno;

function int lockresource( char *lockspec, char *resource )
    {
    char lrbuf[6], *baseres, *tname, *lname;
    int16 pid;
    int fd;

    pid = getpid();

    if( (baseres = rindex( resource, '/' ) ) == NULL )
      then baseres = resource;
      else baseres++;

    sformat( lrbuf, "~{iz4b16}", pid );
    tname = mformat( lockspec, lrbuf );
    
    if( (fd=creat(tname,0666)) < 0				||
	write( fd, &pid, sizeof(pid) ) < sizeof(pid)		||
	close( fd ) < 0						)
      then
	{
	free( tname );
	eformat("Unable to create {s}:  {s}.\n",tname,sys_errlist[errno]);
	if( fd >= 0 ) then close( fd );
	return FALSE;
	}
    
    lname = mformat( lockspec, baseres );

    if( link( tname, lname ) < 0 )
      then
	{
	eformat("Unable to link {s} to {s}:  {s}.\n",
	    tname,lname,sys_errlist[errno]);
	if( (fd=open( lname, 0 )) < 0			||
	    read( fd, &pid, sizeof(pid) ) < sizeof(pid)	||
	    close( fd ) < 0				)
	  then
	    {
	    eformat("{s} in use (Unable to read {s}:  {s}).\n",
		resource,lname,sys_errlist[errno]);
	    if( fd >= 0 ) then close( fd );
	    }
	  else
	    {
	    eformat("{s} in use (already owned by pid {i}).\n",
		resource, pid );
	    }
	unlink( tname );
	free( tname );
	free( lname );
	return FALSE;
	}

    if( unlink( tname ) < 0 )
      then
	{
	eformat("Unable to unlink {s}:  {s}.\n",tname,sys_errlist[errno]);
	unlink( lname );
	free( tname );
	free( lname );
	return FALSE;
	}

    free( tname );
    free( lname );
    return TRUE;
    }

function int unlockresource( char *lockspec, char *resource )
    {
    char *baseres, *lname;
    int16 mypid, checkpid;
    int fd;

    mypid = getpid();

    if( (baseres = rindex( resource, '/' ) ) == NULL )
      then baseres = resource;
      else baseres++;

    lname = mformat( lockspec, baseres );
    if( (fd=open( lname, 0 )) < 0					||
	read( fd, &checkpid, sizeof(checkpid) ) < sizeof(checkpid)	||
	close( fd ) < 0							)
      then
	{
	eformat("Unable to read {s}:  {s}.\n",lname,sys_errlist[errno]);
	if( fd > 0 ) then close( fd );
	free( lname );
	return FALSE;
	}

    if( mypid != checkpid )
      then
	{
	eformat("Unlock failed, {s} owned by pid {i}.\n",resource,checkpid);
	free( lname );
	return FALSE;
	}

    if( unlink(lname) < 0 )
      then
	{
	eformat("Unable to unlink {s}:  {s}.\n",lname,sys_errlist[errno]);
	free( lname );
	return FALSE;
	}
    free( lname );
    return TRUE;
    }
