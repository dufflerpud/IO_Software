/************************************************************************
 *
 *indx#	plot.c - Software that knows how to plot on a small number of plotters
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
 *doc#	Software that knows how to plot on a small number of plotters
 ************************************************************************/
/***************************************************************************
	plot.c:  Routines to plot on a variety of devices
	%Z% %M% %I% %G%
	Created by Christopher M. Caldwell of IO Software, Inc.
***************************************************************************/

#ifdef CURSES
#include "curses.h"
tstp(){}
#endif

#include <local_iosubs.h>
#include <local_format.h>
#include <local_parse.h>
#include <math.h>
#include <stdlib.h>

#define _INPLOT
#include <plot.h>

/*
#define TEMPBUFSIZE	0x7f00
#define PLTFORMAT	"/tmp/PLT.{i}"
*/

#ifdef CHECKSTOP
extern int checkstop();
#else
#define checkstop()	FALSE
#endif

static double nextx,nexty, curx,cury, xchar,ychar;
static char ptype, newcolor, oldcolor;
static double clpmnx, clpmxx, clpmny, clpmxy;
static double tclpmnx, tclpmxx, tclpmny, tclpmxy;
static double intercept;
static int drawn;
static IOFILE fptr;
static int xmatrix, ymatrix, bpb, ymatbpb, matrixsize;
static int penup;

#ifdef TEMPBUFSIZE
static long curpos;
static int numinbuf;
static int tfile;
static char tempname[100];
static char dirtybuf;
#endif
static char *buf;

#define COMMAXX		32767
#define COMMAXY		32767

/**************************************************************************/

static subroutine initmatrix()
    {
#ifdef TEMPBUFSIZE
    sformat( tempname, PLTFORMAT, getpid() );
    if( (tfile=creat(tempname,0600)) < 0 ) then filerr("creat {s}",tempname);
    close( tfile );
    if( (tfile=open(tempname,2)) < 0 ) then filerr("open {s}",tempname);
    curpos = 0L;
    numinbuf = 0;
    dirtybuf = FALSE;
    if( (buf = malloc( TEMPBUFSIZE )) == NULL )
      then { fformat(stderr,"Cannot allocate enough memory.\n"); exit(1); }

#else
    int i;

    if( (buf = malloc( matrixsize )) == NULL )
	{ fformat(stderr,"Cannot allocate enough memory.\n"); exit(1); }
    for( i=0; i<matrixsize; i++ ) buf[i] = 0;
#endif
    drawn = TRUE;
    }

/**************************************************************************/

#ifdef TEMPBUFSIZE
static subroutine shiftbuf( long cp )
    {
    if( dirtybuf && numinbuf > 0 )
      then
	{
	if( lseek(tfile,curpos,0) < 0 ) then filerr("lseek {s}",tempname);
	write(tfile,buf,numinbuf);
	dirtybuf = FALSE;
	}
    curpos = (cp/TEMPBUFSIZE)*TEMPBUFSIZE;
    if( lseek(tfile,curpos,0) < 0 ) then filerr("lseek {s}",tempname);
    if( (numinbuf = read( tfile, buf, TEMPBUFSIZE )) < 0 ) then numinbuf = 0;
    while( numinbuf < TEMPBUFSIZE ) buf[ numinbuf++ ] = 0;
    }
#endif

/**************************************************************************/

static subroutine settable( int x, int y, int v )
    {
    long cp;
    if( !drawn ) then initmatrix();
    cp = (long)y*xmatrix + x;
#ifdef TEMPBUFSIZE
    if( cp < curpos || cp >= curpos+numinbuf ) then shiftbuf( cp );
    dirtybuf = TRUE;
    buf[ cp-curpos ] = v;
#else
    buf[ cp ] = v;
#endif
    }

/**************************************************************************/

static char gettable( int x, int y )
    {
    long cp;
    if( !drawn ) then initmatrix();
    cp = (long)y*xmatrix + x;
#ifdef TEMPBUFSIZE
    if( cp < curpos || cp >= curpos+numinbuf ) then shiftbuf( cp );
    return buf[ cp-curpos ];
#else
    return buf[ cp ];
#endif
    }

