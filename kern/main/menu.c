/*
 * In-kernel menu and command dispatcher.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/limits.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <syscall.h>
#include <uio.h>
#include <vfs.h>
#include <sfs.h>
#include <test.h>
#include "opt-synchprobs.h"
#include "opt-sfs.h"
#include "opt-net.h"

#define _PATH_SHELL "/bin/sh"

#define MAXMENUARGS  16

void
getinterval(time_t s1, u_int32_t ns1, time_t s2, u_int32_t ns2,
	    time_t *rs, u_int32_t *rns)
{
	if (ns2 < ns1) {
		ns2 += 1000000000;
		s2--;
	}

	*rns = ns2 - ns1;
	*rs = s2 - s1;
}

////////////////////////////////////////////////////////////
//
// Command menu functions 

/*
 * Function for a thread that runs an arbitrary userlevel program by
 * name.
 *
 * Note: this cannot pass arguments to the program. You may wish to 
 * change it so it can, because that will make testing much easier
 * in the future.
 *
 * It copies the program name because runprogram destroys the copy
 * it gets by passing it to vfs_open(). 
 */
static
void
cmd_progthread(void *ptr, unsigned long nargs)
{
	char **args = ptr;
	char progname[128];
	int result;

	assert(nargs >= 1);

	if (nargs > 2) {
		kprintf("Warning: argument passing from menu not supported\n");
	}

	/* Hope we fit. */
	assert(strlen(args[0]) < sizeof(progname));

	strcpy(progname, args[0]);

	result = runprogram(progname);
	if (result) {
		kprintf("Running program %s failed: %s\n", args[0],
			strerror(result));
		return;
	}

	/* NOTREACHED: runprogram only returns on error. */
}

/*
 * Common code for cmd_prog and cmd_shell.
 *
 * Note that this does not wait for the subprogram to finish, but
 * returns immediately to the menu. This is usually not what you want,
 * so you should have it call your system-calls-assignment waitpid
 * code after forking.
 *
 * Also note that because the subprogram's thread uses the "args"
 * array and strings, until you do this a race condition exists
 * between that code and the menu input code.
 */
static
int
common_prog(int nargs, char **args)
{
	int result;
	struct thread * thread;

#if OPT_SYNCHPROBS
	kprintf("Warning: this probably won't work with a "
		"synchronization-problems kernel.\n");
#endif

	result = thread_fork(args[0] /* thread name */,
			args /* thread arg */, nargs /* thread arg */,
			cmd_progthread, &thread);
	if (result) {
		kprintf("thread_fork failed: %s\n", strerror(result));
		return result;
	}

	return thread_join(thread);
}

/*
 * Command for running an arbitrary userlevel program.
 */
static
int
cmd_prog(int nargs, char **args)
{
	if (nargs < 2) {
		kprintf("Usage: p program [arguments]\n");
		return EINVAL;
	}

	/* drop the leading "p" */
	args++;
	nargs--;

	return common_prog(nargs, args);
}

/*
 * Command for starting the system shell.
 */
static
int
cmd_shell(int nargs, char **args)
{
	(void)args;
	if (nargs != 1) {
		kprintf("Usage: s\n");
		return EINVAL;
	}

	args[0] = (char *)_PATH_SHELL;

	return common_prog(nargs, args);
}

/*
 * Command for changing directory.
 */
static
int
cmd_chdir(int nargs, char **args)
{
	if (nargs != 2) {
		kprintf("Usage: cd directory\n");
		return EINVAL;
	}

	return vfs_chdir(args[1]);
}

/*
 * Command for printing the current directory.
 */
static
int
cmd_pwd(int nargs, char **args)
{
	char buf[PATH_MAX+1];
	struct uio ku;
	int result;

	(void)nargs;
	(void)args;

	mk_kuio(&ku, buf, sizeof(buf)-1, 0, UIO_READ);
	result = vfs_getcwd(&ku);
	if (result) {
		kprintf("vfs_getcwd failed (%s)\n", strerror(result));
		return result;
	}

	/* null terminate */
	buf[sizeof(buf)-1-ku.uio_resid] = 0;

	/* print it */
	kprintf("%s\n", buf);

	return 0;
}

