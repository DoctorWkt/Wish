/*      Generic SysV specific includes and defines
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 *
 */

#define GENSYSV		/* Generic SysV machine */

#define USES_TCGETA
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
