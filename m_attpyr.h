/*      OSx 5.1a, SysV Universe specific includes and defines
 *
 * $Revision: 41.2 $ $Date: 1995/12/29 02:58:43 $
 *
 */

#define OSX5_1_ATT	/* Pyramid under OSx 5.1a, SysV Universe */

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
#include <string.h>
#include <dirent.h>
#include <termio.h>
#include <sys/file.h>
#include <sys/time.h>

