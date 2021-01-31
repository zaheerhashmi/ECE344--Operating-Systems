

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



	while (1) {
		kprintf("OS/161 kernel [? for menu]: ");
		kgets(buf, sizeof(buf));
		
		if(strcmp(buf[0],"d") == 0 && strcmp(buf[1],"f") == 0){
			kprintf("%s \n",buf);
		}


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


static
int
cmd_df1on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 1;
	return 0;
}

static
int
cmd_df2on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 2;
	return 0;
}

static
int
cmd_df3on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 4;
	return 0;
}

static
int
cmd_df4on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 8;
	return 0;
}

static
int
cmd_df5on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 10;
	return 0;
}

static
int
cmd_df6on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 20;
	return 0;
}

static
int
cmd_df7on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 40;
	return 0;
}

static
int
cmd_df8on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 80;
	return 0;
}

static
int
cmd_df9on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 100;
	return 0;
}

static
int
cmd_df10on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 200;
	return 0;
}

static
int
cmd_df11on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 400;
	return 0;
}

static
int
cmd_df12on(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 800;
	return 0;
}

//OFF FUNCTIONS//


static
int
cmd_df1off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df2off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df3off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df4off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df5off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df6off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df7off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df8off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df9off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df10off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df11off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}

static
int
cmd_df12off(int n, char **a)
{
	(void)n;
	(void)a;
	dbflags = 0;
	return 0;
}







OS/161 Debug flags
    [df 1 on/off]     DB_LOCORE         [df 7 on/off]     DB_EXEC
    [df 2 on/off]     DB_SYSCALL        [df 8 on/off]     DB_VFS
    [df 3 on/off]     DB_INTERRUPT      [df 9 on/off]     DB_SFS
    [df 4 on/off]     DB_DEVICE         [df 10 on/off]    DB_NET
    [df 5 on/off]     DB_THREADS        [df 11 on/off]    DB_NETFS
    [df 6 on/off]     DB_VM             [df 12 on/off]    DB_KMALLOC

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




