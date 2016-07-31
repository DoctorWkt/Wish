/*      Generic SysV specific includes and defines
 *
 * m_gensysv.h: 40.2  7/28/93
 *
 */

#define GENSYSV		/* Generic SysV machine */

#define ATT
#define VARARGS
#define V7JOB
#define PLONG int

#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <termio.h>
#include <sys/file.h>
#include <sys/time.h>
