/************************************************************************
 *
 *indx#	local_iosubs.h - Include file for standalone i/o routines
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
 *doc#	Include file for standalone i/o routines
 ************************************************************************/
#ifndef LOCAL_IOSUBS_DEFINED
#define LOCAL_IOSUBS_DEFINED
#include <local.h>
#include <stdio.h>

#define IOSUCCESS	0
#define IOERROR		(-1)

#ifdef STANDALONE

#define BUFSIZE		512

#define IOEOF		(-1)
#define IO_MALBLK	_B0
#define IO_MALBUF	_B1
#define IO_WRITE	_B7

#define IOINC(p)	( (p)->io_ind >= (p)->io_nread ||		\
				(p)->io_pbufsize != 0 			\
			    ? ioinc(p)					\
			    : (p)->io_buf[ (p)->io_ind++ ] &0xff	)

#define IOOUTC(p,c)	( (p)->io_ind >= (p)->io_bufsize-1 		\
			    ? iooutc((p),(c))				\
			    : (p)->io_buf[ (p)->io_ind++ ] = (c)	)

#define IOCHAN(p)	( (p) -> io_file )

struct iostruct
    {
    int			(*io_func)();
    char		io_status;
    int			io_file;
    char		*io_buf;
    unsigned int	io_bufsize;
    unsigned int	io_ind;
    unsigned int	io_nread;
    char		*io_pbuf;
    unsigned int	io_pbufsize;
    };

typedef struct iostruct	*IOFILE;
typedef struct iostruct	iostr;

extern function int ioinc( IOFILE p );
extern function int ioflush( IOFILE p );
extern function int iooutc( IOFILE p, char c );
extern function long ioseek( IOFILE p, long offset, int where );
extern subroutine iobackc( IOFILE p, char c );
extern function IOFILE ioassociate( int chan, int (*func)(), int status );
extern function int iodisassociate( IOFILE p );
extern function int canseek( int chan );
extern function int iocanseek( IOFILE iof );
extern function int makeseek( int chan );
extern function IOFILE iomakeseek( IOFILE iof );
extern function int ioclose( IOFILE p );
extern function int iooutw( IOFILE p, int w );
extern function int ioinw( IOFILE p );
extern function int iowrite( IOFILE p, char *buf, int sizebuf );
extern function int ioread( IOFILE p, char *buf, int sizebuf );
extern function int iogets( IOFILE p, char *buf, int sizebuf );
extern function int miogets( IOFILE p, char **s );
extern function int iogetl( IOFILE p, char *buf, int sizebuf );
extern function int miogetl( IOFILE p, char **s );
extern function int ioputs( IOFILE p, char *s );
extern function int ioputl( IOFILE p, char *s );
extern subroutine setup_iosubs();

#else

#define IOCHAN(s)	fileno(s)
#define iooutc(s,c)	(putc((c),(s)),0)
#define IOOUTC(s,c)	iooutc((c),(s))
#define ioinc(s)	getc(s)
#define IOINC(s)	ioinc(s)
#define ioflush(s)	fflush(s)
#define ioopen(f,m)	fopen((f),(m))
#define iopipe(f,m)	popen((f),(m))
#define ioclose(s)	fclose(s)
#define ioseek(s,l,w)	fseek((s),(l),(w))
#define iobackc(s,c)	ungetc((c),(s))
#define ioread(f,b,c)	fread(b,1,c,f)
#define iowrite(f,b,c)	fwrite(b,1,c,f)
#define iogetl(f,b,c)	fgets(b,c,f)
#define iogets(f,b,c)	fgets(b,c,f)
#define ioputl(f,b)	fputs(b,f)
#define ioputs(f,b)	fputs(b,f)
#define IOEOF		EOF
typedef FILE		*IOFILE;
#endif

#endif
