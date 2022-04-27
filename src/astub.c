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
Prepared for public release: 05/24/2003 - Charlie Wiederhold, 3D Realms
Modifications for JonoF's port by Jonathon Fowler (jf@jonof.id.au)
*/
//-------------------------------------------------------------------------

/*
********************************************************

 ASTUB.C (c) 1996 Allen H. Blum III

********************************************************


Specs:

Duke Lookup Table

        1  : Blue
        2  : Red
        3  : Normal for Sky
        4  : Black Shadow
        5  :
        6  : Night Vision
        7  : Yellow
        8  : Green

        Duke Sprite

        9  : Blue
        10 : Red
        11 : Green
        12 : Grey
        13 : Ninja
        14 : G.I. Duke
        15 : Brown
    16 : Postal Duke (Dark Blue)

        21 : Blue -> Red
        22 : Blue -> Green
        23 : Blue -> Yellow

********************************************************
*/

#include "build.h"
#include "editor.h"
#include "pragmas.h"
#include "baselayer.h"
#include "names.h"
#include "cache1d.h"

#define TICSPERFRAME 3

//#define VULGARITY



/*#pragma aux setvmode =\
        "int 0x10",\
        parm [eax]\
*/
//#include "water.c"

char *Myname[1]= {"stryker@metronet.com"};

char *defsfilename = "duke3d.def";

static int ototalclock = 0;

static int clockval[16], clockcnt = 0;

#define NUMOPTIONS 9

char option[NUMOPTIONS] = {0,0,0,0,0,0,1,0,0};
int keys[NUMBUILDKEYS] =
    {
        0xc8,0xd0,0xcb,0xcd,0x2a,0x9d,0x1d,0x39,
        0x1e,0x2c,0xd1,0xc9,0x47,0x49,
        0x9c,0x1c,0xd,0xc,0xf,0x45
    };

int nextvoxid = 0;



#define COKE 52

#define MAXHELP2D 9
const char *Help2d[MAXHELP2D]=
        {
    " ' M = Memory",
    " ' 1 = Captions ",
//    " ' 2 = ",
    " ' 3 = Captions Toggle",
//    " ' 4 = MIN FRAMES RATE",
//    " ' 5 = MOTORCYCLE",
    " ' 9 = Swap HI LO",
//    " ' 0 = SHRINK MAP 50",
    " F8  = Current Wall/Sprite",
    " F9  = Current Sector",
    " [   = Search Forward",
    " ]   = Search Backward",
        " ~   = HELP OFF"
        };

#define MAXMODE32D 7
const char *Mode32d[MAXMODE32D]=
        {
    "NONE",
    "SECTORS",
    "WALLS",
    "SPRITES",
    "ALL",
    "ITEMS ONLY",
    "CURRENT SPRITE ONLY"
    };

#define MAXSKILL 5
const char *SKILLMODE[MAXSKILL]=
        {
    "BEGINNER",
    "EASY",
    "NORMAL",
    "NOT EASY",
    "ALL"
    };

#define MAXNOSPRITES 4
const char *ALPHABEASTLOADOMAGA1[MAXNOSPRITES]=
        {
    "DISPLAY ALL SPRITES",
    "NO EFFECTORS",
    "NO ACTORS",
    "NONE"
    };

short MinRate=24, MinD=3;
// CTW - MODIFICATION
// Good to know Allen has changed in all these years. ;)
// CTW END - MODIFICATION
const char *Slow[8]=
        {
        "SALES = 0,000,000  ***********************",
        "100% OF NOTHING IS !! ********************",
        "RENDER IN PROGRESS ***********************",
        "YOUR MOTHER IS A WHORE *******************",
        "YOU SUCK DONKEY **************************",
        "FUCKIN PISS ANT **************************",
        "PISS ANT *********************************",
    "SLOW *************************************"
    };

#define MAXHELP3D 15
const char *Help3d[MAXHELP3D]=
        {
    "3D KEYS HELP",
        " ",
    " F1 = HELP TOGGLE",
    " ' R = FRAMERATE TOGGLE",
    " ' D = SKILL MODE",
    " ' W = TOGGLE SPRITE DISPLAY",
    " ' G = GRAPHIC TOGGLE",
    " ' Y = TOGGLE PURPLE BACKGROUND",
    " ' ENTER = COPY GRAPHIC ONLY",
    " ' T = CHANGE LOTAG",
    " ' H = CHANGE HITAG",
    " ' S = CHANGE SHADE",
    " ' V = CHANGE VISIBILITY",
    " ' C = CHANGE GLOBAL SHADE",
    " ' DEL = CSTAT=0",
        };


/******* VARS ********/

static char tempbuf[1024]; //1024
static int numsprite[MAXSPRITES];
static int multisprite[MAXSPRITES];
static char lo[32];
static const char *levelname;
static short curwall=0,wallpicnum=0,curwallnum=0;
static short cursprite=0,curspritenum=0;
static short cursector_lotag=0,cursectornum=0;
static short search_lotag=0,search_hitag=0;
static unsigned char wallsprite=0;
static unsigned char helpon=0;
static unsigned char on2d3d=0;
//static char onwater=0;
static unsigned char onnames=4;
static unsigned char usedcount=0;
int mousxplc,mousyplc;
int ppointhighlight;
static int counter=0;
unsigned char nosprites=0,purpleon=0,skill=4;
unsigned char framerateon=1,tabgraphic=0;


static unsigned char sidemode=0;
extern int vel, svel, hvel, angvel;
int xvel, yvel, timoff;



void SearchSectorsForward();
void SearchSectorsBackward();
void SpriteName(short spritenum, char *lo2);
int ActorMem(int i);
void PrintStatus(char *string,int num,char x,char y,char color);
void Ver();
void SetBOSS1Palette();
void SetSLIMEPalette();
void SetWATERPalette();
void SetGAMEPalette();
void kensetpalette(unsigned char *vgapal);

void ExtPreLoadMap(void)
{
}

void ExtLoadMap(const char *mapname)
{
	int i;
	int sky=0;
	int j;

	wm_setwindowtitle(mapname);

	// PreCache Wall Tiles
	for(j=0;j<numwalls;j++)
		if(waloff[wall[j].picnum] == 0)
			loadtile(wall[j].picnum);


	// Presize Sprites
	for(j=0;j<MAXSPRITES;j++)
	{
		if(tilesizx[sprite[j].picnum]==0 || tilesizy[sprite[j].picnum]==0)
			sprite[j].picnum=0;

		if(sprite[j].picnum>=20 && sprite[j].picnum<=59)
		{
			if(sprite[j].picnum==26) {sprite[j].xrepeat = 8; sprite[j].yrepeat = 8;}
			else {sprite[j].xrepeat = 32; sprite[j].yrepeat = 32;}
		}
	}

	leveln