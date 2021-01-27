

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
	return 0;
}

{"df 1 on",cmd_df_1_on},
{"df 1 off",},
{"df 2 on",},
{"df 2 off",},
{"df 3 on",},
{"df 3 off",},
{"df 4 on",},
{"df 4 off",},
{"df 5 on",},
{"df 5 off",},
{"df 6 on",},
{"df 6 off",},
{"df 7 on",},
{"df 7 off",},
{"df 8 on",},
{"df 8 off",},
{"df 9 on",},
{"df 9 off",},
{"df 10 on",},
{"df 10 off",},
{"df 11 on",},
{"df 11 off",},
{"df 12 on",},
{"df 12 off",},

static
int
cmd_df_1_on(int n, char **a)
{
	(void)n;
	(void)a;
    dbflags = DB_LOCORE;
    kprintf("Current value of dbflags is 0x%x \n",dbflags);
	return 0;
}


#define DB_LOCORE      0x001
#define DB_SYSCALL     0x002
#define DB_INTERRUPT   0x004
#define DB_DEVICE      0x008
#define DB_THREADS     0x010
#define DB_VM          0x020
#define DB_EXEC        0x040
#define DB_VFS         0x080
#define DB_SFS         0x100
#define DB_NET         0x200
#define DB_NETFS       0x400
#define DB_KMALLOC     0x800