/*
 * Command for running sync.
 */
static
int
cmd_sync(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	vfs_sync();

	return 0;
}

/*
 * Command for doing an intentional panic.
 */
static
int
cmd_panic(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	panic("User requested panic\n");
	return 0;
}

/*
 * Command for shutting down.
 */
static
int
cmd_quit(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	vfs_sync();
	sys_reboot(RB_POWEROFF);
	thread_exit();
	return 0;
}

/*
 * Command for mounting a filesystem.
 */

/* Table of mountable filesystem types. */
static const struct {
	const char *name;
	int (*func)(const char *device);
} mounttable[] = {
#if OPT_SFS
	{ "sfs", sfs_mount },
#endif
	{ NULL, NULL }
};

static
int
cmd_mount(int nargs, char **args)
{
	char *fstype;
	char *device;
	int i;

	if (nargs != 3) {
		kprintf("Usage: mount fstype device:\n");
		return EINVAL;
	}

	fstype = args[1];
	device = args[2];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	for (i=0; mounttable[i].name; i++) {
		if (!strcmp(mounttable[i].name, fstype)) {
			return mounttable[i].func(device);
		}
	}
	kprintf("Unknown filesystem type %s\n", fstype);
	return EINVAL;
}

static
int
cmd_unmount(int nargs, char **args)
{
	char *device;

	if (nargs != 2) {
		kprintf("Usage: unmount device:\n");
		return EINVAL;
	}

	device = args[1];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	return vfs_unmount(device);
}

/*
 * Command to set the "boot fs". 
 *
 * The boot filesystem is the one that pathnames like /bin/sh with
 * leading slashes refer to.
 *
 * The default bootfs is "emu0".
 */
static
int
cmd_bootfs(int nargs, char **args)
{
	char *device;

	if (nargs != 2) {
		kprintf("Usage: bootfs device\n");
		return EINVAL;
	}

	device = args[1];

	/* Allow (but do not require) colon after device name */
	if (device[strlen(device)-1]==':') {
		device[strlen(device)-1] = 0;
	}

	return vfs_setbootfs(device);
}

static
int
cmd_kheapstats(int nargs, char **args)
{
	(void)nargs;
	(void)args;

	kheap_printstats();
	
	return 0;
}

////////////////////////////////////////
//
// Menus.

static
void
showmenu(const char *name, const char *x[])
{
	int ct, half, i;

	kprintf("\n");
	kprintf("%s\n", name);
	
	for (i=ct=0; x[i]; i++) {
		ct++;
	}
	half = (ct+1)/2;

	for (i=0; i<half; i++) {
		kprintf("    %-36s", x[i]);
		if (i+half < ct) {
			kprintf("%s", x[i+half]);
		}
		kprintf("\n");
	}

	kprintf("\n");
}

static const char *opsmenu[] = {
	"[s]       Shell                     ",
	"[p]       Other program             ",
	"[dbflags] Debug flags               ",
	"[mount]   Mount a filesystem        ",
	"[unmount] Unmount a filesystem      ",
	"[bootfs]  Set \"boot\" filesystem     ",
	"[pf]      Print a file              ",
	"[cd]      Change directory          ",
	"[pwd]     Print current directory   ",
	"[sync]    Sync filesystems          ",
	"[panic]   Intentional panic         ",
	"[q]       Quit and shut down        ",
	NULL
};



static
int
cmd_opsmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 operations menu", opsmenu);
	return 0;
}

static const char *testmenu[] = {
	"[at]  Array test                    ",
	"[bt]  Bitmap test                   ",
	"[qt]  Queue test                    ",
	"[km1] Kernel malloc test            ",
	"[km2] kmalloc stress test           ",
	"[tt1] Thread test 1                 ",
	"[tt2] Thread test 2                 ",
	"[tt3] Thread test 3                 ",
#if OPT_NET
	"[net] Network test                  ",
#endif
	"[sy1] Semaphore test                ",
	"[sy2] Lock test             (1)     ",
	"[sy3] CV test               (1)     ",
	"[fs1] Filesystem test               ",
	"[fs2] FS read stress        (4)     ",
	"[fs3] FS write stress       (4)     ",
	"[fs4] FS write stress 2     (4)     ",
	"[fs5] FS create stress      (4)     ",
	NULL
};