subroutine outpnt( int x, int y )
    {
    iooutc( fptr, (x & 0xff) );
    iooutc( fptr, ((x>>8) & 0xff) );
    iooutc( fptr, (y & 0xff) );
    iooutc( fptr, ((y>>8) & 0xff) );
    }

subroutine pinit(int itype,IOFILE ifile,int xm,int ym)
/*
    This routine is called for each new plotter.  It sets the plotter
    type and sets the appropriate file pointer.
*/
    {
    fptr = ifile;
    ptype = itype;
    curx = cury = -1.0;
    nextx = nexty = 0.0;
    drawn = FALSE;
    oldcolor = 0;
    newcolor = 1;
    penup = TRUE;
    switch( ptype )
	{
	case DEBUG_PLOT:fformat(fptr,"%PSI:  Init plotter.\n");		break;
#ifdef CURSES
	case SCREEN:	initscr(); clear();				break;
#endif
	case UNIXPLOT:	iooutc(fptr,'s');
			outpnt( 0, 0 );
			outpnt( COMMAXX, COMMAXY );
			break;
        case REGIS:	fformat(fptr,"\033[;H\033[2J");			break;
	case LA100:	bpb=6;	xmatrix=1000;	ymatrix=500;		break;
	case LA50:	bpb=6;	xmatrix=500;	ymatrix=500;		break;
	case LN03:	bpb=6;	xmatrix=1000;	ymatrix=1000;		break;
	case PRINTER:	bpb=8;	xmatrix=xm;	ymatrix=ym;		break;
	}
    switch( ptype )
	{
	case _MATRIXS:	ymatbpb = ymatrix / bpb;
			if( ymatrix % bpb ) then ymatbpb++;
			matrixsize = xmatrix * ymatbpb;
			break;
	}
    return;
    }

/**************************************************************************/

subroutine penter()
    {
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Enter plot mode.\n");
			    break;

	case _HPSIZEA:	    fformat(fptr,"\033.N;19:");
			    fformat(fptr,"\033.I200;;17:");
			    fformat(fptr,"IN;PS4;SC0,13865,0,10000;\n");
			    break;

	case _HPSIZEB:	    fformat(fptr,"\033.N;19:");
			    fformat(fptr,"\033.I200;;17:");
			    fformat(fptr,"IN;PS3;SC0,15264,0,10000;\n");
			    break;

	case REGIS:	    fformat(fptr,"\033P1p");
			    break;
	
#ifdef CURSES
	case SCREEN:	    crmode();
			    break;
#endif
	}
    }

/**************************************************************************/

subroutine pexit()
    {
    pcolornow( 0 );
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Exit plot mode.\n");	break;
	case _HPSIZEA:	    if( !penup ) then fformat(fptr,"PU;");
			    penup = TRUE;
			    fformat(fptr,"sp0;pa13000,10000;"); 
			    break;
	case _HPSIZEB:	    if( !penup ) then fformat(fptr,"PU;");
			    penup = TRUE;
			    fformat(fptr,"sp0;pa0,10000;");	
			    break;
	case REGIS:	    fformat(fptr,"\033\\");			break;
#ifdef CURSES
	case SCREEN:	    pmovenow(0.0,1.0);
			    refresh();
			    nocrmode();
			    break;
#endif
	}
    ioflush(fptr);
    curx = cury = -1;
    }

/**************************************************************************/

subroutine pclear()
    {
    int x, y;
    pmove( 0.0, 0.0 );
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Clear screen.\n");	break;
	case _HPAFS:	    if( !penup ) then fformat(fptr,"PU;");
			    penup = TRUE;
			    fformat(fptr,"AF;");			break;
	case REGIS:	    fformat(fptr,"s(i0,e)");			break;
#ifdef CURSES
	case SCREEN:	    clear();					break;
#endif
	case _COMPS:        iooutc(fptr,'e');				break;
	case _MATRIXS:	    for( y=0; y<ymatbpb; y++ )
				for( x=0; x<xmatrix; x++ )
				    settable( x, y, 0 );
			    fformat(fptr,"\014");
			    break;
	}
    }

/**************************************************************************/

subroutine pcoord( double *xnew, double *ynew )
/* This routine returns the current coordinates. */
    {
    *xnew = nextx;
    *ynew = nexty;
    return;
    }

