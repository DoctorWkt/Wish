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
#DEFINES=-DUNIVERSE -DATT -DHASH -DSCRIPT
#CLIB=-lcurses

# Pyramid Dual Universe machine under BSD 4.x
#DEFINES=-DUNIVERSE -DUCB -DHASH -DSCRIPT -DJOB
#CLIB=-ltermcap

# Generic SysV machine
#DEFINES=-DATT -DHASH -DSCRIPT
#CLIB=-lcurses

# Generic BSD 4.x machine
#DEFINES=-DUCB -DHASH -DJOB -DSCRIPT
#CLIB=-ltermcap

# Sun OS
DEFINES=-DUCB -DSUN -DHASH -DJOB -DSCRIPT
CLIB=-ltermcap

# Coherent -- comment out the .c.$(O) rule down below, since Coherent
# Make already has it defined in /usr/lib/makeactions & pukes with the
# redefinition.
#DEFINES= -DNEED_GETCWD
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

# The following was used to compile Clam under the previous version of
# ST Minix, using GnuC. Because ST & PC Minix 1.5.10 are nearly identical,
# try compiling _without_ -DATARI_ST and see if Clam compiles & runs.
# If not, try with the define. Once you get it to go, please send patches!
#DEFINES=-DMINIX -DATARI_ST
#CLIB= -ldir32 -liio32 -s -z
#STACK = 30000
#FINAL_TOUCHES= chmem '=$(STACK)' shell

SRCS=   builtin.c clex.c comlined.c \
	exec.c hist.c global.c job.c \
	main.c meta.c parse.c prints.c \
	signal.c term.c var.c

OBJS=   builtin.$(O) clex.$(O) comlined.$(O) \
	exec.$(O) hist.$(O) global.$(O) job.$(O) \
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

clean :
	rm -f *.$(O) shell

store : clean
	compress *.c

retrieve :
	uncompress *.c.Z

delete :
	@ echo "have you backed up? Waiting for 5 seconds..."
	@ sleep 1;echo -n "going..."
	@ sleep 2;echo -n "going..."
	@ sleep 2;echo "GONE\!\!"
	rm -f *.$(O) *.c shell
