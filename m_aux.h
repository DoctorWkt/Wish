/*      A/UX 2.01 specific includes and defines
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 *
 */

#define AUX2_01		/* A/UX 2.01 */

#define USES_TERMIOS
#define POSIXJOB
#define VARARGS

#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>

#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <sys/file.h>
#include <time.h>
#include <sys/wait.h>
