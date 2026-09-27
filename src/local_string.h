/************************************************************************
 *
 *indx#	local_string.h - Include file for replacement for standard C string routines
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
 *doc#	Include file for replacement for standard C string routines
 ************************************************************************/
#ifndef LOCAL_string_INCLUDED
#define LOCAL_string_INCLUDED
#include <local.h>

#if HANDLER_string == SYSTEM_HANDLER
#include <string.h>
#elif HANDLER_string == LOCAL_HANDLER
extern function int strlen( char *s );
extern function int strcpy( char *sto, char *sfrom );
extern function int strncpy( char *sto, char *sfrom, int size );
extern function int strcat( char *sto, char *sfrom );
extern function int strncat( char *sto, char *sfrom, int size );
extern function int strcmp( char *s1, char *s2 );
extern function int strncmp( char *s1, char *s2, int size );
extern function char *strchr( char *s, char c );
extern function char *strrchr( char *s, char c );
extern function char *strpbrk( char *s1, char *s2 );
extern function int strspn( char *s1, char *s2 );
extern function int strcspn( char *s1, char *s2 );
extern function char *strtok( char *s1, char *s2 );
extern function char *strdup( char *s );

/* These are from ctype.h */
extern function int tolower( char c );
extern function int toupper( char c );
#endif

#if defined DEFINE_INDEX_AS_STRCHR && ! defined index
#define index(s,c)	strchr(s,c)
#define rindex(s,c)	strrchr(s,c)
#endif
#if defined DEFINE_STRCHR_AS_INDEX && ! defined strchr
#define strchr(s,c)	index(s,c)
#define strrchr(s,c)	rindex(s,c)
#endif

#ifdef NEED_STRING
#define string(s)	strdup(s)
#endif

#ifdef NEED_LCASE
#define lcase(c)	tolower(c)
#define ucase(c)	toupper(c)
#endif

extern function int digtobin( char c );
extern function char bintodig( int i );
extern function int isbase( char c, int b );
extern function int orig_abbrev( char *com, char *clist[] );
extern subroutine maketoken( char *s );
extern subroutine strinit( char **var );
extern subroutine stradd( char **var, char c );
extern subroutine strsub( char **var );
extern subroutine strdone( char **var );
extern subroutine setup_string();
#endif
