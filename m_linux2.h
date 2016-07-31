/*      Linux 2.x specific includes and defines
 *
 * $Revision: 41.0 $ $Date: 1996/06/19 02:44:39 $
 *
 */

#define LINUX_2				/* Linux 2.x */

#define USES_TERMIOS
#define POSIXJOB
#define SIGTYPE void			/* Signal handlers return this type */

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
