/*      Minix 1.5 specific includes and defines
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 *
 */
#define MINIX1_7	/* Minix 1.7 */

#define USES_TERMIOS
#define PROTO
#define STDARG
#define V7JOB
#define PLONG long	/* Two of Minix's ptrace args are longs !! Why? */

#include <lib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>	/* Only for perror */
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <termcap.h>
#include <dirent.h>
#include <time.h>
