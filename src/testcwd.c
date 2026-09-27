#include <stdio.h>

/************************************************************************/
/************************************************************************/
main( int argc, char *argv[] )
    {
    char *strcwd();

    if( argc > 1 )
	if( argc > 2 )
	    {
	    fprintf(stderr,"Usage:  %s [dir]\n",argv[0]);
	    exit(1);
	    }
	else
	    if( chdir( argv[1] ) < 0 )
		{
		perror( argv[1] );
		exit(1);
		}
    printf( "%s\n", strcwd() );
    exit(0);
    }
