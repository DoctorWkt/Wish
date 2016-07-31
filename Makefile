# The Makefile for Shell 2.0.39
#
# Makefile: 39.6  11/22/93
#
# Insert compiler flags here.
# CFLAGS are used when compiling each file
# LDFLAGS are used when linking
#
CFLAGS=-g
#CFLAGS=-g -Wall -traditional -v
#CFLAGS= -O
#CFLAGS= -Wall
#CFLAGS= -pg

#LDFLAGS= -3
#LDFLAGS= -i
#LDFLAGS= -p
#LDFLAGS= -pg

# And the name of your compiler (may not be cc)
#CC=/usr/bin2/cc
#CC=/local/bin/bcc
CC=gcc

# Some compilers generate object files that don't end in `.o'. Alter the
# following variable if this is the case
#O= s
O= o

# Special defines for the compilation stage.
#
#DEFINES=-ZP			# A/UX 2.01

# Libraries to include during the link stage. On some machines (like OSx under
# the ucb universe, you may need to get some ANSI C string functions that are
# not in libc.a. These are: strtok.o strpbrk.o strspn.o strcspn.o
#
#CLIB=-lcurses			# OSX5_1_ATT GENSYSV
CLIB=-ltermcap			# OSX5_1_BSD GENBSD ULTRIX J386BSD0_1
#				  SUNOS3 SUNOS4
#CLIB=-ltermcap strtok.o strpbrk.o strspn.o strcspn.o	# OSX5_1_BSD maybe
#CLIB=-ltermcap -lposix		# AUX2_01
#CLIB= -lterm -lndir		# COHERENT

# On some systems (PC Minix, Coherent), we need to set the final stack size.
STACK= 30000
#FINAL_TOUCHES= fixstack $(STACK) shell ; chmod 755 shell ; size shell # COHERENT
#FINAL_TOUCHES= chmem '=$(STACK)' shell	# MINIX1_5


# ====== END OF CONFIGURABLE SECTION ======



SRCS=   alias.c bind.c builtin.c clebuf.c clex.c comlined.c exec.c file.c \
	hist.c job.c main.c malloc.c meta.c parse.c prints.c \
	signal.c term.c val.c var.c 

PSRCS=  alias.c bind.c builtin.c clebuf.c clex.c comlined.c exec.c file.c \
	hist.c job.c posixjob.c main.c malloc.c meta.c parse.c \
	signal.c term.c val.c var.c 

OBJS=   alias.$(O) bind.$(O) builtin.$(O) clebuf.$(O) clex.$(O) comlined.$(O) \
	exec.$(O) file.$(O) hist.$(O) job.$(O) main.$(O) malloc.$(O) \
	meta.$(O) parse.$(O) prints.$(O) signal.$(O) term.$(O) val.$(O) \
	var.$(O)

shell : $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o shell $(CLIB)
	$(FINAL_TOUCHES)

$(OBJS): Makefile header.h machine.h

job.c:	posixjob.c ucbjob.c v7job.c

# For Coherent, comment the next two lines out!
.c.$(O): $<
	$(CC) $(CFLAGS) $(DEFINES) -c $<

lint : $(SRCS)
	lint $(LINTFLAGS) $(DEFINES) $(SRCS)

proto:	$(PSRCS)
	proto $(PSRCS) | tail +7 | egrep -v 'undef|getaline' > Proto/p2
	cat Proto/p1 Proto/p2 Proto/p3 > proto.h

clean :
	rm -f *.$(O)

clr :
	rm -f *.$(O) shell

realclean:
	rm -f *.$(O) shell
