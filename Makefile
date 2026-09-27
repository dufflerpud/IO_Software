#indx#	Makefile.simple - Makefile for old IO Software tools
#@HDR@	$Id$
#@HDR@
#@HDR@	Copyright (c) 2024-2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
#@HDR@
#@HDR@	Permission is hereby granted, free of charge, to any person
#@HDR@	obtaining a copy of this software and associated documentation
#@HDR@	files (the "Software"), to deal in the Software without
#@HDR@	restriction, including without limitation the rights to use,
#@HDR@	copy, modify, merge, publish, distribute, sublicense, and/or
#@HDR@	sell copies of the Software, and to permit persons to whom
#@HDR@	the Software is furnished to do so, subject to the following
#@HDR@	conditions:
#@HDR@	
#@HDR@	The above copyright notice and this permission notice shall be
#@HDR@	included in all copies or substantial portions of the Software.
#@HDR@	
#@HDR@	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
#@HDR@	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#@HDR@	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
#@HDR@	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#@HDR@	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#@HDR@	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#@HDR@	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#@HDR@	OTHER DEALINGS IN THE SOFTWARE.
#
#hist#	2026-02-10 - Christopher.M.Caldwell0@gmail.com - Created
########################################################################
#doc#	Makefile.simple - Makefile for old IO Software tools
########################################################################
PROGRAMS=uss mutation
PROJECTSDIR?=$(shell echo $(CURDIR) | sed -e 's+/projects/.*+/projects+')
include $(PROJECTSDIR)/common/Makefile.std

#**************************************************************************#
#	makefile:  Shell commands to build the C library
#	%Z% %M% %I% %G%
#	Created by Christopher M. Caldwell of IO Software, Inc.
#**************************************************************************#

INCDIR=$(SRCDIR)
LIBDIR=$(OBJDIR)
CFLAGS=		-g -O3 -I$(INCDIR)
LDFLAGS=
SOURCES=	Makefile io.h iosubs.c format.c getcwd.c		\
		    getenv.c mem.c parse.c sleep.c string.c		\
		    system.c tempfile.c time.c cversion.c		\
		    event.c plot.c ttyhandler.c hashtable.c		\
		    capparse.c lock.c
CC=gcc
ARCHIVES=$(LIBDIR)/libgpu.a
BINS=$(BINDIR)/testgpu $(BINDIR)/uss $(BINDIR)/mutation
CHMOD=chmod
CHOWN=chown
RANLIB=ranlib
CP=cp
LP=lp

bins:		all

all:		$(ARCHIVES) $(BINS)

$(BINDIR)/time:	time.c
		$(CC) $(CFLAGS) -DMAIN $^ -lu -o $@

$(LIBDIR)/libgpu.a:	\
		$(OBJDIR)/format.o $(OBJDIR)/string.o			\
		$(OBJDIR)/getcwd.o $(OBJDIR)/getenv.o			\
		$(OBJDIR)/parse.o $(OBJDIR)/sleep.o			\
		$(OBJDIR)/system.o $(OBJDIR)/tempfile.o			\
		$(OBJDIR)/time.o $(OBJDIR)/iosubs.o			\
		$(OBJDIR)/mem.o $(OBJDIR)/event.o			\
		$(OBJDIR)/plot.o $(OBJDIR)/ttyhandler.o			\
		$(OBJDIR)/hashtable.o $(OBJDIR)/capparse.o		\
		$(OBJDIR)/lock.o $(OBJDIR)/cversion.o
		rm -f $@
		ar cq $@ $^

orig_install:	$(LIBDIR)/libgpu.a
		$(CP) $(LIBDIR)/libgpu.a $(LIB)/libgpu.a
#		$(CHMOD) 644 $(LIB)/libgpu.a
#		$(CHOWN) bin:bin $(LIB)/libgpu.a
		$(RANLIB) $(LIB)/libgpu.a

clean:
		rm -f $(OBJDIR)/*.o core $(ARCHIVES) $(BINS)

print:
		$(LP) -s -b $(SOURCES)

sources:
		@echo $(SOURCES)

$(BINDIR)/testgpu:\
		$(OBJDIR)/testgpu.o $(OBJDIR)/format.o			\
		$(OBJDIR)/string.o $(OBJDIR)/mem.o
		@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
		$(CC) $(LDFLAGS) $^ -o $@

$(BINDIR)/uss:	$(OBJDIR)/uss.o $(OBJDIR)/format.o $(OBJDIR)/string.o	\
		$(OBJDIR)/mem.o
		@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
		$(CC) $(LDFLAGS) $^ -lm -lcurses -o $@

$(BINDIR)/mutation:\
		$(OBJDIR)/mutation.m4.o $(OBJDIR)/format.o		\
		$(OBJDIR)/settty.o $(OBJDIR)/string.o $(OBJDIR)/mem.o
		@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
		$(CC) $(LDFLAGS) $^ -lm -lcurses -o $@

$(OBJDIR)/mutation.m4.o: \
		$(SRCDIR)/mutation.c
		@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
		m4 <$(SRCDIR)/mutation.c >$(OBJDIR)/mutation.m4.c
		$(CC) $(CFLAGS) -c $(OBJDIR)/mutation.m4.c -o $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
		@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
		$(CC) $(CFLAGS) -c $^ -o $@

%:
		@echo "Invoking std_$@ rule:"
		@$(MAKE) ORIGINAL_TARGET=$@ std_$@
