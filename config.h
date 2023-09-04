/* See LICENSE file for copyright and license details. */

#define TERMINAL "st"
#define TERMCLASS "st-256color"
#define BROWSER "firefox"

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char font[]            = "monospace:size=10";
static const char dmenufont[]       = "monospace:size=10";
static const char *fonts[]    = { font, "NotoColorEmoji:pixelsize=10:antialias=true:autohint=true" };
static const char normfgcolor[]     = "#d8dee9";
static const char normbgcolor[]     = "#242933";
static const char normbordercolor[] = "#4c566a";
static const char selfgcolor[]      = "#eceff4";
static const char selbgcolor[]      = "#5e81ac";
static const char selbordercolor[]  = "#5e81ac";
static const char *colors[][3] = {
	/*                fg           bg           border   */
	[SchemeNorm]  = { normfgcolor, normbgcolor, normbordercolor },
	[SchemeSel]   = { selfgcolor,  selbgcolor,  selbordercolor  },
	[SchemeTitle] = { selfgcolor,  normbgcolor, normbordercolor },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = { TERMINAL, "-n", "spterm", "-g", "120x34", NULL };
const char *spcmd2[] = { TERMINAL, "-n", "spnote", "-g", "95x25", "-e", "notes", "-l", "24", NULL };
const char *spcmd3[] = { TERMINAL, "-n", "spcalc", "-g", "50x20", "-f", "monospace:size=16", "-e", "bc", "-lq", NULL };
static Sp scratchpads[] = {
	/* name        cmd  */
	{ "spterm",    spcmd1 },
	{ "spnote",    spcmd2 },
	{ "spcalc",    spcmd3 },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance     title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",        NULL,        NULL,           0,         1,          0,           0,        -1 },
	{ "Dragon",      NULL,        NULL,           0,         1,          0,           1,        -1 },
	{ TERMCLASS,     NULL,        NULL,           0,         0,          1,           0,        -1 },
	{ TERMCLASS,     "floatterm", NULL,           0,         1,          0,           0,        -1 },
	{ NULL,          NULL,        "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
	{ TERMCLASS,     "spterm",    NULL,           SPTAG(0),  1,          1,           0,        -1 },
	{ TERMCLASS,     "spnote",    NULL,           SPTAG(1),  1,          1,           0,        -1 },
	{ TERMCLASS,     "spcalc",    NULL,           SPTAG(2),  1,          1,           0,        -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "[M]",      monocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define SPAWN(...) { .v = (const char*[]){ __VA_ARGS__, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { TERMINAL, NULL };

#include <X11/XF86keysym.h>

static const Key keys[] = {
	/* modifier                     key                       function                argument */
	{ MODKEY,                       XK_Return,                spawn,                  { .v = termcmd } },
	{ MODKEY,                       XK_r,                     spawn,                  { .v = dmenucmd } },
	{ MODKEY,                       XK_b,                     togglebar,              {0} },
	{ MODKEY,                       XK_j,                     focusstack,             { .i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,             { .i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,                     pushdown,               {0} },
	{ MODKEY|ShiftMask,             XK_k,                     pushup,                 {0} },
	{ MODKEY,                       XK_o,                     incnmaster,             { .i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,                     incnmaster,             { .i = -1 } },
	{ MODKEY,                       XK_h,                     setmfact,               { .f = -0.05 } },
	{ MODKEY,                       XK_l,                     setmfact,               { .f = +0.05 } },
	{ MODKEY,                       XK_space,                 zoom,                   {0} },
	{ MODKEY|ShiftMask,             XK_space,                 togglefloating,         {0} },
	{ MODKEY,                       XK_s,                     togglesticky,           {0} },
	{ MODKEY,                       XK_f,                     togglefullscr,          {0} },
	{ MODKEY,                       XK_Tab,                   view,                   {0} },
	{ MODKEY|ShiftMask,             XK_c,                     killclient,             {0} },
	{ MODKEY,                       XK_backslash,             setlayout,              {0} },
	{ MODKEY,                       XK_t,                     setlayout,              { .v = &layouts[0] } },
	{ MODKEY,                       XK_m,                     setlayout,              { .v = &layouts[1] } },
	{ MODKEY|ShiftMask,             XK_f,                     setlayout,              { .v = &layouts[2] } },
	{ MODKEY,                       XK_0,                     view,                   { .ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                     tag,                    { .ui = ~0 } },
	{ MODKEY,                       XK_comma,                 focusmon,               { .i = -1 } },
	{ MODKEY,                       XK_period,                focusmon,               { .i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,                 { .i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,                 { .i = +1 } },
	{ MODKEY,                       XK_grave,                 togglescratch,          { .ui = 0 } },
	{ MODKEY,                       XK_apostrophe,            togglescratch,          { .ui = 1 } },
	{ MODKEY,                       XK_equal,                 togglescratch,          { .ui = 2 } },
	// { MODKEY|ShiftMask,             XK_q,                     quit,                   {0} },
	TAGKEYS(                        XK_1,                                             0)
	TAGKEYS(                        XK_2,                                             1)
	TAGKEYS(                        XK_3,                                             2)
	TAGKEYS(                        XK_4,                                             3)
	TAGKEYS(                        XK_5,                                             4)
	TAGKEYS(                        XK_6,                                             5)
	TAGKEYS(                        XK_7,                                             6)
	TAGKEYS(                        XK_8,                                             7)
	TAGKEYS(                        XK_9,                                             8)
	{ ALTKEY|ShiftMask,             XK_f,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon.bin", "nnn", "-Rrn") },
	{ ALTKEY|ShiftMask,             XK_m,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon.bin", "ncmpcpp") },
	{ ALTKEY|ShiftMask,             XK_e,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon.bin", "neomutt") },
	{ ALTKEY|ShiftMask,             XK_b,                     spawn,                  SPAWN(BROWSER) },
	{ ALTKEY|ShiftMask,             XK_i,                     spawn,                  SPAWN("dmenu_handler") },
	{ ALTKEY|ShiftMask,             XK_o,                     spawn,                  SPAWN("clipmenu") },
	{ ALTKEY|ShiftMask,             XK_p,                     spawn,                  SPAWN("passmenu") },
	{ ALTKEY|ShiftMask,             XK_period,                spawn,                  SPAWN("mpc", "-q", "next") },
	{ ALTKEY|ShiftMask,             XK_comma,                 spawn,                  SPAWN("mpc", "-q", "prev") },
	{ ALTKEY|ShiftMask,             XK_space,                 spawn,                  SPAWN("mpc", "-q", "toggle") },
	{ ALTKEY|ShiftMask,             XK_slash,                 spawn,                  SPAWN("mpc", "-q", "seek", "0") },
	{ ALTKEY|ShiftMask,             XK_backslash,             spawn,                  SPAWN("dunstctl", "close-all") },
	{ ALTKEY,                       XK_F1,                    spawn,                  SPAWN("dmenu_projects") },
	{ ALTKEY,                       XK_F2,                    spawn,                  SPAWN("dmenu_mount") },
	{ ALTKEY|ShiftMask,             XK_F2,                    spawn,                  SPAWN("dmenu_umount") },
	{ ALTKEY,                       XK_F3,                    spawn,                  SPAWN("redshift", "-P", "-O", "4500", "-g", "0.95") },
	{ ALTKEY,                       XK_F4,                    spawn,                  SPAWN("redshift", "-x") },
	{ ALTKEY,                       XK_F7,                    spawn,                  SHCMD(TERMINAL " -n floatterm -e pulsemixer; pkill -x -RTMIN+7 dwmblocks") },
	{ ALTKEY,                       XK_F11,                   spawn,                  SHCMD(TERMINAL " -n floatterm -e wpa_cli; pkill -x -RTMIN+5 dwmblocks") },
	{ 0,                            XK_Print,                 spawn,                  SPAWN("dmenu_screenshot") },
	{ ALTKEY,                       XK_Print,                 spawn,                  SPAWN("dmenu_record") },
	{ ALTKEY|ControlMask,           XK_Print,                 spawn,                  SPAWN("dmenu_record", "stop") },
	{ MODKEY,                       XK_q,                     spawn,                  SPAWN("dmenu_sysact") },
	{ ALTKEY|ShiftMask,             XK_q,                     spawn,                  SPAWN("dmenu_sysact", "lock") },
	{ ALTKEY|ShiftMask,             XK_equal,                 spawn,                  SPAWN("volumectl", "inc") },
	{ ALTKEY|ShiftMask,             XK_minus,                 spawn,                  SPAWN("volumectl", "dec") },
	{ ALTKEY|ShiftMask,             XK_BackSpace,             spawn,                  SPAWN("volumectl", "toggle") },
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn,                  SPAWN("volumectl", "inc") },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn,                  SPAWN("volumectl", "dec") },
	{ 0,                            XF86XK_AudioMute,         spawn,                  SPAWN("volumectl", "toggle") },
	{ 0,                            XF86XK_AudioMicMute,      spawn,                  SPAWN("amixer", "set", "Capture", "toggle") },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn,                  SPAWN("screenlightctl", "inc") },
	{ 0,                            XF86XK_MonBrightnessDown, spawn,                  SPAWN("screenlightctl", "dec") },
	// { 0,                            XF86XK_RFKill,            spawn,                  SPAWN("rfkill", "toggle", "all") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkStatusText,        0,              Button3,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
