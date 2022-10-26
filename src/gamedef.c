
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
Modifications for JonoF's port by Jonathon Fowler (jf@jonof.id.au)
*/
//-------------------------------------------------------------------------

#include "duke3d.h"
#include <assert.h>

int conversion = 13;    // by default we think we're 1.3d until compilation informs us otherwise

extern short otherp;

static short total_lines,line_number;
static char checking_ifelse,parsing_state;
static short num_squigilly_brackets;

static short g_i,g_p;
static int g_x;
static int *g_t;
static spritetype *g_sp;

static char compilefile[255] = "(none)";    // file we're currently compiling

enum labeltypes {
    LABEL_ANY    = -1,
    LABEL_DEFINE = 1,
    LABEL_STATE  = 2,
    LABEL_ACTOR  = 4,
    LABEL_ACTION = 8,
    LABEL_AI     = 16,
    LABEL_MOVE   = 32,
};

static const char *labeltypenames[] = {
    "define",
    "state",
    "actor",
    "action",
    "ai",
    "move"
};

static void translatelabeltype(int type, char *buf)
{
    int i;
    
    buf[0] = 0;
    for (i=0;i<6;i++) {
        if (!(type & (1<<i))) continue;
        if (buf[0]) Bstrcat(buf, " or ");
        Bstrcat(buf, labeltypenames[i]);
    }
}

#define NUMKEYWORDS     (int)(sizeof(keyw)/sizeof(keyw[0]))

static const char *keyw[/*NUMKEYWORDS*/] =
{
    "definelevelname",  // 0
    "actor",            // 1    [#]
    "addammo",          // 2    [#]
    "ifrnd",            // 3    [C]
    "enda",             // 4    [:]
    "ifcansee",         // 5    [C]
    "ifhitweapon",      // 6    [#]
    "action",           // 7    [#]
    "ifpdistl",         // 8    [#]
    "ifpdistg",         // 9    [#]
    "else",             // 10   [#]
    "strength",         // 11   [#]
    "break",            // 12   [#]
    "shoot",            // 13   [#]
    "palfrom",          // 14   [#]
    "sound",            // 15   [filename.voc]
    "fall",             // 16   []
    "state",            // 17
    "ends",             // 18
    "define",           // 19
    "//",               // 20
    "ifai",             // 21
    "killit",           // 22
    "addweapon",        // 23
    "ai",               // 24
    "addphealth",       // 25
    "ifdead",           // 26
    "ifsquished",       // 27
    "sizeto",           // 28
    "{",                // 29
    "}",                // 30
    "spawn",            // 31
    "move",             // 32
    "ifwasweapon",      // 33
    "ifaction",         // 34
    "ifactioncount",    // 35
    "resetactioncount", // 36
    "debris",           // 37
    "pstomp",           // 38
    "/*",               // 39
    "cstat",            // 40
    "ifmove",           // 41
    "resetplayer",      // 42
    "ifonwater",        // 43
    "ifinwater",        // 44
    "ifcanshoottarget", // 45
    "ifcount",          // 46
    "resetcount",       // 47
    "addinventory",     // 48
    "ifactornotstayput",// 49
    "hitradius",        // 50
    "ifp",              // 51
    "count",            // 52
    "ifactor",          // 53
    "music",            // 54
    "include",          // 55
    "ifstrength",       // 56
    "definesound",      // 57
    "guts",             // 58
    "ifspawnedby",      // 59
    "gamestartup",      // 60
    "wackplayer",       // 61
    "ifgapzl",          // 62
    "ifhitspace",       // 63
    "ifoutside",        // 64
    "ifmultiplayer",    // 65
    "operate",          // 66
    "ifinspace",        // 67
    "debug",            // 68
    "endofgame",        // 69
    "ifbulletnear",     // 70
    "ifrespawn",        // 71
    "iffloordistl",     // 72
    "ifceilingdistl",   // 73
    "spritepal",        // 74
    "ifpinventory",     // 75
    "betaname",         // 76
    "cactor",           // 77
    "ifphealthl",       // 78
    "definequote",      // 79
    "quote",            // 80
    "ifinouterspace",   // 81
    "ifnotmoving",      // 82
    "respawnhitag",     // 83
    "tip",              // 84
    "ifspritepal",      // 85
    "money",            // 86
    "soundonce",        // 87
    "addkills",         // 88
    "stopsound",        // 89
    "ifawayfromwall",   // 90
    "ifcanseetarget",   // 91
    "globalsound",      // 92
    "lotsofglass",      // 93
    "ifgotweaponce",    // 94
    "getlastpal",       // 95
    "pkick",            // 96
    "mikesnd",          // 97
    "useractor",        // 98
    "sizeat",           // 99
    "addstrength",      // 100   [#]
    "cstator",          // 101
    "mail",             // 102
    "paper",            // 103
    "tossweapon",       // 104
    "sleeptime",        // 105
    "nullop",           // 106
    "definevolumename", // 107
    "defineskillname",  // 108
    "ifnosounds",       // 109
    "clipdist",         // 110
    "ifangdiffl",       // 111
};


short getincangle(short a,short na)
{
    a &= 2047;
    na &= 2047;

    if(klabs(a-na) < 1024)
        return (na-a);
    else
    {
        if(na > 1024) na -= 2048;
        if(a > 1024) a -= 2048;

        na -= 2048;
        a -= 2048;
        return (na-a);
    }
}

char ispecial(char c)
{
    if(c == 0x0a)
    {
        line_number++;
        return 1;
    }

    if(c == ' ' || c == 0x0d)
        return 1;

    return 0;
}

char isaltok(char c)
{
    return ( isalnum(c) || c == '{' || c == '}' || c == '/' || c == '*' || c == '-' || c == '_' || c == '.');
}