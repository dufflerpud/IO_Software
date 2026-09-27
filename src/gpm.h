/************************************************************************
 *
 *indx#	gpm.h - (VERY brief explanation of what this file is/does)
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
 *doc#	(Less brief explanation of what this file is/does)
 ************************************************************************/
/***************************************************************************
	gpm.h:  Definitions used by iosubs
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell
***************************************************************************/

#ifndef DONE_GPM

#include <local.h>

#define then
#define subroutine void
#define function

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

#ifdef BYTESWAPPED
extern int16		swapbytes();
#define BSW(x)		(swapbytes(x))
#else
#define BSW(x)		(x)
#endif

#ifndef _B0
#define _B0		0x00000001
#define _B1		0x00000002
#define _B2		0x00000004
#define _B3		0x00000008
#define _B4		0x00000010
#define _B5		0x00000020
#define _B6		0x00000040
#define _B7		0x00000080
#define _B8		0x00000100
#define _B9		0x00000200
#define _B10		0x00000400
#define _B11		0x00000800
#define _B12		0x00001000
#define _B13		0x00002000
#define _B14		0x00004000
#define _B15		0x00008000
#define _B16		0x00010000
#define _B17		0x00020000
#define _B18		0x00040000
#define _B19		0x00080000
#define _B20		0x00100000
#define _B21		0x00200000
#define _B22		0x00400000
#define _B23		0x00800000
#define _B24		0x01000000
#define _B25		0x02000000
#define _B26		0x04000000
#define _B27		0x08000000
#define _B28		0x10000000
#define _B29		0x20000000
#define _B30		0x40000000
#define _B31		0x80000000
#endif

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
#endif

#ifdef USE_STDIO
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
#define IOEOF		EOF
typedef FILE		*IOFILE;
#endif

#define DONE_GPM
#endif
