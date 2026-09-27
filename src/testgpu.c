#include <local.h>
#include <local_format.h>
#include <stdlib.h>

/************************************************************************/
/************************************************************************/
subroutine main()
    {
    format("This is a test.\n");
    char *s = mformat("I like testing.");
    format("This will be a test:  [{s}]\n",s);
    free(s);
    }