char *linetypes[]=
    {
    "none",
    "solid",
    "dashed",
    "dotted",
    "longdashed",
    "shortdashed",
    "dot-dashed",
    NULL
    };

subroutine pcolornow( int icolor )
    {
    icolor = ( icolor<=0 ? 0 : (icolor-1)%plotnumcolors[ptype] + 1 );
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Set color to {i}.\n",icolor);
			    break;

	case _HPPROTS:	    if( !penup ) then fformat(fptr,"PU;");
			    fformat(fptr,"SP{i};\n",icolor);
			    break;

	case UNIXPLOT:	    fformat(fptr,"f{s}\n", linetypes[icolor] );
			    break;

	case COMPRESSED:    iooutc(fptr,'h');
			    iooutc(fptr,icolor);
			    break;

	case REGIS:	    fformat(fptr,"w(i{i})",icolor);
			    break;
	}
    oldcolor = icolor;
    penup = TRUE;
    }

/**************************************************************************/

subroutine pcolor( int icolor )
    {
    icolor = ( icolor<=0 ? 0 : (icolor-1)%plotnumcolors[ptype] + 1 );
    newcolor = icolor;
    return;
    }

/**************************************************************************/

subroutine pmovenow( double newx, double newy )
    {
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(stderr,"%PSI:  Move to ({f.6},{f.6}).\n",
				newx,newy);
			    break;

	case _HPPROTS:      if( !penup ) then fformat(fptr,"PU;");
			    penup = TRUE;
			    fformat(fptr,"PA{i},{i};\n",
				(int)(newx*10000.0),(int)(newy*10000.0));
			    break;

	case _COMPS:        iooutc(fptr,'m');
			    outpnt((int)(newx*COMMAXX),(int)(newy*COMMAXY));
			    break;

	case REGIS:	    fformat(fptr,"p[{i},{i}]",
				(int)(0.5+newx*480.0),
				(int)(479.5-newy*479.0) );
			    break;

#ifdef CURSES
	case SCREEN:	    move( (int)(23.5-23.0*newy), (int)(49.0*newx+0.5) );
			    break;
#endif
	}
    curx = newx;
    cury = newy;
    }

/**************************************************************************/

subroutine pmove( double xnew, double ynew )
/* This routine sets the current position in a 1.0 x 1.0 grid. */
    {
    nextx = xnew;
    nexty = ynew;
    switch( ptype )
	{
	case _HPPROTS:	if( !penup )
			  then
			    {
			    fformat(fptr,"PU;");
			    ioflush(fptr);
			    penup = TRUE;
			    }
	}
    }

subroutine matrixdraw( double xnew, double ynew )
    {
    int ixc, inxc, idx, ixp;
    int iyc, inyc, idy, iyp, iyh, iyl;
    int intrvl;

    ixc = (xmatrix-1)*nextx;	iyc = (ymatrix-1)*nexty;
    inxc = (xmatrix-1)*xnew;	inyc = (ymatrix-1)*ynew;
    idx = inxc-ixc;		idy = inyc-iyc;
    if( idx == 0 && idy == 0 ) return;
    if( (idx>=0?idx:-idx) >= (idy>=0?idy:-idy) )
      then
	{
	if( idx >= 0 ) then intrvl = 1; else intrvl = -1;
	for( ixp=ixc; ixp!=inxc+intrvl; ixp+=intrvl )
	    {
	    if( ixp < 0 || ixp >= xmatrix ) then continue;
	    iyp = iyc + idy*(ixp-ixc)/idx;
	    if( iyp < 0 || iyp >= ymatrix ) then continue;
	    iyh = iyp/bpb;
	    iyl = iyp%bpb;
	    settable( ixp, iyh, gettable( ixp, iyh ) | (1<<(bpb-1-iyl)) );
	    }
	}
      else
	{
	if( idy >= 0 ) then intrvl = 1; else intrvl = -1;
	for( iyp=iyc; iyp!=inyc+intrvl; iyp+=intrvl )
	    {
	    if( iyp < 0 || iyp >= ymatrix ) then continue;
	    ixp = ixc + idx*(iyp-iyc)/idy;
	    if( ixp < 0 || ixp >= xmatrix ) then continue;
	    iyh = iyp/bpb;
	    iyl = iyp%bpb;
	    settable( ixp, iyh, gettable( ixp, iyh ) | (1<<(bpb-1-iyl)) );
	    }
	}
    }

