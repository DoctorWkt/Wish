# insert compiler flags here.
# CFLAGS are used when compiling each file
# LDFLAGS are used when linking
#
#CFLAGS= -O -I/usr/include2 
#CFLAGS= -3
CFLAGS=-gx
#CFLAGS= -ansi
#CFLAGS= -O
#LDFLAGS= -3
#LDFLAGS= -i

# And the name of your compiler (may not be cc)
#CC=/usr/bin2/cc
#CC=/local/bin/bcc
#CC=gcc

# Some compilers generate object files that don't end in `.o'. Alter the
# following variable if this is the case
#O= s
O= o

# Depending upon what type of machine you are compiling this for, uncomment
# one of the following sections

# Pyramid Dual Universe machine under SysV
#DEFINES=-DSYSVPYR
#CLIB=-lcurses

# Pyramid Dual Universe machine under BSD 4.x
# You'll need to extract some string routines from the att library
# before compiling. Do:
#  % att ar x /lib/libc.a strtok.o strpbrk.o strspn.o
#
#DEFINES=-DBSDPYR
#CLIB=-ltermcap strtok.o strpbrk.o strspn.o

# Generic SysV machine
#DEFINES=-DGENSYSV
#CLIB=-lcurses

# Generic BSD 4.x machine
# The following three routines may not be available in your library:
#    strtok.o strpbrk.o strspn.o
# If the compilation fails because these are missing, you will have to
# find the files and add them to the CLIB line. You may even have to
# write them yourself; sorry.
#DEFINES=-DGENBSD
#CLIB=-ltermcap

# Sun OS
DEFINES=-DSUN
CLIB=-ltermcap

# Coherent -- comment out the .c.$(O) rule down below, since Coherent
# Make already has it defined in /usr/lib/makeactions & pukes with the
# redefinition.
#DEFINES= -DCOHERENT
#CLIB= -lterm -lndir
#STACK = 3000
#FINAL_TOUCHES = fixstack $(STACK) shell ; chmod 755 shell ; size shell
#CFLAGS = -O -VSUVAR $(DEFINES)

# Clam won't compile under Minix 1.5.10, because asld runs out of symbol
# table space. It may just compile under 1.3c. At the moment I'm using a
# mixture of the K&R compiler and the Ansi compiler to avoid asld.
#DEFINES=-DMINIX
#STACK = 30000
#FINAL_TOUCHES= chmem '=$(STACK)' shell

SRCS=   alias.c builtin.c clex.c comlined.c \
	exec.c file.c hist.c job.c \
	main.c meta.c parse.c prints.c \
	signal.c term.c var.c

OBJS=   alias.$(O) builtin.$(O) clex.$(O) comlined.$(O) \
	exec.$(O) file.$(O) hist.$(O) job.$(O) \
	main.$(O) meta.$(O) parse.$(O) prints.$(O) \
	signal.$(O) term.$(O) var.$(O)

shell : $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o shell $(CLIB)
	$(FINAL_TOUCHES)

$(OBJS): Makefile header.h

# For Coherent, comment the next two lines out!
.c.$(O): $<
	$(CC) $(CFLAGS) $(DEFINES) -c $<

lint : $(SRCS)
	lint $(LINTFLAGS) $(DEFINES) $(SRCS)

clr :
	rm -f *.$(O) shell

clean :
	rm -f *.$(O)

store : clean
	compress *.c

retrieve :
	uncompress *.c.Z

delete :
	@ echo "have you backed up? Waiting for 5 seconds..."
	@ sleep 1;echo -n "going..."
	@ sleep 2;echo -n "going..."
	@ sleep 2;echo "GONE\!\!"
	rm -f *.$(O) *.c *.h shell
