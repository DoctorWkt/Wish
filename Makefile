CFLAGS= -DMINIX

shell: builtin.s comlined.s exec.s global.s job.s main.s parse.s prints.s \
		signal.s term.s var.s
	cc -v -o shell builtin.s comlined.s exec.s global.s job.s main.s parse.s \
		prints.s signal.s term.s var.s
