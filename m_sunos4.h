/*      Sun OS 4.x specific includes and defines
 *
 * $Revision: 41.2 $ $Date: 1995/12/29 05:20:48 $
 *
 */

#define SUNOS4				/* Sun OS 4.x */

#define USES_TERMIOS
#define POSIXJOB
#define VARARGS

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <strings.h>
#include <memory.h>
/* #include <sgtty.h> */
#include <dirent.h>
#include <string.h>