static
int
cmd_testmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 tests menu", testmenu);
	kprintf("    (1) These tests will fail until you finish the "
		"synch assignment.\n");
	kprintf("    (4) These tests will fail until you finish the "
		"file system assignment.\n");
	kprintf("\n");

	return 0;
}

static const char *mainmenu[] = {
	"[?o] Operations menu                ",
	"[?t] Tests menu                     ",
#if OPT_SYNCHPROBS
	"[1a] Cat/mouse with semaphores      ",
	"[1b] Cat/mouse with locks and CVs   ",
	"[1c] Stoplight                      ",
#endif
	"[kh] Kernel heap stats              ",
	"[q] Quit and shut down              ",
	NULL
};

static
int
cmd_mainmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 kernel menu", mainmenu);
	return 0;
}

//Debug Flags Struct //
static const char *dbflagsmen[] = {
	"[df 1 on/off]        DB_LOCORE   ",
	"[df 2 on/off]        DB_SYSCALL  ",
	"[df 3 on/off]        DB_INTERRUPT",
	"[df 4 on/off]        DB_DEVICE   ",
	"[df 5 on/off]        DB_THREADS  ",
	"[df 6 on/off]        DB_VM       ",
	"[df 7 on/off]        DB_EXEC     ",
	"[df 8 on/off]        DB_VFS      ",
	"[df 9 on/off]        DB_SFS      ",
	"[df 10 on/off]       DB_NET      ",
	"[df 11 on/off]       DB_NETFS    ",
	"[df 12 on/off]       DB_KMALLOC  ",
	NULL
};

static
int
cmd_dbflagsmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("OS/161 Debug Flags",dbflagsmen);
	kprintf("Current value of dbflags is 0x%x \n",dbflags);
	return 0;
}

//DEBUG FLAG FUNCTIONS//

//ON functions//

static
int
cmd_df1on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_LOCORE + dbflags;
	return 0;
}

static
int
cmd_df2on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_SYSCALL + dbflags;
	return 0;
}

static
int
cmd_df3on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_INTERRUPT + dbflags;
	return 0;
}

static
int
cmd_df4on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_DEVICE + dbflags;
	return 0;
}

static
int
cmd_df5on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_THREADS + dbflags;
	return 0;
}

static
int
cmd_df6on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_VM + dbflags;
	return 0;
}

static
int
cmd_df7on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_EXEC + dbflags;
	return 0;
}

static
int
cmd_df8on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_VFS + dbflags;
	return 0;
}

static
int
cmd_df9on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_SFS + dbflags;
	return 0;
}

static
int
cmd_df10on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_NET + dbflags;
	return 0;
}

static
int
cmd_df11on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags =  DB_NETFS + dbflags;
	return 0;
}

static
int
cmd_df12on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = DB_KMALLOC + dbflags;
	return 0;
}

//OFF FUNCTIONS//


static
int
cmd_df1off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_LOCORE;
	return 0;
}

static
int
cmd_df2off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_SYSCALL;
	return 0;
}

static
int
cmd_df3off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_INTERRUPT;
	return 0;
}

static
int
cmd_df4off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_DEVICE;
	return 0;
}

static
int
cmd_df5off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_THREADS;
	return 0;
}

static
int
cmd_df6off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_VM;
	return 0;
}

static
int
cmd_df7off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_EXEC;
	return 0;
}

static
int
cmd_df8off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_VFS;
	return 0;
}

static
int
cmd_df9off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_SFS;
	return 0;
}

static
int
cmd_df10off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_NET;
	return 0;
}

static
int
cmd_df11off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_NETFS;
	return 0;
}

static
int
cmd_df12off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = dbflags - DB_KMALLOC;
	return 0;
}

static
int
cmd_dfinvalid(int n, char **a)
{
	(void)n;
	(void)a;
	kprintf("EXPECTING: Usage: df nr on/off \n");
	return 0;
}



////////////////////////////////////////
//
// Command table.

