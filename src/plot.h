/************************************************************************
 *
 *indx#	plot.h - Include file for plot routines
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
 *doc#	Include file for plot routines
 ************************************************************************/
/***************************************************************************
	plot.h:  Macros for plotting
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#define DEBUG_PLOT	0
#define HP_A		1
#define HP_B		2
#define HP_A_AF		3
#define HP_B_AF		4
#define REGIS		5
#define COMPRESSED	6
#define UNIXPLOT	7
#define SCREEN		8
#define LA100		9
#define LA50		10
#define LN03		11
#define PRINTER		12

#ifdef _INPLOT
char *plottypes[] =
    {
    "Debug",
    "HP size A, no auto-feed",
    "HP size B, no auto-feed",
    "HP size A, auto-feed",
    "HP size B, auto-feed",
    "Regis",
    "Compressed",
    "Unix plot",
    "Screen",
    "LA100",
    "LA50",
    "LN03",
    "Dumb printer",
    NULL
    };

int plotnumcolors[] =
    {
    9,
    6,
    6,
    8,
    8,
    3,
    4,
    4,
    3,
    1,
    1,
    1,
    9
    };

#define _COMPS		COMPRESSED:	case UNIXPLOT

#define _HPNAFS		HP_A:		case HP_B
#define _HPAFS		HP_A_AF:	case HP_B_AF
#define _HPSIZEA	HP_A:		case HP_A_AF
#define _HPSIZEB	HP_B:		case HP_B_AF
#define _HPPROTS	_HPSIZEA:	case _HPSIZEB

#define _SIXELS		LA100:		case LA50:	case LN03
#define _MATRIXS	_SIXELS:	case PRINTER

#else
extern char *plottypes[];
#endif

extern subroutine outpnt( int x, int y );
extern subroutine pinit(int itype,IOFILE ifile,int xm,int ym);
extern subroutine penter();
extern subroutine pexit();
extern subroutine pclear();
extern subroutine pcoord( double *xnew, double *ynew );
extern subroutine pcolornow( int icolor );
extern subroutine pcolor( int icolor );
extern subroutine pmovenow( double newx, double newy );
extern subroutine pmove( double xnew, double ynew );
extern subroutine matrixdraw( double xnew, double ynew );
extern subroutine screendraw( double xnew, double ynew );
extern subroutine pdraw( double xnew, double ynew );
extern function int function ppixel(int x,int y);
extern subroutine pplot();
extern subroutine pchar( char achar, double xc1, double yc1, double height, double dangle );
extern subroutine pstr( char *achars, double xchar, double ychar, double height, double dangle );
extern subroutine pformat(double xchar,double ychar,double height,double dangle,char *fmt,...);
extern subroutine pcxy( char *ic, double *gx, double *gy );
