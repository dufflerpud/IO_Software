#ifndef LOCAL_CAPPARSE_DEFINED
#define LOCAL_CAPPARSE_DEFINED

union c_p_types
    {
    int				intvalue;
    char			*strvalue;
    };

struct c_pentry
    {
    char			*c_name;
    char			c_ecode;
    union c_p_types		c_value;
    };

extern function char *gentry( char *fname, char *name );
extern function struct c_pentry *c_psearch( struct c_pentry *c_p, char *s );
extern function char *strsearch( struct c_pentry *c_p, char *s, char *def );
extern function int intsearch( struct c_pentry *c_p, char *s, int def );
extern function struct c_pentry *parseentry( char *s );
extern subroutine c_p_clean( struct c_pentry *c_p );

#endif
