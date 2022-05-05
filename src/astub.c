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

	levelname=mapname;
	pskyoff[0]=0;
	for(i=0;i<8;i++) pskyoff[i]=0;

	for(i=0;i<numsectors;i++)
	{
		switch(sector[i].ceilingpicnum)
		{
			case MOONSKY1 :
			case BIGORBIT1 : // orbit
			case LA : // la city
				sky=sector[i].ceilingpicnum;
				break;
		}
	}

	switch(sky)
	{
		case MOONSKY1 :
			//        earth          mountian   mountain         sun
			pskyoff[6]=1; pskyoff[1]=2; pskyoff[4]=2; pskyoff[2]=3;
			break;

		case BIGORBIT1 : // orbit
			//       earth1         2           3           moon/sun
			pskyoff[5]=1; pskyoff[6]=2; pskyoff[7]=3; pskyoff[2]=4;
			break;

		case LA : // la city
			//       earth1         2           3           moon/sun
			pskyoff[0]=1; pskyoff[1]=2; pskyoff[2]=1; pskyoff[3]=3;
			pskyoff[4]=4; pskyoff[5]=0; pskyoff[6]=2; pskyoff[7]=3;
			break;
	}

	pskybits=3;
	parallaxtype=0;
}

void overwritesprite (int thex, int they, short tilenum,signed char shade, unsigned char stat, unsigned char dapalnum)
{
        rotatesprite(thex<<16,they<<16,65536L,(stat&8)<<7,tilenum,shade,dapalnum,
                (((stat&1)^1)<<4)+(stat&2)+((stat&4)>>2)+(((stat&16)>>2)^((stat&8)>>1)),
                windowx1,windowy1,windowx2,windowy2);
}

void putsprite (int thex, int they, int zoom, short rot, short tilenum, signed char shade, unsigned char dapalnum)
{char stat=0;
    rotatesprite(thex<<16,they<<16,65536L-zoom,(rot+(stat&8))<<7,tilenum,shade,dapalnum,
                (((stat&1)^1)<<4)+(stat&2)+((stat&4)>>2)+(((stat&16)>>2)^((stat&8)>>1)),
                windowx1,windowy1,windowx2,windowy2);
}


void ExtPreSaveMap(void)
{
}

void ExtSaveMap(const char *mapname)
{
	wm_setwindowtitle(mapname);

	saveboard("backup.map",&posx,&posy,&posz,&ang,&cursectnum);
}

const char *ExtGetSectorCaption(short sectnum)
{
	if(!(onnames==1 || onnames==4))
	{
		tempbuf[0] = 0;
		return(tempbuf);
	}

	if ((sector[sectnum].lotag|sector[sectnum].hitag) == 0)
	{
		tempbuf[0] = 0;
	}
	else
	{
		switch((unsigned short)sector[sectnum].lotag)
		{
//       case 1 : Bsprintf(lo,"WATER"); break;
//       case 2 : Bsprintf(lo,"UNDERWATER"); break;
//       case 3 : Bsprintf(lo,"EARTHQUAKE"); break;
			default : Bsprintf(lo,"%hu",(unsigned short)sector[sectnum].lotag); break;
		}
		Bsprintf(tempbuf,"%hu,%s", (unsigned short)sector[sectnum].hitag, lo);
        }
	return(tempbuf);
}

