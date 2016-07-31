/*      Sun OS 4.x specific includes and defines
 *
 * m_sunos4.h: 40.3  8/2/93
 *
 */

#define SUNOS4				/* Sun OS 4.x */

#define POSIX
#define POSIXJOB
#define VARARGS

#include <sys/types.h>
#include <unistd.h>
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
/* #include <sgtty.h> */
#include <dirent.h>
#include <string.h>