/**************************************************************************/

#ifdef CURSES
subroutine screendraw( double xnew, double ynew )
    {
    int ixc, ixn, ixd, iaxd, ixi;
    int iyc, iyn, iyd, iayd, iyi;
    int xaxis, ie;

    ixc = 49.0*nextx + 0.5;	ixn = 49.0*xnew + 0.5;
    iyc = 23.5 - 23.0*nexty;	iyn = 23.5 - 23.0*ynew;
    ixd = (ixn - ixc);
    if(ixd<0) then iaxd = -ixd; else iaxd = ixd;
    if(ixd<0) then ixi = -1; else if(ixd>0) then ixi = 1; else ixi = 0;
    iyd = (iyn - iyc);
    if(iyd<0) then iayd = -iyd; else iayd = iyd;
    if(iyd<0) then iyi = -1; else if(iyd>0) then iyi = 1; else iyi = 0;

    if( xaxis = ( iaxd > iayd ) )
      then { if( (ie = 2*iayd - iaxd) < 0 ) then ie = -ie; }
      else { if( (ie = 2*iaxd - iayd) < 0 ) then ie = -ie; }
    while( TRUE )
	{
	move( iyc, ixc );
	addch( oldcolor+'0' );
	if( xaxis )
	  then
	    {
	    if( ie <= 0 )
	      then ie += 2*iayd;
	      else { iyc += iyi; ie += (2*iayd-2*iaxd); }
	    ixc += ixi;
	    }
	  else
	    {
	    if( ie <= 0 )
	      then ie += 2*iaxd;
	      else { ixc += ixi; ie += (2*iaxd-2*iayd); }
	    iyc += iyi;
	    }
	if( ixn+ixi==ixc && ixi!=0 ) then break;
	if( iyn+iyi==iyc && iyi!=0 ) then break;
	if( ixi==0 && iyi==0 ) then break;
	}
    }
#endif

/**************************************************************************/

subroutine pdraw( double xnew, double ynew )
/* This routine draws a line from nextx,nexty to xnew,ynew. */
    {
    if( xnew==nextx && ynew==nexty ) then return;
    if( curx!=nextx || cury!=nexty ) then pmovenow( nextx, nexty );
    if( oldcolor!=newcolor ) then pcolornow( newcolor );
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Draw to ({f.6},{f.6}).\n",
				xnew,ynew);
			    break;

	case _HPPROTS:	    if( penup ) then fformat(fptr,"PD;");
			    penup = FALSE;
			    fformat(fptr,"PA{i},{i};\n",
				(int)(xnew*10000.0),(int)(ynew*10000.0));
			    break;

	case _COMPS:        iooutc(fptr,'n');
			    outpnt((int)(xnew*COMMAXX),(int)(ynew*COMMAXY));
			    break;

	case REGIS:	    fformat(fptr,"v[{i},{i}]",
				(int)(0.5+xnew*480.0),
				(int)(479.5-ynew*479.0) );
			    break;

#ifdef CURSES
	case SCREEN:	    screendraw(xnew,ynew);	break;
#endif

	case _MATRIXS:	    matrixdraw(xnew,ynew);	break;
	}
    curx = nextx = xnew;
    cury = nexty = ynew;
    drawn = TRUE;
    return;
    }

static subroutine psetclip( double sminx, double smaxx, double sminy, double smaxy )
    {
    double tolx, toly;
    clpmnx = sminx;
    clpmxx = smaxx;
    clpmny = sminy;
    clpmxy = smaxy;
    tolx = (clpmxx - clpmnx) / 9999.123456789;
    toly = (clpmxy - clpmny) / 9999.214365879;
    tclpmnx = clpmnx - tolx;
    tclpmxx = clpmxx + tolx;
    tclpmny = clpmny - toly;
    tclpmxy = clpmxy + toly;
    }

static function int inter( double ox, double oy, double nx, double ny, double clipx, double miny, double maxy )
    {
    if( ox == nx			||
        (ox <= clipx && nx <= clipx)	||
	(ox >= clipx && nx >= clipx)	) then return FALSE;
    intercept = ((clipx-ox) / (nx-ox)) * (ny-oy) + oy;
    if( intercept < miny || intercept > maxy ) then return FALSE;
    return TRUE;
    }

