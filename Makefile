CFLAGS= -gx -DJOB

shell: builtin.o comlined.o exec.o job.o main.o parse.o signal.o var.o
	cc -o shell builtin.o comlined.o exec.o job.o main.o parse.o \
		signal.o var.o
