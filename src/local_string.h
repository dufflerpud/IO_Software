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
