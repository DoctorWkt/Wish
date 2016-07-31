/*      Ultrix specific includes and defines
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 *
 */

#define ULTRIX4_2	/* Ultrix machine */

#define USES_SGTTY
#define USES_MORESIG
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
