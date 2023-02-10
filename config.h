/* See LICENSE file for copyright and license details. */

#define TERMINAL "st"
#define TERMCLASS "St"
#define BROWSER "firefox"

#define XK_XF86MonBrightnessDown 0x1008ff03
#define XK_XF86MonBrightnessUp 0x1008ff02
#define XK_XF86AudioMute 0x1008ff12
#define XK_XF86AudioLowerVolume 0x1008ff11
#define XK_XF86AudioRaiseVolume 0x1008ff13
#define XK_XF86AudioMicMute 0x1008ffb2
#define XK_XF86RFKill 0x1008ffb5

#include "movestack.c"

/* appearance */
static unsigned int borderpx  = 1;        /* border pixel of windows */
static unsigned int snap      = 32;       /* snap pixel */
static unsigned int gappih    = 20;       /* horiz inner gap between windows */
static unsigned int gappiv    = 10;       /* vert inner gap between windows */
static unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static unsigned int gappov    = 30;       /* vert outer gap between windows and screen edge */
static int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static int showbar            = 1;        /* 0 means no bar */
static int topbar             = 1;        /* 0 means bottom bar */
static char font[]            = "monospace:size=10";
static char dmenufont[]       = "monospace:size=10";
static const char *fonts[]    = { font };
static char normbgcolor[]     = "#222222";
static char normbordercolor[] = "#444444";
static char normfgcolor[]     = "#bbbbbb";
static char selfgcolor[]      = "#eeeeee";
static char selbordercolor[]  = "#005577";
static char selbgcolor[]      = "#005577";
static const unsigned int baralpha    = 0xd0;
static const unsigned int borderalpha = OPAQUE;
static const char *colors[][3] = {
	/*                fg           bg           border   */
	[SchemeNorm]  = { normfgcolor, normbgcolor, normbordercolor },
	[SchemeSel]   = { selfgcolor,  selbgcolor,  selbordercolor  },
	[SchemeTitle] = { selfgcolor,  normbgcolor, normbordercolor },
};
static const unsigned int alphas[][3] = {
	/*                fg      bg        border     */
	[SchemeNorm]  = { OPAQUE, baralpha, borderalpha },
	[SchemeSel]   = { OPAQUE, baralpha, borderalpha },
	[SchemeTitle] = { OPAQUE, baralpha, borderalpha },
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

/* staticstatus */
static int statmonval = 0;

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class         instance    title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",        NULL,       NULL,           0,         1,          0,           0,        -1 },
	{ "Dragon",      NULL,       NULL,           0,         1,          0,           1,        -1 },
	{ TERMCLASS,     NULL,       NULL,           0,         0,          1,           0,        -1 },
	{ TERMCLASS,     "floating", NULL,           0,         1,          0,           0,        -1 },
	{ NULL,          NULL,       "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
	{ TERMCLASS,     "spterm",   NULL,           SPTAG(0),  1,          1,           0,        -1 },
	{ TERMCLASS,     "spnote",   NULL,           SPTAG(1),  1,          1,           0,        -1 },
	{ TERMCLASS,     "spcalc",   NULL,           SPTAG(2),  1,          1,           0,        -1 },
};

/* layout(s) */
static float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "TTT",      bstack },
	{ "[@]",      spiral },
	{ "[\\]",     dwindle },
	{ "[D]",      deck },
	{ "[M]",      monocle },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ NULL,       NULL },
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
static const char *dmenucmd[] = { "dmenu_run", "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { TERMINAL, NULL };

static const Key keys[] = {
	/* modifier                     key                       function                argument */
	{ MODKEY,                       XK_Return,                spawn,                  { .v = termcmd } },
	{ MODKEY,                       XK_r,                     spawn,                  { .v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_r,                     spawn,                  SPAWN("dmenuflatpakrun") },
	{ MODKEY,                       XK_b,                     togglebar,              {0} },
	{ MODKEY,                       XK_j,                     focusstack,             { .i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,             { .i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,                     movestack,              { .i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,                     movestack,              { .i = -1 } },
	{ MODKEY,                       XK_o,                     incnmaster,             { .i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,                     incnmaster,             { .i = -1 } },
	{ MODKEY,                       XK_h,                     setmfact,               { .f = -0.05 } },
	{ MODKEY,                       XK_l,                     setmfact,               { .f = +0.05 } },
	{ MODKEY,                       XK_space,                 zoom,                   {0} },
	{ MODKEY|ControlMask,           XK_space,                 focusmaster,            {0} },
	{ MODKEY|ShiftMask,             XK_space,                 togglefloating,         {0} },
	{ MODKEY,                       XK_s,                     togglesticky,           {0} },
	{ MODKEY|ShiftMask,             XK_s,                     togglealwaysontop,      {0} },
  { MODKEY|ControlMask,           XK_f,                     togglecanfocusfloating, {0} },
  { MODKEY|ControlMask|ShiftMask, XK_f,                     resetcanfocusfloating,  {0} },
	{ MODKEY,                       XK_f,                     togglefullscr,          {0} },
	{ MODKEY,                       XK_a,                     togglegaps,             {0} },
	{ MODKEY|ControlMask,           XK_a,                     togglesmartgaps,        {0} },
	{ MODKEY|ShiftMask,             XK_a,                     defaultgaps,            {0} },
	{ MODKEY,                       XK_z,                     incrgaps,               { .i = +3 } },
	{ MODKEY|ShiftMask,             XK_z,                     incrgaps,               { .i = -3 } },
	{ MODKEY,                       XK_Tab,                   view,                   {0} },
	{ MODKEY|ShiftMask,             XK_c,                     killclient,             {0} },
	{ MODKEY,                       XK_backslash,             setlayout,              {0} },
	{ MODKEY,                       XK_t,                     setlayout,              { .v = &layouts[0] } }, /* tile */
	{ MODKEY|ShiftMask,             XK_t,                     setlayout,              { .v = &layouts[1] } }, /* bstack */
	{ MODKEY,                       XK_y,                     setlayout,              { .v = &layouts[2] } }, /* spiral */
	{ MODKEY|ShiftMask,             XK_y,                     setlayout,              { .v = &layouts[3] } }, /* dwindle */
	{ MODKEY,                       XK_u,                     setlayout,              { .v = &layouts[4] } }, /* deck */
	{ MODKEY|ShiftMask,             XK_u,                     setlayout,              { .v = &layouts[5] } }, /* monocle */
	{ MODKEY,                       XK_i,                     setlayout,              { .v = &layouts[6] } }, /* centeredmaster */
	{ MODKEY|ShiftMask,             XK_i,                     setlayout,              { .v = &layouts[7] } }, /* centeredfloatingmaster */
	{ MODKEY|ShiftMask,             XK_f,                     setlayout,              { .v = &layouts[8] } }, /* float */
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
	{ ALTKEY|ShiftMask,             XK_f,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon", "$SHELL", "nnn", "-Rrn") },
	{ ALTKEY|ShiftMask,             XK_m,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon", "$SHELL", "ncmpcpp") },
	{ ALTKEY|ShiftMask,             XK_e,                     spawn,                  SPAWN(TERMINAL, "-e", "bicon", "$SHELL", "neomutt") },
	{ ALTKEY|ShiftMask,             XK_b,                     spawn,                  SPAWN(BROWSER) },
	{ ALTKEY|ShiftMask,             XK_i,                     spawn,                  SPAWN("dmenuhandler") },
	{ ALTKEY|ShiftMask,             XK_o,                     spawn,                  SPAWN("clipmenu") },
	{ ALTKEY|ShiftMask,             XK_p,                     spawn,                  SPAWN("passmenu") },
	{ ALTKEY|ShiftMask,             XK_s,                     spawn,                  SPAWN("dmenuspt") },
	{ ALTKEY|ShiftMask,             XK_equal,                 spawn,                  SPAWN("volumectl", "up") },
	{ ALTKEY|ShiftMask,             XK_minus,                 spawn,                  SPAWN("volumectl", "down") },
	{ ALTKEY|ShiftMask,             XK_BackSpace,             spawn,                  SPAWN("volumectl", "toggle") },
	{ ALTKEY|ShiftMask,             XK_period,                spawn,                  SPAWN("mpc", "-q", "next") },
	{ ALTKEY|ShiftMask,             XK_comma,                 spawn,                  SPAWN("mpc", "-q", "prev") },
	{ ALTKEY|ShiftMask,             XK_space,                 spawn,                  SPAWN("mpc", "-q", "toggle") },
	{ ALTKEY|ShiftMask,             XK_slash,                 spawn,                  SPAWN("mpc", "-q", "seek", "0") },
	{ ALTKEY|ShiftMask,             XK_backslash,             spawn,                  SHCMD("dunstctl set-paused toggle; pkill -x -RTMIN+6 dwmblocks") },
	{ ALTKEY|ShiftMask,             XK_bracketleft,           spawn,                  SPAWN("dunstctl", "history-pop") },
	{ ALTKEY|ShiftMask,             XK_bracketright,          spawn,                  SPAWN("dunstctl", "close-all") },
	{ ALTKEY,                       XK_F1,                    spawn,                  SPAWN("tmuxx", "-") },
	{ ALTKEY,                       XK_F2,                    spawn,                  SPAWN("dmenumount") },
	{ ALTKEY|ShiftMask,             XK_F2,                    spawn,                  SPAWN("dmenuumount") },
	{ ALTKEY,                       XK_F3,                    spawn,                  SPAWN("redshift", "-P", "-O", "4500", "-g", "0.95") },
	{ ALTKEY,                       XK_F4,                    spawn,                  SPAWN("redshift", "-x") },
	{ ALTKEY,                       XK_F5,                    spawn,                  SPAWN("playerctl", "-a", "play-pause") },
	{ ALTKEY,                       XK_F7,                    spawn,                  SHCMD(TERMINAL " -n st-floating -e pulsemixer; pkill -x -RTMIN+7 dwmblocks") },
	{ ALTKEY,                       XK_F9,                    spawn,                  SPAWN("dmenuscreenshot") },
	{ ALTKEY|ShiftMask,             XK_F9,                    spawn,                  SPAWN("dmenurecord") },
	{ ALTKEY|ControlMask,           XK_F9,                    spawn,                  SPAWN("dmenurecord", "stop") },
	{ ALTKEY,                       XK_F11,                   spawn,                  SHCMD(TERMINAL " -n st-floating -e nmtui; pkill -x -RTMIN+5 dwmblocks") },
	{ ALTKEY|ShiftMask,             XK_q,                     spawn,                  SPAWN("dmenushutdown") },
	{ ALTKEY|ControlMask,           XK_q,                     spawn,                  SPAWN("dmenushutdown", "lock") },
	{ 0,                            XK_XF86MonBrightnessDown, spawn,                  SPAWN("xbacklight", "-dec", "10") },
	{ 0,                            XK_XF86MonBrightnessUp,   spawn,                  SPAWN("xbacklight", "-inc", "10") },
	{ 0,                            XK_XF86AudioMute,         spawn,                  SPAWN("volumectl", "toggle") },
	{ 0,                            XK_XF86AudioLowerVolume,  spawn,                  SPAWN("volumectl", "down") },
	{ 0,                            XK_XF86AudioRaiseVolume,  spawn,                  SPAWN("volumectl", "up") },
	{ 0,                            XK_XF86AudioMicMute,      spawn,                  SPAWN("amixer", "set", "Capture", "toggle") },
	// { 0,                            XK_XF86RFKill,            spawn,                  SPAWN("rfkill", "toggle", "all") },

	/*
	{ MODKEY|Mod1Mask,              XK_u,                     incrgaps,               { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_u,                     incrgaps,               { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_i,                     incrigaps,              { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_i,                     incrigaps,              { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_o,                     incrogaps,              { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_o,                     incrogaps,              { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_6,                     incrihgaps,             { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_6,                     incrihgaps,             { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_7,                     incrivgaps,             { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_7,                     incrivgaps,             { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_8,                     incrohgaps,             { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_8,                     incrohgaps,             { .i = -1 } },
	{ MODKEY|Mod1Mask,              XK_9,                     incrovgaps,             { .i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_9,                     incrovgaps,             { .i = -1 } },
	*/
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button3,        zoom,           {0} },
	{ ClkStatusText,        0,              Button3,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/*
 * Xresources preferences to load at startup
 */
static const ResourcePref resources[] = {
	{ "font",               STRING,  &font },
	{ "dmenufont",          STRING,  &dmenufont },
	{ "background",         STRING,  &normbgcolor },
	{ "backgroundSel",      STRING,  &selbgcolor },
	{ "forground",          STRING,  &normfgcolor },
	{ "forgroundSel",       STRING,  &selfgcolor },
	{ "background",         STRING,  &normbordercolor },
	{ "backgroundSel",      STRING,  &selbordercolor },
	{ "borderpx",           INTEGER, &borderpx },
	{ "snap",               INTEGER, &snap },
	{ "showbar",            INTEGER, &showbar },
	{ "topbar",             INTEGER, &topbar },
	{ "nmaster",            INTEGER, &nmaster },
	{ "resizehints",        INTEGER, &resizehints },
	{ "mfact",              FLOAT,   &mfact },
	{ "gappih",             INTEGER, &gappih },
	{ "gappiv",             INTEGER, &gappiv },
	{ "gappoh",             INTEGER, &gappoh },
	{ "gappov",             INTEGER, &gappov },
	{ "smartgaps",          INTEGER, &smartgaps },
	{ "swallowfloating",    INTEGER, &swallowfloating },
	{ "statusmon",          INTEGER, &statmonval },
};