static struct {
	const char *name;
	int (*func)(int nargs, char **args);
} cmdtable[] = {
	/* menus */
	{ "?",		cmd_mainmenu },
	{ "h",		cmd_mainmenu },
	{ "help",	cmd_mainmenu },
	{ "?o",		cmd_opsmenu },
	{ "?t",		cmd_testmenu },

	/* operations */
	{ "s",		cmd_shell },
	{ "p",		cmd_prog },
	{ "mount",	cmd_mount },
	{ "dbflags", cmd_dbflagsmenu},
	{ "unmount",	cmd_unmount },
	{ "bootfs",	cmd_bootfs },
	{ "pf",		printfile },
	{ "cd",		cmd_chdir },
	{ "pwd",	cmd_pwd },
	{ "sync",	cmd_sync },
	{ "panic",	cmd_panic },
	{ "q",		cmd_quit },
	{ "exit",	cmd_quit },
	{ "halt",	cmd_quit },
	{ "df1on",	cmd_df1on },
	{ "df2on",	cmd_df2on },
	{ "df3on",	cmd_df3on },
	{ "df4on",	cmd_df4on },
	{ "df5on",	cmd_df5on },
	{ "df6on",	cmd_df6on },
	{ "df7on",	cmd_df7on },
	{ "df8on",	cmd_df8on },
	{ "df9on",	cmd_df9on},
	{ "df10on",	cmd_df10on },
	{ "df11on",	cmd_df11on },
	{ "df12on",	cmd_df12on },
	{ "df1off",	cmd_df1off },
	{ "df2off",	cmd_df2off },
	{ "df3off",	cmd_df3off },
	{ "df4off",	cmd_df4off },
	{ "df5off",	cmd_df5off },
	{ "df6off",	cmd_df6off },
	{ "df7off",	cmd_df7off },
	{ "df8off",	cmd_df8off },
	{ "df9off",	cmd_df9off },
	{ "df10off",cmd_df10off },
	{ "df11off",cmd_df11off },
	{ "df12off",cmd_df12off },
	{ "dfinval",cmd_dfinvalid},
#if OPT_SYNCHPROBS
	/* in-kernel synchronization problems */
	{ "1a",		catmousesem },
	{ "1b",		catmouselock },
	{ "1c",		createcars },
#endif

	/* stats */
	{ "kh",         cmd_kheapstats },

	/* base system tests */
	{ "at",		arraytest },
	{ "bt",		bitmaptest },
	{ "qt",		queuetest },
	{ "km1",	malloctest },
	{ "km2",	mallocstress },
#if OPT_NET
	{ "net",	nettest },
#endif
	{ "tt1",	threadtest },
	{ "tt2",	threadtest2 },
	{ "tt3",	threadtest3 },
	{ "sy1",	semtest },

	/* synchronization assignment tests */
	{ "sy2",	locktest },
	{ "sy3",	cvtest },

	/* file system assignment tests */
	{ "fs1",	fstest },
	{ "fs2",	readstress },
	{ "fs3",	writestress },
	{ "fs4",	writestress2 },
	{ "fs5",	createstress },

	{ NULL, NULL }
};

/*
 * Process a single command.
 */
static
int
cmd_dispatch(char *cmd)
{
	time_t beforesecs, aftersecs, secs;
	u_int32_t beforensecs, afternsecs, nsecs;
	char *args[MAXMENUARGS];
	int nargs=0;
	char *word;
	char *context;
	int i, result;

	for (word = strtok_r(cmd, " \t", &context);
	     word != NULL;
	     word = strtok_r(NULL, " \t", &context)) {
		if (nargs >= MAXMENUARGS) {
			kprintf("Command line has too many words\n");
			return E2BIG;
		}
		args[nargs++] = word;
	}
	if (nargs==0) {
		return 0;
	}
//	char *check[MAXMENUARGS] = args;
	
	for (i=0; cmdtable[i].name; i++) {
		
		if (*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func!=NULL);

			gettime(&beforesecs, &beforensecs);

			result = cmdtable[i].func(nargs, args);

			gettime(&aftersecs, &afternsecs);
			getinterval(beforesecs, beforensecs,
				    aftersecs, afternsecs,
				    &secs, &nsecs);

			kprintf("Operation took %lu.%09lu seconds\n",
				(unsigned long) secs,
				(unsigned long) nsecs);
			return result;
		}
	/*	else if ( args == check && strcmp(store, cmdtable[i].name)) {
			kprintf("entering first if");
			assert(cmdtable[i].func!=NULL);

			gettime(&beforesecs, &beforensecs);

			result = cmdtable[i].func(nargs, args);

			gettime(&aftersecs, &afternsecs);
			getinterval(beforesecs, beforensecs,
				    aftersecs, afternsecs,
				    &secs, &nsecs);

			kprintf("Operation took %lu.%09lu seconds\n",
				(unsigned long) secs,
				(unsigned long) nsecs);
			return result;
		} */
	}
	kprintf("%s: Command not found\n", args[0]);
	return EINVAL;
}

