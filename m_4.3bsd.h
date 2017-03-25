/*      4.3BSD specific includes and defines
 *
 */

#define GENBSD		/* Generic BSD 4.x machine */
#define BSD43		/* and specifically 4.3BSD */

#define USES_SGTTY
#define USES_MORESIG
#define UCBJOB
#define VARARGS
#define USES_DIRECT
#define USES_GETWD


#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <sgtty.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/wait.h>
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
