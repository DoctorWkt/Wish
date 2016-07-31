/*      Sun OS 3.x specific includes and defines
 *
 * m_sunos3.h: 40.3  7/28/93
 *
 */

#define SUNOS3		/* Sun OS 3.x */

#define UCB
#define UCBJOB
#define VARARGS

#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sgtty.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>

#ifdef USES_DIRECT
# include <sys/dir.h>
#else
# include <dirent.h>
#endif

#ifdef UCBJOB
# include <sys/ioctl.h>
#endif

#define strchr  index
#define strrchr rindex
#define memset(a,b,c) bzero(a,c)