/*
 * Evaluate a command line that may contain multiple semicolon-delimited
 * commands.
 *
 * If "isargs" is set, we're doing command-line processing; print the
 * comamnds as we execute them and panic if the command is invalid or fails.
 */
static
void
menu_execute(char *line, int isargs)
{
	char *command;
	char *context;
	int result;

	for (command = strtok_r(line, ";", &context);
	     command != NULL;
	     command = strtok_r(NULL, ";", &context)) {

		if (isargs) {
			kprintf("OS/161 kernel: %s\n", command);
		}
		result = cmd_dispatch(command);
		if (result) {
			kprintf("Menu command failed: %s\n", strerror(result));
			if (isargs) {
				panic("Failure processing kernel arguments\n");
			}
		}
	}
}

/*
 * Command menu main loop.
 *
 * First, handle arguments passed on the kernel's command line from
 * the bootloader. Then loop prompting for commands.
 *
 * The line passed in from the bootloader is treated as if it had been
 * typed at the prompt. Semicolons separate commands; spaces and tabs
 * separate words (command names and arguments).
 *
 * So, for instance, to mount an SFS on lhd0 and make it the boot
 * filesystem, and then boot directly into the shell, one would use
 * the kernel command line
 *
 *      "mount sfs lhd0; bootfs lhd0; s"
 */

// DF_ON//
	char df_1on[] = "df1on";
	char df_2on[] = "df2on";
	char df_3on[] = "df3on";
	char df_4on[] = "df4on";
	char df_5on[] = "df5on";
	char df_6on[] = "df6on";
	char df_7on[] = "df7on";
	char df_8on[] = "df8on";
	char df_9on[] = "df9on";
	char df_10on[] = "df10on";
	char df_11on[] = "df11on";
	char df_12on[] = "df12on";

	// DF_OFF//

	char df_1off[] = "df1off";
	char df_2off[] = "df2off";
	char df_3off[] = "df3off";
	char df_4off[] = "df4off";
	char df_5off[] = "df5off";
	char df_6off[] = "df6off";
	char df_7off[] = "df7off";
	char df_8off[] = "df8off";
	char df_9off[] = "df9off";
	char df_10off[] = "df10off";
	char df_11off[] = "df11off";
	char df_12off[] = "df12off";

	//DFINVALID//
	char df_inval[] = "dfinval";