const char *ExtGetWallCaption(short wallnum)
{
    int i=0;

    if(!(onnames==2 || onnames==4))
    {
        tempbuf[0] = 0;
        return(tempbuf);
    }


    if(keystatus[0x57]>0) // f11   Grab pic 0x4e +
    {
        wallpicnum = wall[curwall].picnum;
        Bsprintf(tempbuf,"Grabbed Wall Picnum %d",wallpicnum);
        printmessage16(tempbuf);
    }


    // HERE

    if(keystatus[0x1a]>0) // [     search backward
    {
       keystatus[0x1a]=0;
        if(wallsprite==0)
        { SearchSectorsBackward();
        } else

        if(wallsprite==1)
        {
            if(curwallnum>0) curwallnum--;
            for(i=curwallnum;i>=0;i--)
            {
                if(
                    (wall[i].picnum==wall[curwall].picnum)
                    &&((search_lotag==0)||
                      (search_lotag!=0 && search_lotag==wall[i].lotag))
                    &&((search_hitag==0)||
                      (search_hitag!=0 && search_hitag==wall[i].hitag))
                  )
                {
                    posx=(wall[i].x)-(( (wall[i].x)-(wall[wall[i].point2].x) )/2);
                    posy=(wall[i].y)-(( (wall[i].y)-(wall[wall[i].point2].y) )/2);
                    printmessage16("< Wall Search : Found");
//                    curwallnum--;
                    keystatus[0x1a]=0;
                    return(tempbuf);
                }
                curwallnum--;
            }
            printmessage16("< Wall Search : none");
        } else

        if(wallsprite==2)
        {
            if(curspritenum>0) curspritenum--;
            for(i=curspritenum;i>=0;i--)
            {

                if(
                    (sprite[i].picnum==sprite[cursprite].picnum &&
                        sprite[i].statnum==0 )
                    &&((search_lotag==0)||
                      (search_lotag!=0 && search_lotag==sprite[i].lotag))
                    &&((search_hitag==0)||
                      (search_hitag!=0 && search_hitag==sprite[i].hitag))
                  )
                {
                    posx=sprite[i].x;
                    posy=sprite[i].y;
                    ang= sprite[i].ang;
                    printmessage16("< Sprite Search : Found");
//                    curspritenum--;
                    keystatus[0x1a]=0;
                    return(tempbuf);
                }
                curspritenum--;
            }
            printmessage16("< Sprite Search : none");
        }
    }


    if(keystatus[0x1b]>0) // ]     search forward
    {
       keystatus[0x1b]=0;
        if(wallsprite==0)
        { SearchSectorsForward();
        } else

        if(wallsprite==1)
        {
            if(curwallnum<MAXWALLS) curwallnum++;
            for(i=curwallnum;i<=MAXWALLS;i++)
            {
                if(
                    (wall[i].picnum==wall[curwall].picnum)
                    &&((search_lotag==0)||
                      (search_lotag!=0 && search_lotag==wall[i].lotag))
                    &&((search_hitag==0)||
                      (search_hitag!=0 && search_hitag==wall[i].hitag))
                  )
                {
                    posx=(wall[i].x)-(( (wall[i].x)-(wall[wall[i].point2].x) )/2);
                    posy=(wall[i].y)-(( (wall[i].y)-(wall[wall[i].point2].y) )/2);
                    printmessage16("> Wall Search : Found");
//                    curwallnum++;
                    keystatus[0x1b]=0;
                    return(tempbuf);
                }
                curwallnum++;
            }
            printmessage16("> Wall Search : none");
        } else

        if(wallsprite==2)
        {
            if(curspritenum<MAXSPRITES) curspritenum++;
            for(i=curspritenum;i<=MAXSPRITES;i++)
            {
                if(
                    (sprite[i].picnum==sprite[cursprite].picnum &&
                        sprite[i].statnum==0 )
                    &&((search_lotag==0)||
                      (search_lotag!=0 && search_lotag==sprite[i].lotag))
                    &&((search_hitag==0)||
                      (search_hitag!=0 && search_hitag==sprite[i].hitag))
                  )
                {
                    posx=sprite[i].x;
                    posy=sprite[i].y;
                    ang= sprite[i].ang;
                    printmessage16("> Sprite Search : Found");
//                    curspritenum++;
                    keystatus[0x1b]=0;
                    return(tempbuf);
                }
                curspritenum++;
            }
            printmessage16("> Sprite Search : none");
        }
    }


        if ((wall[wallnum].lotag|wall[wallnum].hitag) == 0)
        {
                tempbuf[0] = 0;
        }
        else
        {
                Bsprintf(tempbuf,"%hu,%hu",(unsigned short)wall[wallnum].hitag,
                                                                                  (unsigned short)wall[wallnum].lotag);
        }
        return(tempbuf);
} //end

