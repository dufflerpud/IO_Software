# Documentation for IO Software software
This is very old software (Circa 1985) that I created and own the rights
to for the use of Vibrac Corporation.  It is included here for reference
only, as it really hasn't been tested in decades.  I made sure it builds
and the test programs "mutation" and "uss" actually run, but there is
a ton of standalone software that I no longer have any way of testing.
<hr>

<table src="src/*.c src/*.h"><tr><th align=left><a href='#dt_8CRcpuiQd'>capparse.c</a></th><td>Software to parse characteristics of /etc/remote</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQe'>cversion.c</a></th><td>Embed version number in archive</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQf'>errors.c</a></th><td>Software to track a stack of errors</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQg'>event.c</a></th><td>Log an event</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQh'>format.c</a></th><td>Very much like printf and related software</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQi'>getcwd.c</a></th><td>Get current working directory (probably for standalone use)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQj'>getenv.c</a></th><td>Get value of an environment variable (standalone)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQk'>hashtable.c</a></th><td>Software for creating and maintaining a hash table</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQl'>iosubs.c</a></th><td>Software to do buffered i/o (replacement for stdio)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQm'>lock.c</a></th><td>Software for locking a resource with symlinks</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQn'>mem.c</a></th><td>Software for memory allocation</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQo'>mutation.c</a></th><td>Software to simulate programs mutating in a test tube</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQp'>mutation.m4.c</a></th><td>(VERY brief explanation of what this file is/does)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQq'>parse.c</a></th><td>Very much like printf and related software</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQr'>plot.c</a></th><td>Software that knows how to plot on a small number of plotters</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQs'>settty.c</a></th><td>Software for setting tty characteristics</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQt'>sleep.c</a></th><td>Standalone sleep routine</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQu'>string.c</a></th><td>Replacement for standard C string routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQv'>system.c</a></th><td>Standalone replacement for standard system() routine</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQw'>tempfile.c</a></th><td>Software for creating files in /tmp</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQx'>testcwd.c</a></th><td>(VERY brief explanation of what this file is/does)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQy'>testgpu.c</a></th><td>Routines to test software from this library</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiQz'>time.c</a></th><td>Standalone routines for manipulating time strings</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR0'>ttyhandler.c</a></th><td>Routines for doing i/o to a terminal</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR1'>uss.c</a></th><td>Unix Spread Sheet</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR2'>gpm.h</a></th><td>(VERY brief explanation of what this file is/does)</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR3'>local.h</a></th><td>One place to set configuration variables</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR4'>local_capparse.h</a></th><td>Include file for terminal capability routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR5'>local_error.h</a></th><td>Include file for error handling</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR6'>local_format.h</a></th><td>Include file for replacement for printf</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR7'>local_getcwd.h</a></th><td>Include file for standalone getcwd</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR8'>local_getenv.h</a></th><td>Include file for local version of getenv</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiR9'>local_iosubs.h</a></th><td>Include file for standalone i/o routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRA'>local_lock.h</a></th><td>Include file for resource locking</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRB'>local_mem.h</a></th><td>Include file for memory management</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRC'>local_parse.h</a></th><td>Include file for scanf replacement</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRD'>local_settty.h</a></th><td>Include file for software to set tty characteristics</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRE'>local_sleep.h</a></th><td>Include file for standalone sleep routine</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRF'>local_string.h</a></th><td>Include file for replacement for standard C string routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRG'>local_system.h</a></th><td>Include file for standalone standare system routine</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRH'>local_tempfile.h</a></th><td>Include file for software to handle /tmp files</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRI'>local_time.h</a></th><td>Include file for standalone time routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRJ'>plot.h</a></th><td>Include file for plot routines</td></tr>
<tr><th align=left><a href='#dt_8CRcpuiRK'>ttyhandler.h</a></th><td>Include file for routines to do terminal i/o</td></tr></table>

<hr>

<div id=docs>

## <a id='dt_8CRcpuiQd'>capparse.c</a>
Software to parse characteristics of /etc/remote

## <a id='dt_8CRcpuiQe'>cversion.c</a>
Embed version number in archive

## <a id='dt_8CRcpuiQf'>errors.c</a>
Software to track a stack of errors

## <a id='dt_8CRcpuiQg'>event.c</a>
Log an event

## <a id='dt_8CRcpuiQh'>format.c</a>
Very much like printf and related software, except
printf("text%*.2f",10,floatvar)
becomes
format("text{fr.2}",floatvar,10)
known modifiers are:
left justify
right justify
center spacing (center justify)
decimal places
specify pad character
and object types are:
string
float
int
character
long