char* db_proc( char* cmd);
char* db_proc( char* cmd)
{

	// DF ON//
	char df1on[] = "df 1 on";
	char df2on[] = "df 2 on";
	char df3on[] = "df 3 on";
	char df4on[] = "df 4 on";
	char df5on[] = "df 5 on";
	char df6on[] = "df 6 on";
	char df7on[] = "df 7 on";
	char df8on[] = "df 8 on";
	char df9on[] = "df 9 on";
	char df10on[] = "df 10 on";
	char df11on[] = "df 11 on";
	char df12on[] = "df 12 on";
// DF OFF //
	char df1off[] = "df 1 off";
	char df2off[] = "df 2 off";
	char df3off[] = "df 3 off";
	char df4off[] = "df 4 off";
	char df5off[] = "df 5 off";
	char df6off[] = "df 6 off";
	char df7off[] = "df 7 off";
	char df8off[] = "df 8 off";
	char df9off[] = "df 9 off";
	char df10off[] = "df 10 off";
	char df11off[] = "df 11 off";
	char df12off[] = "df 12 off";

	// Checking argument // 
	int df1on_ = strcmp(cmd, df1on);
	int df1off_ = strcmp(cmd, df1off);

	int df2on_ = strcmp(cmd, df2on);
	int df2off_ = strcmp(cmd, df2off);

	int df3on_ = strcmp(cmd, df3on);
	int df3off_ = strcmp(cmd, df3off);

	int df4on_ = strcmp(cmd, df4on);
	int df4off_ = strcmp(cmd, df4off);

	int df5on_ = strcmp(cmd, df5on);
	int df5off_ = strcmp(cmd, df5off);

	int df6on_ = strcmp(cmd, df6on);
	int df6off_ = strcmp(cmd, df6off);

	int df7on_ = strcmp(cmd, df7on);
	int df7off_ = strcmp(cmd, df7off);

	int df8on_ = strcmp(cmd, df8on);
	int df8off_ = strcmp(cmd, df8off);

	int df9on_ = strcmp(cmd, df9on);
	int df9off_ = strcmp(cmd, df9off);

	int df10on_ = strcmp(cmd, df10on);
	int df10off_ = strcmp(cmd, df10off);

	int df11on_ = strcmp(cmd, df11on);
	int df11off_ = strcmp(cmd, df11off);

	int df12on_ = strcmp(cmd, df12on);
	int df12off_ = strcmp(cmd, df12off);

		
		// DF1
		if(df1on_ == 0){
			menu_execute(df_1on,0);
		}

		else if(df1off_ == 0){
			menu_execute (df_1off,0);
		}

		// DF2
		
		else if(df2on_ == 0){
			menu_execute(df_2on,0);
		}

		
		else if(df2off_ == 0){
			menu_execute(df_2off,0);
		}

		// DF3
		
		else if(df3on_ == 0){
			menu_execute(df_3on,0);
		}

		
		else if(df3off_ == 0){
			menu_execute(df_3off,0);
		}

		// DF4
		
		else if(df4on_ == 0){
			menu_execute(df_4on,0);
		}

		
		else if(df4off_ == 0){
			menu_execute(df_4off,0);
		}

		// DF5
		
		else if(df5on_ == 0){
			menu_execute(df_5on,0);
		}

		
		else if(df5off_ == 0){
			menu_execute(df_5off,0);
		}

		// DF6
		
		else if(df6on_ == 0){
			menu_execute(df_6on,0);
		}

		
		else if(df6off_ == 0){
			menu_execute(df_6off,0);
		}

		// DF7
		
		else if(df7on_ == 0){
			menu_execute(df_7on,0);
		}

		
		else if(df7off_ == 0){
			menu_execute(df_7off,0);
		}

		// DF8
		
		else if(df8on_ == 0){
			menu_execute(df_8on,0);
		}

		
		else if(df8off_ == 0){
			menu_execute(df_8off,0);
		}

		// DF9
		
		else if(df9on_ == 0){
			menu_execute(df_9on,0);
		}

		
		else if(df9off_ == 0){
			menu_execute(df_9off,0);
		}

		// DF10
		
		else if(df10on_ == 0){
			menu_execute(df_10on,0);
		}

		
		else if(df10off_ == 0){
			menu_execute(df_10off,0);
		}

		// DF11
		
		else if(df11on_ == 0){
			menu_execute(df_11on,0);
		}

		
		else if(df11off_ == 0){
			menu_execute(df_11off,0);
		}

		// DF12
		
		else if(df12on_ == 0){
			menu_execute(df_12on,0);
		}

		
		else if(df12off_ == 0){
			menu_execute(df_12off,0);
		}

	
	else {menu_execute(df_inval,0);}
	return (NULL);
	
}

void
menu(char *args)
{
	char buf[64];
	char *dbarg;

	menu_execute(args, 1);

	while (1) {
		kprintf("OS/161 kernel [? for menu]: ");
		kgets(buf, sizeof(buf));

		if(buf[0] == 'd' && buf[1] == 'f'){
			//kprintf("df processing function called \n");
			dbarg = db_proc(buf);
		//	menu_execute(dbarg,0);
		 
		}	
		else{
		menu_execute(buf, 0);
		}
	}
}
