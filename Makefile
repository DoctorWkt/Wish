CFLAGS= -gx -DUCB -DSUN -DJOB

shell: builtin.o comlined.o exec.o global.o job.o main.o parse.o prints.o \
		signal.o term.o var.o
	cc -o shell builtin.o comlined.o exec.o global.o job.o main.o parse.o \
		prints.o signal.o term.o var.o -ltermcap