## <a id='dt_8CRcpuiQi'>getcwd.c</a>
Get current working directory (probably for standalone use)

## <a id='dt_8CRcpuiQj'>getenv.c</a>
Get value of an environment variable (standalone)

## <a id='dt_8CRcpuiQk'>hashtable.c</a>
Software for creating and maintaining a hash table

## <a id='dt_8CRcpuiQl'>iosubs.c</a>
Software to do buffered i/o (replacement for stdio)

## <a id='dt_8CRcpuiQm'>lock.c</a>
Software for locking a resource with symlinks

## <a id='dt_8CRcpuiQn'>mem.c</a>
Software for memory allocation

## <a id='dt_8CRcpuiQo'>mutation.c</a>
Software to simulate programs mutating in a test tube
Documentation in mutation.html.

## <a id='dt_8CRcpuiQp'>mutation.m4.c</a>
(Less brief explanation of what this file is/does)

## <a id='dt_8CRcpuiQq'>parse.c</a>
Very much like scanf and related software, except
printf("text%*.2f",10,floatvar)
becomes
format("text{fr.2}",floatvar,10)
known modifiers are:
left justify
right justify
center spacing (center justify)
decimal places
specify pad character
and object types are:
string
float
int
character
long

## <a id='dt_8CRcpuiQr'>plot.c</a>
Software that knows how to plot on a small number of plotters

## <a id='dt_8CRcpuiQs'>settty.c</a>
Software for setting tty characteristics

## <a id='dt_8CRcpuiQt'>sleep.c</a>
Standalone sleep routine

## <a id='dt_8CRcpuiQu'>string.c</a>
Replacement for standard C string routines

## <a id='dt_8CRcpuiQv'>system.c</a>
Standalone replacement for standard system() routine

## <a id='dt_8CRcpuiQw'>tempfile.c</a>
Software for creating files in /tmp

## <a id='dt_8CRcpuiQx'>testcwd.c</a>
(Less brief explanation of what this file is/does)

## <a id='dt_8CRcpuiQy'>testgpu.c</a>
Routines to test software from this library

## <a id='dt_8CRcpuiQz'>time.c</a>
Standalone routines for manipulating time strings

## <a id='dt_8CRcpuiR0'>ttyhandler.c</a>
Routines for doing i/o to a terminal

## <a id='dt_8CRcpuiR1'>uss.c</a>
Unix Spread sheet (documentation in uss.html)

## <a id='dt_8CRcpuiR2'>gpm.h</a>
(Less brief explanation of what this file is/does)

## <a id='dt_8CRcpuiR3'>local.h</a>
One place to set configuration variables

## <a id='dt_8CRcpuiR4'>local_capparse.h</a>
Include file for terminal capability routines

## <a id='dt_8CRcpuiR5'>local_error.h</a>
Include file for error handling

## <a id='dt_8CRcpuiR6'>local_format.h</a>
Include file for replacement for printf

## <a id='dt_8CRcpuiR7'>local_getcwd.h</a>
Include file for standalone getcwd

## <a id='dt_8CRcpuiR8'>local_getenv.h</a>
Include file for local version of getenv

## <a id='dt_8CRcpuiR9'>local_iosubs.h</a>
Include file for standalone i/o routines

## <a id='dt_8CRcpuiRA'>local_lock.h</a>
Include file for resource locking

## <a id='dt_8CRcpuiRB'>local_mem.h</a>
Include file for memory management

## <a id='dt_8CRcpuiRC'>local_parse.h</a>
Include file for scanf replacement

## <a id='dt_8CRcpuiRD'>local_settty.h</a>
Include file for software to set tty characteristics

## <a id='dt_8CRcpuiRE'>local_sleep.h</a>
Include file for standalone sleep routine

## <a id='dt_8CRcpuiRF'>local_string.h</a>
Include file for replacement for standard C string routines

## <a id='dt_8CRcpuiRG'>local_system.h</a>
Include file for standalone standare system routine

## <a id='dt_8CRcpuiRH'>local_tempfile.h</a>
Include file for software to handle /tmp files

## <a id='dt_8CRcpuiRI'>local_time.h</a>
Include file for standalone time routines

## <a id='dt_8CRcpuiRJ'>plot.h</a>
Include file for plot routines

## <a id='dt_8CRcpuiRK'>ttyhandler.h</a>
Include file for routines to do terminal i/o</div>

<hr>

Many of these tools are extremely specific to the environment the author was
working in or the projects involved.  Many started out very specific and got
generalized over time - sometimes way beyond how they will ever realistically
be used.