const char *ExtGetSpriteCaption(short spritenum)
{


    if( onnames!=5 &&
        onnames!=6 &&
        (!(onnames==3 || onnames==4))
      )
    {
        tempbuf[0] = 0;
        return(tempbuf);
    }

    if( onnames==5 &&
        ( ((unsigned short)sprite[spritenum].picnum <= 9 ) ||
        ((unsigned short)sprite[spritenum].picnum == SEENINE )
        )
        )
    { tempbuf[0] = 0; return(tempbuf); }

    if( onnames==6 &&
        (unsigned short)sprite[spritenum].picnum != (unsigned short)sprite[cursprite].picnum
      )
    { tempbuf[0] = 0; return(tempbuf); }

    tempbuf[0] = 0;
    if ((sprite[spritenum].lotag|sprite[spritenum].hitag) == 0)
    {
        SpriteName(spritenum,lo);
        if(lo[0]!=0)
        {
            if(sprite[spritenum].pal==1) Bsprintf(tempbuf,"%s-M",lo);
            else Bsprintf(tempbuf,"%s",lo);
        }
    }
    else
	    /*
        if( (unsigned short)sprite[spritenum].picnum == 175)
        {
            Bsprintf(lo,"%hu",(unsigned short)sprite[spritenum].lotag);
            Bsprintf(tempbuf,"%hu,%s",(unsigned short)sprite[spritenum].hitag,lo);
        }
        else
	*/
            {
                SpriteName(spritenum,lo);
                Bsprintf(tempbuf,"%hu,%hu %s",
                (unsigned short)sprite[spritenum].hitag,
                (unsigned short)sprite[spritenum].lotag,
                lo);
            }
            return(tempbuf);
} //end

//printext16 parameters:
//printext16(int xpos, int ypos, short col, short backcol,
//           char name[82], char fontsize)
//  xpos 0-639   (top left)
//  ypos 0-479   (top left)
//  col 0-15
//  backcol 0-15, -1 is transparent background
//  name
//  fontsize 0=8*8, 1=3*5

//drawline16 parameters:
// drawline16(int x1, int y1, int x2, int y2, char col)
//  x1, x2  0-639
//  y1, y2  0-143  (status bar is 144 high, origin is top-left of STATUS BAR)
//  col     0-15



void TotalMem()
{
    char incache[MAXTILES];
    int i,tottiles,totsprites,totactors;

    memset(incache, 0, sizeof(incache));

	for(i=0;i<numsectors;i++)
	{
		incache[sector[i].ceilingpicnum] = 1;
		incache[sector[i].floorpicnum] = 1;
	}
	for(i=0;i<numwalls;i++)
	{
		incache[wall[i].picnum] = 1;
		if (wall[i].overpicnum >= 0)
			incache[wall[i].overpicnum] = 1;
	}

	tottiles = 0;
	for(i=0;i<MAXTILES;i++)
		if (incache[i] > 0)
			tottiles += tilesizx[i]*tilesizy[i];

    memset(incache, 0, sizeof(incache));

        for(i=0;i<MAXSPRITES;i++)
                if (sprite[i].statnum < MAXSTATUS)
                        incache[sprite[i].picnum] = 1;
        totsprites = 0;
	totactors = 0;

        for(i=0;i<MAXTILES;i++)
    {
                if (incache[i] > 0)
        {
         switch(i)
         {
            case LIZTROOP :
            case LIZTROOPRUNNING :
            case LIZTROOPSTAYPUT :
            case LIZTROOPSHOOT :
            case LIZTROOPJETPACK :
            case LIZTROOPONTOILET :
            case LIZTROOPDUCKING :
                totactors+=ActorMem(LIZTROOP);
                incache[LIZTROOP]=0;
                incache[LIZTROOPRUNNING]=0;
                incache[LIZTROOPSTAYPUT]=0;
                incache[LIZTROOPSHOOT]=0;
                incache[LIZTROOPJETPACK]=0;
                incache[LIZTROOPONTOILET]=0;
                incache[LIZTROOPDUCKING]=0;
                break;
        case OCTABRAIN :
        case OCTABRAINSTAYPUT:
        totactors+=ActorMem(OCTABRAIN);
        incache[OCTABRAIN]=0;
        incache[OCTABRAINSTAYPUT]=0;
                break;
                 case DRONE :
                totactors+=ActorMem(DRONE);
                incache[DRONE]=0;
                break;
        case COMMANDER :
        totactors+=ActorMem(COMMANDER);
        incache[COMMANDER]=0;
                break;
            case RECON :
                totactors+=ActorMem(RECON);
                incache[RECON]=0;
                break;
            case PIGCOP :
        totactors+=ActorMem(COMMANDER);
                incache[PIGCOP]=0;
                break;
            case LIZMAN :
        case LIZMANSTAYPUT :
            case LIZMANSPITTING :
            case LIZMANFEEDING :
            case LIZMANJUMP :
                totactors+=ActorMem(LIZMAN);
                incache[LIZMAN]=0;
        incache[LIZMANSTAYPUT]=0;
                incache[LIZMANSPITTING]=0;
                incache[LIZMANFEEDING]=0;
   