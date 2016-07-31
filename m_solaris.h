/*      Solaris specific includes and defines
 *
 * $Revision: 41.2 $ $Date: 1995/12/29 05:20:48 $
 *
 */

#define SOLARIS_2				/* Solaris 2.x */

#define USES_TERMIOS
#define POSIXJOB
#define VARARGS

#define SIGTYPE void 		      /* Signal handlers return this type */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>
