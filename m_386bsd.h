/*      386BSD-0.1 specific includes and defines
 *
 * m_386bsd.h: 40.3  8/2/93
 *
 */

#define J386BSD0_1		/* 386BSD-0.1 */

#define POSIX			/* Define POSIX for POSIX system calls or */
#undef UCB			/* UCB for 4.3BSD system calls */

#define POSIXJOB		/* Define POSIXJOB for POSIX job control or */
#undef UCBJOB 			/* UCBJOB for 4.3BSD job control */

#define VARARGS
#define SIGTYPE __sighandler_t	/* Signal handlers return this type */


#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/wait.h>

#ifdef POSIX
# include <string.h>
# include <termios.h>
#endif

#ifdef UCB
# include <strings.h>
# include <sgtty.h>
# include <sys/resource.h>
#endif

#ifdef UCBJOB
# include <sys/ioctl.h>
#endif