subroutine pcdraw( double nx, double ny )
/* This routine draws a clipped line from the current position to nx,ny. */
    {
    double xcd[2], ycd[2];
    int ncd, inwindow;

    inwindow = nextx > tclpmnx && nextx < tclpmxx &&
	       nexty > tclpmny && nexty < tclpmxy;

    ncd = 0;
    if( inter(nextx,nexty,nx,ny,tclpmnx,tclpmny,tclpmxy) )
      then xcd[ncd]=clpmnx,ycd[ncd]=intercept,ncd++;
    if( inter(nextx,nexty,nx,ny,tclpmxx,tclpmny,tclpmxy) )
      then xcd[ncd]=clpmxx,ycd[ncd]=intercept,ncd++;
    if( inter(nexty,nextx,ny,nx,tclpmny,tclpmnx,tclpmxx) )
      then ycd[ncd]=clpmny,xcd[ncd]=intercept,ncd++;
    if( inter(nexty,nextx,ny,nx,tclpmxy,tclpmnx,tclpmxx) )
      then ycd[ncd]=clpmxy,xcd[ncd]=intercept,ncd++;

    if( inwindow )
      then
	if( ncd==0 )
	  then pdraw(nx,ny);
	  else
	    {
	    pdraw(xcd[0],ycd[0]);
	    pmove(nx,ny);
	    }
      else
	if( ncd==0 )
	  then pmove(nx,ny);
	else if( ncd==1 )
	  then
	    {
	    pmove(xcd[0],ycd[0]);
	    pdraw(nx,ny);
	    }
	  else
	    {
	    if( (xcd[0]-nextx)*(xcd[0]-nextx) + (ycd[0]-nexty)*(ycd[0]-nexty) <
		(xcd[1]-nextx)*(xcd[1]-nextx) + (ycd[1]-nexty)*(ycd[1]-nexty) )
	      then
		{
		pmove(xcd[0],ycd[0]);
		pdraw(xcd[1],ycd[1]);
		}
	      else
		{
		pmove(xcd[1],ycd[1]);
		pdraw(xcd[0],ycd[0]);
		}
	    pmove(nx,ny);
	    }
    }

static subroutine outklg( int ichar, int icount )
    {
    int i;

    if( icount <= 0 ) then return;
    else if( icount <= 4 ) then for( i=0; i<icount; i++ ) iooutc(fptr,ichar);
    else fformat( fptr, "!{i}{c}", icount, ichar );
    return;
    }

/**************************************************************************/

int function ppixel(int x,int y)
    {
    return ( gettable(x,y/bpb) & (1<<(bpb-1-y%bpb)) ) != 0;
    }

/**************************************************************************/

subroutine pplot()
/* This routine sends the entire picture out to a LA100 like plotter.  */
    {
    int inx, iny;
    int x, y;
    int ilast, icount, icur;
    int j;
    nextx = nexty = 0.0;
    if( !drawn ) then return;
    switch( ptype )
	{
	case DEBUG_PLOT:    fformat(fptr,"%PSI:  Print plot.\n");	break;
	case _HPAFS:	    break;
	case REGIS:	    break;
#ifdef CURSES
	case SCREEN:	    refresh();
			    break;
#endif

	case _SIXELS:	    fformat( fptr, "\033P1q" );
			    for( iny=ymatbpb-1; iny>=0; iny-- )
				{
				ilast = -1;
				icount = 0;
				for( inx=0; inx<xmatrix; inx++ )
				    if( (icur=gettable(inx,iny)+077)==ilast )
				      then icount++;
				      else
					{
					outklg( ilast, icount );
					ilast = icur;
					icount = 1;
					}
				outklg( ilast, icount );
				iooutc(fptr,'-');
				}
			    fformat( fptr, "\033\\" );
			    break;


	case PRINTER:	    for( iny=ymatrix-1; iny>=0; iny-- )
				{
				for( inx=0; inx<xmatrix; inx++ )
				    if( ppixel(inx,iny) )
				      then iooutc(fptr,'#');
				      else iooutc(fptr,' ');
				iooutc(fptr,'\n');
				}
			    break;
	}
    switch( ptype )
	{
	case _MATRIXS:
			    drawn = FALSE;
#ifdef TEMPBUFSIZE
			    close( tfile );
			    unlink( tempname );
#endif
			    free( buf );
			    buf = NULL;
	}
    }

