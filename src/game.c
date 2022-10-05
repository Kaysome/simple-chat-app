
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

#include "scriplib.h"
#include "mathutil.h"
#include "mouse.h"

#include "osd.h"
#include "osdcmds.h"
#include "startwin.h"
#include "grpscan.h"

#include "util_lib.h"


#define TIMERUPDATESIZ 32

int cameradist = 0, cameraclock = 0;
int sbarscale = 65536;
unsigned char playerswhenstarted;
char qe;

static int32 CommandSetup = 0;
static int32 CommandSoundToggleOff = 0;
static int32 CommandMusicToggleOff = 0;
static char const *CommandMap = NULL;
static char const *CommandName = NULL;
int32 CommandWeaponChoice = 0;
static struct strllist {
    struct strllist *next;
    char *str;
} *CommandPaths = NULL, *CommandGrps = NULL;
static int CommandFakeMulti = 0;

char duke3dgrp[BMAX_PATH+1] = "duke3d.grp";
char defaultconfilename[BMAX_PATH] = "game.con";
char const *confilename = defaultconfilename;

char boardfilename[BMAX_PATH] = {0};
unsigned char waterpal[768], slimepal[768], titlepal[768], drealms[768], endingpal[768];
char firstdemofile[80] = { '\0' };

static int netparam = 0;    // Index into argv of the first -net argument
static int endnetparam = 0; // Index into argv following the final -net parameter.
static int netsuccess = 0;  // Outcome of calling initmultiplayersparms().

void setstatusbarscale(int sc)
{
    sbarscale = (sc << 16) / 8;
}

void statusbarsprite(int x, int y, int z, short a, short picnum, signed char dashade,
    unsigned char dapalnum, unsigned char dastat, int cx1, int cy1, int cx2, int cy2)
{
    int sx, sy, sc;

    if (ud.screen_size == 4) sx = x * sbarscale;
    else sx = (160l<<16) - (160l - x) * sbarscale;

    sy = (200l<<16) - (200l - y) * sbarscale;
    sc = mulscale16(z, sbarscale);

    rotatesprite(sx,sy,sc,a,picnum,dashade,dapalnum,dastat,cx1,cy1,cx2,cy2);
}

void fragbarsprite(int x, int y, int z, short a, short picnum, signed char dashade,
    unsigned char dapalnum, unsigned char dastat, int cx1, int cy1, int cx2, int cy2)
{
    int sx, sy, sc;

    sx = (160l<<16) - (160l - x) * sbarscale;
    sy = y * sbarscale;
    sc = mulscale16(z, sbarscale);

    rotatesprite(sx,sy,sc,a,picnum,dashade,dapalnum,dastat,cx1,cy1,cx2,cy2);
}

void patchstatusbar(int x1, int y1, int x2, int y2)
{
    int ty;
    int clx1,cly1,clx2,cly2,clofx,clofy;
    int barw;

    ty = tilesizy[BOTTOMSTATUSBAR];

    if (x1 == 0 && y1 == 0 && x2 == 320 && y2 == 200)
    {
        clofx = clofy = 0;
        clx1 = cly1 = 0;
        clx2 = xdim; cly2 = ydim;
    }
    else
    {
        barw = scale(320<<16, ydim * sbarscale, 200*pixelaspect);
        clofx = ((xdim<<16) - barw)>>1;
        clofy = (ydim<<16) - scale(200<<16, ydim * sbarscale, 200<<16) + 32768;
        clx1 = scale(x1<<16, ydim * sbarscale, 200*pixelaspect);
        cly1 = scale(y1<<16, ydim * sbarscale, 200<<16);
        clx2 = scale(x2<<16, ydim * sbarscale, 200*pixelaspect);
        cly2 = scale(y2<<16, ydim * sbarscale, 200<<16);
        clx1 = (clx1 + clofx)>>16;
        cly1 = (cly1 + clofy)>>16;
        clx2 = (clx2 + clofx)>>16;
        cly2 = (cly2 + clofy)>>16;
    }

    statusbarsprite(0,200-ty,65536,0,BOTTOMSTATUSBAR,4,0,
                 10+16+64+128,clx1,cly1,clx2-1,cly2-1);
}

int recfilep,totalreccnt;
char debug_on = 0,actor_tog = 0,*rtsptr,memorycheckoveride=0;



extern int32 numlumps;

FILE *frecfilep = (FILE *)NULL;
void pitch_test( void );

unsigned char restorepalette,screencapt,nomorelogohack;
int sendmessagecommand = -1;

static char const *duke3ddef = "duke3d.def";

//task *TimerPtr=NULL;

extern int lastvisinc;

void setgamepalette(struct player_struct *player, unsigned char *pal, int set)
{
    if (player != &ps[screenpeek]) {
        // another head
        player->palette = pal;
        return;
    }

    if (!POLYMOST_RENDERMODE_POLYGL()) {
        // 8-bit mode
        setbrightness(ud.brightness>>2, pal, set);
        //pub = pus = NUMPAGES;
    }
#if USE_POLYMOST && USE_OPENGL
    else if (pal == palette || pal == waterpal || pal == slimepal) {
        // only reset the palette to normal if the previous one wasn't handled by tinting
        polymosttexfullbright = 240;
        if (player->palette != palette && player->palette != waterpal && player->palette != slimepal)
            setbrightness(ud.brightness>>2, palette, set);
        else setpalettefade(0,0,0,0);
    } else {
        polymosttexfullbright = 256;
        setbrightness(ud.brightness>>2, pal, set);
    }
#endif
    player->palette = pal;
}

int gametext(int x,int y,const char *t,signed char s,short dabits)
{
    short ac,newx;
    char centre;
    const char *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac];
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,dabits,0,0,xdim-1,ydim-1);

        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac];

        t++;
    }

    return (x);
}

int gametextpal(int x,int y,const char *t,signed char s,unsigned char p)
{
    short ac,newx;
    char centre;
    const char *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tilesizx[ac];
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,2+8+16,0,0,xdim-1,ydim-1);
        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tilesizx[ac];

        t++;
    }

    return (x);
}

int gametextpart(int x,int y,const char *t,signed char s,short p)
{
    short ac,newx, cnt;
    char centre;
    const char *oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;
    cnt = 0;

    if(centre)
    {
        while(*t)
        {
            if(cnt == p) break;

            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            newx += tilesizx[ac];
            t++;
            cnt++;

        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    cnt = 0;
    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

        if(cnt == p)
        {
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,1,2+8+16,0,0,xdim-1,ydim-1);
            break;
        }
        else
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

        x += tilesizx[ac];

        t++;
        cnt++;
    }

    return (x);
}

#define MINITEXT_SBAR 0x0100
#define MINITEXT_FBAR 0x0200

int minitext(int x,int y,const char *t,unsigned char p,short sb)
{
    short ac,dastat;
    char ch;

    dastat = sb & 255;

    while(*t)
    {
        ch = Btoupper(*t);
        if(ch == 32) {x+=5;t++;continue;}
        else ac = ch - '!' + MINIFONT;

        if (sb & MINITEXT_SBAR) statusbarsprite(x,y,65536L,0,ac,0,p,dastat,0,0,xdim-1,ydim-1);
        else if (sb & MINITEXT_FBAR) fragbarsprite(x,y,65536L,0,ac,0,p,dastat,0,0,xdim-1,ydim-1);
        else rotatesprite(x<<16,y<<16,65536L,0,ac,0,p,dastat,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

int minitextshade(int x,int y,const char *t,signed char s,unsigned char p,short sb)
{