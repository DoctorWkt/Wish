/*      OSx 5.1a, BSD Universe specific includes and defines
 *
 * m_bsdpyr.h: 40.4  8/2/93
 *
 */

#define OSX5_1_BSD 	/* Pyramid OSx 5.1a, BSD Universe */

#define UCB
#define UCBJOB
#define USES_DIRECT	/* Uses struct direct, not struct dirent */
#define USES_GETWD	/* Uses getwd(), not getcwd() */
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
#include <sys/dir.h>
#include <strings.h>
#include <wait.h>
#define strchr  index
#define strrchr rindex
#define memset(a,b,c) bzero(a,c)