char *lettercodes[] =
    {
    NULL,
    "0400404404m22",
    "24140301103041433424m22",
    "24014124m22",
    "2420m0242m22",
    "0440m0044m22",
    "2402204224m22",
    "2024024224m22",
    "00440440m22",
    "123222330333003000m22",
    "0422442220m22",
    "443313041311001131403133m22",
    "2420220242220440220044m22",
    "0444004004m22",
    "2420m22",
    "24014124m03204303m22",
    "2262m22",
    "2226m22",
    "1425342314m383353m62023862m35",
    "00606101026263030464650506666707086860505848403038282010180800m34",
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
    "2223m2428",
    "18081718m37283837",
    "0646m3733m1713m0444",
    "04132322233344351506172728273746",
    "2718071627m4802m3223344332",
    "44120304262718070642",
    "27182827",
    "483827233242",
    "081827231202",
    "4703m2228m0743",
    "0545m2723",
    "213233232232",
    "0545",
    "3222233332",
    "0248",
    "12030718384743321238",
    "1728221232",
    "07183847460242",
    "07183847463515354443321203",
    "42480454",
    "031232434536060848",
    "051636454332120307183847",
    "084812",
    "15060718384746351504031232434435",
    "031232434738180705143445",
    "2423333424m3637272636",
    "213233322232m3536262535",
    "430547",
    "4606m0444",
    "074503",
    "07183847462524m2322",
    "453424152636454738180703123243",
    "42462806020444",
    "0535464738080232434435",
    "4332120307183847",
    "02324347380802",
    "42020535050848",
    "020535050848",
    "47381807031232434525",
    "020805454842",
    "183828221232",
    "0312324348",
    "020804481542",
    "080242",
    "0208264842",
    "0208444842",
    "030718384743321203",
    "02083847463505",
    "030718384743321203m2442",
    "020838474635051542",
    "031232434435150607183847",
    "22280848",
    "080312324348",
    "082248",
    "080312232523324348",
    "0248m0842",
    "0825222548",
    "08480242",
    "48282242",
    "0842",
    "08282202",
    "062846",
    "0141",
    "0826",
    "051636454332120314344342",
    "08070312324345361605",
    "4332120305163645",
    "43321203051636454842",
    "04444536160503123243",
    "1217283847m0535",
    "011030414536160503123243",
    "02080516364542",
    "2827m1626221232",
    "011030313626m3738",
    "121813462442",
    "1828221232",
    "02060516252225364542",
    "02060516364542",
    "030516364543321203",
    "000516364543321203",
    "43321203051636454050",
    "020605163645",
    "03123243341405163645",
    "2826163626233243",
    "06031232434642",
    "062246",
    "060312232423324346",
    "0246m0642",
    "06031232434641301001",
    "06460242",
    "483827261524233242",
    "2822",
    "081827263524231202",
    "07183647"
    };

subroutine pchar( char achar, double xc1, double yc1, double height, double dangle )
/* This routine plots a character. */
    {
    char mmove;
    double xchar,ychar,xnew,ynew;
    double ch, sh;
    double rangle;
    int ix, iy, ichar;
    int i = 0;

    if( xc1 == -999.0 || yc1 == -999.0 )
      then { xchar=nextx; ychar=nexty; }
      else { xchar=xc1; ychar=yc1; }

    rangle = dangle*3.14159/180.0;
    ch = height*cos(rangle);
    sh = height*sin(rangle);
    if( achar < 32 ) then { xchar-=ch/2; ychar-=sh/2; }
    mmove = TRUE;
    if( lettercodes[achar] != NULL )
      then
	while( (ichar=lettercodes[achar][i++]) != 0 )
	    if( ichar == 'm' )
	      then mmove=TRUE;
	      else
		{
		if( checkstop() ) then return;
		ix = ichar - '0';
		iy = lettercodes[achar][i++] - '0';
		xnew = xchar + (ch*ix - sh*iy)/10.0;
		ynew = ychar + (sh*ix + ch*iy)/10.0;
		if( mmove )
		  then pmove( xnew, ynew );
		  else pdraw( xnew, ynew );
		mmove = FALSE;
		}

    if( achar < 32 )
      then { xchar += ch/2.0; ychar += sh/2.0; }
      else { xchar += ch; ychar += sh; }
    pmove( xchar, ychar );
    return;
    }

