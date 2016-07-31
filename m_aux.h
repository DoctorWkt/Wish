/*      A/UX 2.01 specific includes and defines
 *
 * m_aux.h: 40.3  8/2/93
 *
 */

#define AUX2_01		/* A/UX 2.01 */

#define POSIX
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