/**************************************************************************/

subroutine pstr( char *achars, double xchar, double ychar, double height, double dangle )
/* This routine plots a string. */
    {
    if( xchar!=-999.0 || ychar!=-999.0 ) then pmove(xchar,ychar);
    while( !checkstop() && *achars )
	pchar( *achars++, -999.0, -999.0, height, dangle );
    }

/**************************************************************************/

subroutine pformat(double xchar,double ychar,double height,double dangle,char *fmt,...)
/* This routine is the plotting equivalent of format. */
    {
    char buf[1000];
    va_list ap;
    va_start( ap, fmt );
    sxformat( buf, fmt, ap );
    va_end( ap );
    pstr( buf, xchar, ychar, height, dangle );
    return;
    }

subroutine pcxy( char *ic, double *gx, double *gy )
    {
    char inln[100];
    int ix, iy, exitcom;

    pmovenow( *gx=nextx, *gy=nexty );
    switch( ptype )
	{
	case REGIS:	do  {
			    fformat( fptr, "r(p(i))" );
			    ioflush( fptr );
			    (void)iogetl( stdin, inln, 100 );
			    } while(sparse(inln,"{c}[{i},{i}]",ic,&ix,&iy)<3);
			*gx = ix / 480.0;
			*gy = (iy-59) / 240.0;
			break;

#ifdef CURSES
	case SCREEN:	for( exitcom=FALSE; !exitcom; )
			    {
			    pmovenow( *gx, *gy );
			    refresh();
			    switch( getch() )
				{
				case 012:
				case 015:
				case 033:	exitcom=TRUE;	break;

				case 'H':	*gx = 0.0;	break;
				case 'J':	*gy = 0.0;	break;
				case 'K':	*gy = 1.0;	break;
				case 'L':	*gx = 1.0;	break;

				case '1':	if( (*gy -= 1.0/23.0) < 0.0 )
						  then *gy = 0.0;
						if( (*gx -= 1.0/49.0) < 0.0 )
						  then *gx = 0.0;
						break;

				case 'j':
				case '2':	if( (*gy -= 1.0/23.0) < 0.0 )
						  then *gy = 0.0;
						break;

				case '3':	if( (*gy -= 1.0/23.0) < 0.0 )
						  then *gy = 0.0;
						if( (*gx += 1.0/49.0) > 1.0 )
						  then *gx = 1.0;
						break;

				case 'h':
				case '4':	if( (*gx -= 1.0/49.0) < 0.0 )
						  then *gx = 0.0;
						break;

				case 'c':
				case '5':	*gx = *gy = 0.5;

				case 'l':
				case '6':	if( (*gx += 1.0/49.0) > 1.0 )
						  then *gx = 1.0;
						break;

				case '7':	if( (*gy += 1.0/23.0) > 1.0 )
						  then *gy = 1.0;
						if( (*gx -= 1.0/49.0) < 0.0 )
						  then *gx = 0.0;
						break;

				case 'k':
				case '8':	if( (*gy += 1.0/23.0) > 1.0 )
						  then *gy = 1.0;
						break;

				case '9':	if( (*gy += 1.0/23.0) > 1.0 )
						  then *gy = 1.0;
						if( (*gx += 1.0/49.0) > 1.0 )
						  then *gx = 1.0;
						break;
				}
			    }
			*ic = getch();
			break;
#endif

	default:	while( TRUE )
			    {
			    format("%PSI: Enter coordinates ");
			    format("(character x y): ");
			    if(iogetl(stdin,inln,100)==NULL) then exit(1);
			    if( sparse(inln,"{c}{d}{d}",ic,gx,gy)==3	&&
				*gx>=0.0 && *gx<=1.0			&&
				*gy>=0.0 && *gy<=1.0			)
			      then break;
			    format("%PSI:  Illegal input.\n");
			    }
			break;
	}
    }
