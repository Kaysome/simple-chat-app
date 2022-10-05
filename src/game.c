
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
    short ac,dastat;
    char ch;

    dastat = sb & 255;

    while(*t)
    {
        ch = Btoupper(*t);
        if(ch == 32) {x+=5;t++;continue;}
        else ac = ch - '!' + MINIFONT;

        if (sb & MINITEXT_SBAR) statusbarsprite(x,y,65536L,0,ac,s,p,dastat,0,0,xdim-1,ydim-1);
        else if (sb & MINITEXT_FBAR) fragbarsprite(x,y,65536L,0,ac,s,p,dastat,0,0,xdim-1,ydim-1);
        else rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,dastat,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

void gamenumber(int x,int y,int n,signed char s)
{
    char b[10];
    //ltoa(n,b,10);
    Bsnprintf(b,10,"%d",n);
    gametext(x,y,b,s,2+8+16);
}


char recbuf[80];
void allowtimetocorrecterrorswhenquitting(void)
{
    int i, j, oldtotalclock;

    ready2send = 0;

    for(j=0;j<8;j++)
    {
        oldtotalclock = totalclock;

        while (totalclock < oldtotalclock+TICSPERFRAME) {
            handleevents();
            getpackets();
        }

        if(KB_KeyPressed(sc_Escape)) return;

        packbuf[0] = 127;
        for(i=connecthead;i>=0;i=connectpoint2[i])
        {
            if (i != myconnectindex) sendpacket(i,packbuf,1);
            if ((!networkmode) && (myconnectindex != connecthead)) break; //slaves in M/S mode only send to master
        }
     }
}

#define MAXUSERQUOTES 4
int quotebot, quotebotgoal;
short user_quote_time[MAXUSERQUOTES];
char user_quote[MAXUSERQUOTES][128];
// char typebuflen,typebuf[41];

void adduserquote(char *daquote)
{
    int i;

    for(i=MAXUSERQUOTES-1;i>0;i--)
    {
        strcpy(user_quote[i],user_quote[i-1]);
        user_quote_time[i] = user_quote_time[i-1];
    }
    strcpy(user_quote[0],daquote);
    buildprintf("%s\n", daquote);
    user_quote_time[0] = 180;
    pub = NUMPAGES;
}


void getpackets(void)
{
    int i, j, k, l;
    int other, packbufleng;
    input *osyn, *nsyn;

    sampletimer();

    // only dispatch commands here when not in a game
    if( !(ps[myconnectindex].gm&MODE_GAME) ) { OSD_DispatchQueued(); }

    if (numplayers < 2) return;
    while ((packbufleng = getpacket(&other,packbuf)) > 0)
    {
        switch(packbuf[0])
        {
            case 126:
                         //Slaves in M/S mode only send to master
                         //Master re-transmits message to all others
                     if ((!networkmode) && (myconnectindex == connecthead))
                         for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                             if (i != other) sendpacket(i,packbuf,packbufleng);

                multiflag = 2;
                multiwhat = 0;
                multiwho = packbuf[2]; //other: need to send in m/s mode because of possible re-transmit
                multipos = packbuf[1];
                loadplayer( multipos );
                multiflag = 0;
                break;
            case 0:  //[0] (receive master sync buffer)
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    {
                        if (playerquitflag[i] == 0) continue;
                        if (i == myconnectindex)
                            otherminlag = (int)((signed char)packbuf[j]);
                        j++;
                    }

                osyn = (input *)&inputfifo[(movefifoend[connecthead]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[connecthead])&(MOVEFIFOSIZ-1)][0];

                k = j;
                for(i=connecthead;i>=0;i=connectpoint2[i])
                    j += playerquitflag[i];
                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    if (playerquitflag[i] == 0) continue;

                    l = packbuf[k++];
                    if (i == myconnectindex)
                        { j += ((l&1)<<1)+(l&2)+((l&4)>>2)+((l&8)>>3)+((l&16)>>4)+((l&32)>>5)+((l&64)>>6)+((l&128)>>7); continue; }

                    copybufbyte(&osyn[i],&nsyn[i],sizeof(input));
                    if (l&1)   nsyn[i].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&2)   nsyn[i].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&4)   nsyn[i].avel = (signed char)packbuf[j++];
                    if (l&8)   nsyn[i].bits = ((nsyn[i].bits&0xffffff00)|((int)packbuf[j++]));
                    if (l&16)  nsyn[i].bits = ((nsyn[i].bits&0xffff00ff)|((int)packbuf[j++])<<8);
                    if (l&32)  nsyn[i].bits = ((nsyn[i].bits&0xff00ffff)|((int)packbuf[j++])<<16);
                    if (l&64)  nsyn[i].bits = ((nsyn[i].bits&0x00ffffff)|((int)packbuf[j++])<<24);
                    if (l&128) nsyn[i].horz = (signed char)packbuf[j++];

                    if (nsyn[i].bits&(1<<26)) playerquitflag[i] = 0;
                    movefifoend[i]++;
                }

                while (j != packbufleng)
                {
                    for(i=connecthead;i>=0;i=connectpoint2[i])
                        if(i != myconnectindex)
                    {
                        syncval[i][syncvalhead[i]&(MOVEFIFOSIZ-1)] = packbuf[j];
                        syncvalhead[i]++;
                    }
                    j++;
                }

                for(i=connecthead;i>=0;i=connectpoint2[i])
                    if (i != myconnectindex)
                        for(j=1;j<movesperpacket;j++)
                        {
                            copybufbyte(&nsyn[i],&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input));
                            movefifoend[i]++;
                        }

                 movefifosendplc += movesperpacket;

                break;
            case 1:  //[1] (receive slave sync buffer)
                j = 2; k = packbuf[1];

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((int)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((int)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((int)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((int)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
                movefifoend[other]++;

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                break;

            case 4:
                   //slaves in M/S mode only send to master
                if ((!networkmode) && (myconnectindex == connecthead))
                {
                   if (packbuf[1] == 255)
                   {
                         //Master re-transmits message to all others
                      for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                         if (i != other)
                            sendpacket(i,packbuf,packbufleng);
                   }
                   else if (((int)packbuf[1]) != myconnectindex)
                   {
                         //Master re-transmits message not intended for master
                      sendpacket((int)packbuf[1],packbuf,packbufleng);
                      break;
                   }
                }

                i = sizeof(recbuf)-1;
                i = max(0, min(i, packbufleng-2));
                memcpy(recbuf, (char*)packbuf + 2, i);
                recbuf[i] = 0;

                adduserquote(recbuf);
                sound(EXITMENUSOUND);

                pus = NUMPAGES;
                pub = NUMPAGES;

                break;

            case 5:
                         //Slaves in M/S mode only send to master
                         //Master re-transmits message to all others
                     if ((!networkmode) && (myconnectindex == connecthead))
                         for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                             if (i != other) sendpacket(i,packbuf,packbufleng);

                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                ud.coop = ud.m_coop;

                if (enterlevel(MODE_GAME)) backtomenu();

                break;
            case 6:
                    //slaves in M/S mode only send to master
                    //Master re-transmits message to all others
                if ((!networkmode) && (myconnectindex == connecthead))
                    for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                        if (i != other) sendpacket(i,packbuf,packbufleng);


                if (other != packbuf[1])
                    debugprintf("getpackets: player %d announcement with mismatched index %d.\n",
                        other, packbuf[1]);

                if (packbuf[2] != BYTEVERSION)
                    gameexit("\nYou cannot play Duke with different versions.");

                for (i=3,j=0; packbuf[i] && i<packbufleng; i++)
                    if (j < (int)sizeof(ud.user_name[0])-1)
                        ud.user_name[other][j++] = packbuf[i];
                ud.user_name[other][j] = 0;
                i++;

                if (packbufleng-i < 10+3)
                {
                    debugprintf("getpackets: player %d announcement is too short.\n", other);
                    break;
                }

                //This used to be Duke packet #9... now concatenated with Duke packet #6
                for (j=0;j<10;j++,i++) ud.wchoice[other][j] = packbuf[i];

                ps[other].aim_mode = packbuf[i++];
                ps[other].auto_aim = packbuf[i++];
                ps[other].weaponswitch = packbuf[i++];

                break;
            case 7:
                         //slaves in M/S mode only send to master
                         //Master re-transmits message to all others
                     if ((!networkmode) && (myconnectindex == connecthead))
                         for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                             if (i != other) sendpacket(i,packbuf,packbufleng);

                if(numlumps == 0) break;

                if (SoundToggle == 0 || ud.lockout == 1 || FXDevice < 0 )
                    break;
                rtsptr = (char *)RTS_GetSound(packbuf[1]-1);
                FX_PlayAuto3D(rtsptr,RTS_SoundLength(packbuf[1]-1),0,0,0,255,-packbuf[1]);
                rtsplaying = 7;
                break;
            case 8:
                         //Slaves in M/S mode only send to master
                         //Master re-transmits message to all others
                     if ((!networkmode) && (myconnectindex == connecthead))
                         for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                             if (i != other) sendpacket(i,packbuf,packbufleng);

                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = ud.coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                l = sizeof(boardfilename)-1;
                l = min(l, packbufleng-11);
                copybufbyte(packbuf+10,boardfilename,l);
                boardfilename[l] = 0;

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                if (enterlevel(MODE_GAME)) backtomenu();
                break;

            case 16:
                movefifoend[other] = movefifoplc = movefifosendplc = fakemovefifoplc = 0;
                syncvalhead[other] = syncvaltottail = 0L;
                // fall through
            case 17:
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    if (other == connecthead)
                        for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                        {
                            if (i == myconnectindex)
                                otherminlag = (int)((signed char)packbuf[j]);
                            j++;
                        }

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                k = packbuf[j++];
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((int)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((int)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((int)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((int)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
                movefifoend[other]++;

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                if (j > packbufleng)
                    buildprintf("INVALID GAME PACKET!!! (%d too many bytes)\n",j-packbufleng);

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                break;
            case 127:
                break;

            case 250:
                playerreadyflag[other]++;
                break;
            case 255:
                gameexit(" ");
                break;
        }
    }
}

extern void computergetinput(int snum, input *syn);
void faketimerhandler()
{
    int i, j, k;
//    short who;
    input *osyn, *nsyn;

    sampletimer();
    if ((totalclock < ototalclock+TICSPERFRAME) || (ready2send == 0)) return;
    ototalclock += TICSPERFRAME;

    getpackets(); if (getoutputcirclesize() >= 16) return;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
            if (movefifoend[i] < movefifoend[myconnectindex]-200) return;

     getinput(myconnectindex);

     avgfvel += loc.fvel;
     avgsvel += loc.svel;
     avgavel += loc.avel;
     avghorz += loc.horz;
     avgbits |= loc.bits;
     if (movefifoend[myconnectindex]&(movesperpacket-1))
     {
          copybufbyte(&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex],
                          &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex],sizeof(input));
          movefifoend[myconnectindex]++;
          return;
     }
     nsyn = &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex];
     nsyn[0].fvel = avgfvel/movesperpacket;
     nsyn[0].svel = avgsvel/movesperpacket;
     nsyn[0].avel = avgavel/movesperpacket;
     nsyn[0].horz = avghorz/movesperpacket;
     nsyn[0].bits = avgbits;
     avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
     movefifoend[myconnectindex]++;

     if (numplayers < 2)
     {
          if (ud.multimode > 1) for(i=connecthead;i>=0;i=connectpoint2[i])
              if(i != myconnectindex)
              {
                  //clearbufbyte(&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input),0L);
                  if(ud.playerai)
                      computergetinput(i,&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i]);
                  movefifoend[i]++;
              }
          return;
     }

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
        {
            k = (movefifoend[myconnectindex]-1)-movefifoend[i];
            myminlag[i] = min(myminlag[i],k);
            mymaxlag = max(mymaxlag,k);
        }

    if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
    {
        i = mymaxlag-bufferjitter; mymaxlag = 0;
        if (i > 0) bufferjitter += ((3+i)>>2);
        else if (i < 0) bufferjitter -= ((1-i)>>2);
    }

    if (networkmode == 1)
    {
        packbuf[0] = 17;
        if ((movefifoend[myconnectindex]-1) == 0) packbuf[0] = 16;
        j = 1;

            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            if (myconnectindex != connecthead)
            {
                i = myminlag[connecthead]-otherminlag;
                if (klabs(i) > 8) i >>= 1;
                else if (klabs(i) > 2) i = ksgn(i);
                else i = 0;

                totalclock -= TICSPERFRAME*i;
                myminlag[connecthead] -= i; otherminlag += i;
            }

            if (myconnectindex == connecthead)
                for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        k = j;
        packbuf[j++] = 0;

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].fvel;
            packbuf[j++] = (unsigned char)(nsyn[0].fvel>>8);
            packbuf[k] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].svel;
            packbuf[j++] = (unsigned char)(nsyn[0].svel>>8);
            packbuf[k] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].avel;
            packbuf[k] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[k] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[k] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[k] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[k] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (unsigned char)nsyn[0].horz;
            packbuf[k] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sendpacket(i,packbuf,j);

        return;
    }
    if (myconnectindex != connecthead)   //Slave
    {
            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            i = myminlag[connecthead]-otherminlag;
            if (klabs(i) > 8) i >>= 1;
            else if (klabs(i) > 2) i = ksgn(i);
            else i = 0;

            totalclock -= TICSPERFRAME*i;
            myminlag[connecthead] -= i; otherminlag += i;

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        packbuf[0] = 1; packbuf[1] = 0; j = 2;

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].fvel;
            packbuf[j++] = (unsigned char)(nsyn[0].fvel>>8);
            packbuf[1] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].svel;
            packbuf[j++] = (unsigned char)(nsyn[0].svel>>8);
            packbuf[1] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (unsigned char)nsyn[0].avel;
            packbuf[1] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[1] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[1] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[1] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[1] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (unsigned char)nsyn[0].horz;
            packbuf[1] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        sendpacket(connecthead,packbuf,j);
        return;
    }

          //This allows packet resends
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (movefifoend[i] <= movefifosendplc)
        {
            packbuf[0] = 127;
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               sendpacket(i,packbuf,1);
            return;
        }

    while (1)  //Master
    {
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (playerquitflag[i] && (movefifoend[i] <= movefifosendplc)) return;

        osyn = (input *)&inputfifo[(movefifosendplc-1)&(MOVEFIFOSIZ-1)][0];
        nsyn = (input *)&inputfifo[(movefifosendplc  )&(MOVEFIFOSIZ-1)][0];

            //MASTER -> SLAVE packet
        packbuf[0] = 0; j = 1;

            //Fix timers and buffer/jitter value
        if ((movefifosendplc&(TIMERUPDATESIZ-1)) == 0)
        {
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               if (playerquitflag[i])
                packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        k = j;
        for(i=connecthead;i>=0;i=connectpoint2[i])
           j += playerquitflag[i];
        for(i=connecthead;i>=0;i=connectpoint2[i])
        {
            if (playerquitflag[i] == 0) continue;

            packbuf[k] = 0;
            if (nsyn[i].fvel != osyn[i].fvel)
            {
                packbuf[j++] = (unsigned char)nsyn[i].fvel;
                packbuf[j++] = (unsigned char)(nsyn[i].fvel>>8);
                packbuf[k] |= 1;
            }
            if (nsyn[i].svel != osyn[i].svel)
            {
                packbuf[j++] = (unsigned char)nsyn[i].svel;
                packbuf[j++] = (unsigned char)(nsyn[i].svel>>8);
                packbuf[k] |= 2;
            }
            if (nsyn[i].avel != osyn[i].avel)
            {
                packbuf[j++] = (unsigned char)nsyn[i].avel;
                packbuf[k] |= 4;
            }
            if ((nsyn[i].bits^osyn[i].bits)&0x000000ff) packbuf[j++] = (nsyn[i].bits&255), packbuf[k] |= 8;
            if ((nsyn[i].bits^osyn[i].bits)&0x0000ff00) packbuf[j++] = ((nsyn[i].bits>>8)&255), packbuf[k] |= 16;
            if ((nsyn[i].bits^osyn[i].bits)&0x00ff0000) packbuf[j++] = ((nsyn[i].bits>>16)&255), packbuf[k] |= 32;
            if ((nsyn[i].bits^osyn[i].bits)&0xff000000) packbuf[j++] = ((nsyn[i].bits>>24)&255), packbuf[k] |= 64;
            if (nsyn[i].horz != osyn[i].horz)
            {
                packbuf[j++] = (unsigned char)nsyn[i].horz;
                packbuf[k] |= 128;
            }
            k++;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
            if (playerquitflag[i])
            {
                 sendpacket(i,packbuf,j);
                 if (nsyn[i].bits&(1<<26))
                    playerquitflag[i] = 0;
            }

        movefifosendplc += movesperpacket;
    }
}

extern int cacnum;
typedef struct { void **hand; int leng; unsigned char *lock; } cactype;
extern cactype cac[];

void caches(void)
{
     short i,k;

     k = 0;
     for(i=0;i<cacnum;i++)
          if ((*cac[i].lock) >= 200)
          {
                sprintf(buf,"Locked- %d: Leng:%d, Lock:%d",i,cac[i].leng,*cac[i].lock);
                printext256(0L,k,31,-1,buf,1); k += 6;
          }

     k += 6;

     for(i=1;i<11;i++)
          if (lumplockbyte[i] >= 200)
          {
                sprintf(buf,"RTS Locked %d:",i);
                printext256(0L,k,31,-1,buf,1); k += 6;
          }


}



void checksync(void)
{
      int i;

      for(i=connecthead;i>=0;i=connectpoint2[i])
            if (syncvalhead[i] == syncvaltottail) break;
      if (i < 0)
      {
             syncstat = 0;
             do
             {
                     for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                            if (syncval[i][syncvaltottail&(MOVEFIFOSIZ-1)] !=
                                syncval[connecthead][syncvaltottail&(MOVEFIFOSIZ-1)])
                                 syncstat = 1;
                     syncvaltottail++;
                     for(i=connecthead;i>=0;i=connectpoint2[i])
                            if (syncvalhead[i] == syncvaltottail) break;
             } while (i < 0);
      }
      if (connectpoint2[connecthead] < 0) syncstat = 0;

      if (syncstat)
      {
          printext256(4L,130L,31,0,"Out Of Sync - Please restart game",0);
          printext256(4L,138L,31,0,"RUN DN3DHELP.EXE for information.",0);
      }
      if (syncstate)
      {
          printext256(4L,160L,31,0,"Missed Network packet!",0);
          printext256(4L,138L,31,0,"RUN DN3DHELP.EXE for information.",0);
      }
}


void check_fta_sounds(short i)
{
    if(sprite[i].extra > 0) switch(PN)
    {
        case LIZTROOPONTOILET:
        case LIZTROOPJUSTSIT:
        case LIZTROOPSHOOT:
        case LIZTROOPJETPACK:
        case LIZTROOPDUCKING:
        case LIZTROOPRUNNING:
        case LIZTROOP:
            spritesound(PRED_RECOG,i);
            break;
        case LIZMAN:
        case LIZMANSPITTING:
        case LIZMANFEEDING:
        case LIZMANJUMP:
            spritesound(CAPT_RECOG,i);
            break;
        case PIGCOP:
        case PIGCOPDIVE:
            spritesound(PIG_RECOG,i);
            break;
        case RECON:
            spritesound(RECO_RECOG,i);
            break;
        case DRONE:
            spritesound(DRON_RECOG,i);
            break;
        case COMMANDER:
        case COMMANDERSTAYPUT:
            spritesound(COMM_RECOG,i);
            break;
        case ORGANTIC:
            spritesound(TURR_RECOG,i);
            break;
        case OCTABRAIN:
        case OCTABRAINSTAYPUT:
            spritesound(OCTA_RECOG,i);
            break;
        case BOSS1:
            sound(BOS1_RECOG);
            break;
        case BOSS2:
            if(sprite[i].pal == 1)
                sound(BOS2_RECOG);
            else sound(WHIPYOURASS);
            break;
        case BOSS3:
            if(sprite[i].pal == 1)
                sound(BOS3_RECOG);
            else sound(RIPHEADNECK);
            break;
        case BOSS4:
        case BOSS4STAYPUT:
            if(sprite[i].pal == 1)
                sound(BOS4_RECOG);
            sound(BOSS4_FIRSTSEE);
            break;
        case GREENSLIME:
            spritesound(SLIM_RECOG,i);
            break;
    }
}

short inventory(spritetype *s)
{
    switch(s->picnum)
    {
        case FIRSTAID:
        case STEROIDS:
        case HEATSENSOR:
        case BOOTS:
        case JETPACK:
        case HOLODUKE:
        case AIRTANK:
            return 1;
    }
    return 0;
}


short badguy(spritetype *s)
{

    switch(s->picnum)
    {
            case SHARK:
            case RECON:
            case DRONE:
            case LIZTROOPONTOILET:
            case LIZTROOPJUSTSIT:
            case LIZTROOPSTAYPUT:
            case LIZTROOPSHOOT:
            case LIZTROOPJETPACK:
            case LIZTROOPDUCKING:
            case LIZTROOPRUNNING:
            case LIZTROOP:
            case OCTABRAIN:
            case COMMANDER:
            case COMMANDERSTAYPUT:
            case PIGCOP:
            case EGG:
            case PIGCOPSTAYPUT:
            case PIGCOPDIVE:
            case LIZMAN:
            case LIZMANSPITTING:
            case LIZMANFEEDING:
            case LIZMANJUMP:
            case ORGANTIC:
            case BOSS1:
            case BOSS2:
            case BOSS3:
            case BOSS4:
            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:
            case RAT:
            case ROTATEGUN:
                return 1;
    }
    if( actortype[s->picnum] ) return 1;

    return 0;
}


short badguypic(short pn)
{

    switch(pn)
    {
            case SHARK:
            case RECON:
            case DRONE:
            case LIZTROOPONTOILET:
            case LIZTROOPJUSTSIT:
            case LIZTROOPSTAYPUT:
            case LIZTROOPSHOOT:
            case LIZTROOPJETPACK:
            case LIZTROOPDUCKING:
            case LIZTROOPRUNNING:
            case LIZTROOP:
            case OCTABRAIN:
            case COMMANDER:
            case COMMANDERSTAYPUT:
            case PIGCOP:
            case EGG:
            case PIGCOPSTAYPUT:
            case PIGCOPDIVE:
            case LIZMAN:
            case LIZMANSPITTING:
            case LIZMANFEEDING:
            case LIZMANJUMP:
            case ORGANTIC:
            case BOSS1:
            case BOSS2:
            case BOSS3:
            case BOSS4:
            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:
            case RAT:
            case ROTATEGUN:
                return 1;
    }

    if( actortype[pn] ) return 1;

    return 0;
}



void myos(int x, int y, short tilenum, signed char shade, char orientation)
{
    char p;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    p = sector[ps[screenpeek].cursectnum].floorpal;
    rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void myospal(int x, int y, short tilenum, signed char shade, char orientation, char p)
{
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

void invennum(int x,int y,unsigned char num1,char ha,unsigned char sbits)
{
    char dabuf[80] = {0};
    sprintf(dabuf,"%d",num1);
    if(num1 > 99)
    {
        statusbarsprite(x-4,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        statusbarsprite(x,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        statusbarsprite(x,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
}

void orderweaponnum(short ind,int x,int y,int num1, int num2,char ha)
{
    (void)num1; (void)num2;

    statusbarsprite(x-7,y,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    statusbarsprite(x-3,y,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);

    minitextshade(x+1,y-4,"ORDER",26,6,2+8+16+128 + MINITEXT_SBAR);
}


void weaponnum(short ind,int x,int y,int num1, int num2,char ha)
{
    char dabuf[80] = {0};

    statusbarsprite(x-7,y,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    statusbarsprite(x-3,y,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    statusbarsprite(x+9,y,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    if(num1 > 99) num1 = 99;
    if(num2 > 99) num2 = 99;

    sprintf(dabuf,"%d",num1);
    if(num1 > 9)
    {
        statusbarsprite(x,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num2);
    if(num2 > 9)
    {
        statusbarsprite(x+13,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+17,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else statusbarsprite(x+13,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

void weaponnum999(char ind,int x,int y,int num1, int num2,char ha)
{
    char dabuf[80] = {0};

    statusbarsprite(x-7,y,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    statusbarsprite(x-4,y,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    statusbarsprite(x+13,y,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num1);
    if(num1 > 99)
    {
        statusbarsprite(x,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+8,y,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        statusbarsprite(x+4,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+8,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else statusbarsprite(x+8,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num2);
    if(num2 > 99)
    {
        statusbarsprite(x+17,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+21,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+25,y,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num2 > 9)
    {
        statusbarsprite(x+17,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        statusbarsprite(x+21,y,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else statusbarsprite(x+25,y,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

    //REPLACE FULLY
void weapon_amounts(struct player_struct *p,int x,int y,int u)
{
     int cw;

     cw = p->curr_weapon;

     if (u&4)
     {
          if (u != -1) patchstatusbar(88,178,88+37,178+6); //original code: (96,178,96+12,178+6);
          weaponnum999(PISTOL_WEAPON,x,y,
                          p->ammo_amount[PISTOL_WEAPON],max_ammo_amount[PISTOL_WEAPON],
                          12-20*(cw == PISTOL_WEAPON) );
     }
     if (u&8)
     {
          if (u != -1) patchstatusbar(88,184,88+37,184+6); //original code: (96,184,96+12,184+6);
          weaponnum999(SHOTGUN_WEAPON,x,y+6,
                          p->ammo_amount[SHOTGUN_WEAPON],max_ammo_amount[SHOTGUN_WEAPON],
                          (!p->gotweapon[SHOTGUN_WEAPON]*9)+12-18*
                          (cw == SHOTGUN_WEAPON) );
     }
     if (u&16)
     {
          if (u != -1) patchstatusbar(88,190,88+37,190+6); //original code: (96,190,96+12,190+6);
          weaponnum999(CHAINGUN_WEAPON,x,y+12,
                            p->ammo_amount[CHAINGUN_WEAPON],max_ammo_amount[CHAINGUN_WEAPON],
                            (!p->gotweapon[CHAINGUN_WEAPON]*9)+12-18*
                            (cw == CHAINGUN_WEAPON) );
     }
     if (u&32)
     {
          if (u != -1) patchstatusbar(127,178,127+29,178+6); //original code: (135,178,135+8,178+6);
          weaponnum(RPG_WEAPON,x+39,y,
                      p->ammo_amount[RPG_WEAPON],max_ammo_amount[RPG_WEAPON],
                      (!p->gotweapon[RPG_WEAPON]*9)+12-19*
                      (cw == RPG_WEAPON) );
     }
     if (u&64)
     {
          if (u != -1) patchstatusbar(127,184,127+29,184+6); //original code: (135,184,135+8,184+6);
          weaponnum(HANDBOMB_WEAPON,x+39,y+6,
                          p->ammo_amount[HANDBOMB_WEAPON],max_ammo_amount[HANDBOMB_WEAPON],
                          (((!p->ammo_amount[HANDBOMB_WEAPON])|(!p->gotweapon[HANDBOMB_WEAPON]))*9)+12-19*
                          ((cw == HANDBOMB_WEAPON) || (cw == HANDREMOTE_WEAPON)));
     }
     if (u&128)
     {
          if (u != -1) patchstatusbar(127,190,127+29,190+6); //original code: (135,190,135+8,190+6);

if (VOLUMEONE) {
            orderweaponnum(SHRINKER_WEAPON,x+39,y+12,
                            p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
                            (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
                            (cw == SHRINKER_WEAPON) );
} else {
            if(p->subweapon&(1<<GROW_WEAPON))
                 weaponnum(SHRINKER_WEAPON,x+39,y+12,
                      p->ammo_amount[GROW_WEAPON],max_ammo_amount[GROW_WEAPON],
                      (!p->gotweapon[GROW_WEAPON]*9)+12-18*
                      (cw == GROW_WEAPON) );
            else
                 weaponnum(SHRINKER_WEAPON,x+39,y+12,
                      p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
                      (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
                      (cw == SHRINKER_WEAPON) );
}
      }
      if (u&256)
      {
            if (u != -1) patchstatusbar(158,178,162+29,178+6); //original code: (166,178,166+8,178+6);

if (VOLUMEONE) {
          orderweaponnum(DEVISTATOR_WEAPON,x+70,y,
                            p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                            (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                            (cw == DEVISTATOR_WEAPON) );
} else {
            weaponnum(DEVISTATOR_WEAPON,x+70,y,
                            p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                            (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                            (cw == DEVISTATOR_WEAPON) );
}
      }
      if (u&512)
      {
            if (u != -1) patchstatusbar(158,184,162+29,184+6); //original code: (166,184,166+8,184+6);
if (VOLUMEONE) {
            orderweaponnum(TRIPBOMB_WEAPON,x+70,y+6,
                            p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
                            (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
                            (cw == TRIPBOMB_WEAPON) );
} else {
            weaponnum(TRIPBOMB_WEAPON,x+70,y+6,
                            p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
                            (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
                            (cw == TRIPBOMB_WEAPON) );
}
      }

      if (u&65536L)
      {
            if (u != -1) patchstatusbar(158,190,162+29,190+6); //original code: (166,190,166+8,190+6);
if (VOLUMEONE) {
          orderweaponnum(-1,x+70,y+12,
                            p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
                            (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
                            (cw == FREEZE_WEAPON) );
} else {
            weaponnum(-1,x+70,y+12,
                            p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
                            (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
                            (cw == FREEZE_WEAPON) );
}
      }
}


void digitalnumber(int x,int y,int n,char s,unsigned char cs)
{
    short i, j, k, p, c;
    char b[10];

    Bsnprintf(b,10,"%d",n);
    i = strlen(b);
    j = 0;

    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        j += tilesizx[p]+1;
    }
    c = x-(j>>1);

    j = 0;
    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        statusbarsprite(c+j,y,65536L,0,p,s,0,cs,0,0,xdim-1,ydim-1);
        j += tilesizx[p]+1;
    }
}

/*

void scratchmarks(int x,int y,int n,char s,char p)
{
    int i, ni;

    ni = n/5;
    for(i=ni;i >= 0;i--)
    {
        overwritesprite(x-2,y,SCRATCH+4,s,0,0);
        x += tilesizx[SCRATCH+4]-1;
    }

    ni = n%5;
    if(ni) overwritesprite(x,y,SCRATCH+ni-1,s,p,0);
}
  */
void displayinventory(struct player_struct *p)
{
    short n, j, xoff, y;

    j = xoff = 0;

    n = (p->jetpack_amount > 0)<<3; if(n&8) j++;
    n |= ( p->scuba_amount > 0 )<<5; if(n&32) j++;
    n |= (p->steroids_amount > 0)<<1; if(n&2) j++;
    n |= ( p->holoduke_amount > 0)<<2; if(n&4) j++;
    n |= (p->firstaid_amount > 0); if(n&1) j++;
    n |= (p->heat_amount > 0)<<4; if(n&16) j++;
    n |= (p->boot_amount > 0)<<6; if(n&64) j++;

    xoff = 160-(j*11);

    j = 0;

    if(ud.screen_size > 4)
        y = 154;
    else y = 172;

    if(ud.screen_size == 4)
    {
        if(ud.multimode > 1)
            xoff += 56;
        else xoff += 65;
    }

    while( j <= 9 )
    {
        if( n&(1<<j) )
        {
            switch( n&(1<<j) )
            {
                case   1:
                rotatesprite(xoff<<16,y<<16,65536L,0,FIRSTAID_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   2:
                rotatesprite((xoff+1)<<16,y<<16,65536L,0,STEROIDS_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   4:
                rotatesprite((xoff+2)<<16,y<<16,65536L,0,HOLODUKE_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   8:
                rotatesprite(xoff<<16,y<<16,65536L,0,JETPACK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  16:
                rotatesprite(xoff<<16,y<<16,65536L,0,HEAT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  32:
                rotatesprite(xoff<<16,y<<16,65536L,0,AIRTANK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case 64:
                rotatesprite(xoff<<16,(y-1)<<16,65536L,0,BOOT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
            }

            xoff += 22;

            if(p->inven_icon == j+1)
                rotatesprite((xoff-2)<<16,(y+19)<<16,65536L,1024,ARROW,-32,0,2+16,windowx1,windowy1,windowx2,windowy2);
        }

        j++;
    }
}



void displayfragbar(void)
{
    short i, j;

    j = 0;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if(i > j) j = i;

    fragbarsprite(0,0,65536L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 4) fragbarsprite(0,tilesizy[FRAGBAR],65536L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 8) fragbarsprite(0,tilesizy[FRAGBAR]*2,65536L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 12) fragbarsprite(0,tilesizy[FRAGBAR]*3,65536L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        minitext(21+(73*(i&3)),2+((i&28)<<1),&ud.user_name[i][0],sprite[ps[i].i].pal,2+8+16+128 + MINITEXT_FBAR);
        sprintf(buf,"%d",ps[i].frag-ps[i].fraggedself);
        minitext(17+50+(73*(i&3)),2+((i&28)<<1),buf,sprite[ps[i].i].pal,2+8+16+128 + MINITEXT_FBAR);
    }
}

void coolgaugetext(short snum)
{
     struct player_struct *p;
     int i, j, o, ss, u;
     short inv_percent = -1;

     p = &ps[snum];

     if (p->invdisptime > 0) displayinventory(p);


     if(ps[snum].gm&MODE_MENU)
          if( (current_menu >= 400  && current_menu <= 405) )
                return;

     ss = ud.screen_size; if (ss < 4) return;

     if ( ud.multimode > 1 && ud.coop != 1 )
     {
          if (pus)
                { displayfragbar(); }
          else
          {
                for(i=connecthead;i>=0;i=connectpoint2[i])
                     if (ps[i].frag != sbar.frag[i]) { displayfragbar(); break; }
          }
          for(i=connecthead;i>=0;i=connectpoint2[i])
                if (i != myconnectindex)
                     sbar.frag[i] = ps[i].frag;
     }

     if (ss == 4)   //DRAW MINI STATUS BAR:
     {
          statusbarsprite(5,200-28,65536L,0,HEALTHBOX,0,21,10+16,0,0,xdim-1,ydim-1);
          if (p->inven_icon)
                statusbarsprite(69,200-30,65536L,0,INVENTORYBOX,0,21,10+16,0,0,xdim-1,ydim-1);

          if(sprite[p->i].pal == 1 && p->last_extra < 2)
                digitalnumber(20,200-17,1,-16,10+16);
          else digitalnumber(20,200-17,p->last_extra,-16,10+16);

          statusbarsprite(37,200-28,65536L,0,AMMOBOX,0,21,10+16,0,0,xdim-1,ydim-1);

          if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
          digitalnumber(53,200-17,p->ammo_amount[i],-16,10+16);

          o = 158;
          if (p->inven_icon)
          {
                switch(p->inven_icon)
                {
                     case 1: i = FIRSTAID_ICON; break;
                     case 2: i = STEROIDS_ICON; break;
                     case 3: i = HOLODUKE_ICON; break;
                     case 4: i = JETPACK_ICON; break;
                     case 5: i = HEAT_ICON; break;
                     case 6: i = AIRTANK_ICON; break;
                     case 7: i = BOOT_ICON; break;
                     default: i = -1;
                }
                if (i >= 0) statusbarsprite(231-o,200-21,65536L,0,i,0,0,10+16,0,0,xdim-1,ydim-1);

                minitext(292-30-o,190,"%",6,10+16 + MINITEXT_SBAR);

                j = 0x80000000;
                switch(p->inven_icon)
                {
                     case 1: i = p->firstaid_amount; break;
                     case 2: i = ((p->steroids_amount+3)>>2); break;
                     case 3: i = ((p->holoduke_amount+15)/24); j = p->holoduke_on; break;
                     case 4: i = ((p->jetpack_amount+15)>>4); j = p->jetpack_on; break;
                     case 5: i = p->heat_amount/12; j = p->heat_on; break;
                     case 6: i = ((p->scuba_amount+63)>>6); break;
                     case 7: i = (p->boot_amount>>1); break;
                }
                invennum(284-30-o,200-6,(char)i,0,10);
                if (j > 0) minitext(288-30-o,180,"ON",0,10+16 + MINITEXT_SBAR);
                else if ((unsigned int)j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16 + MINITEXT_SBAR);
                if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16 + MINITEXT_SBAR);
          }
          pus = 0;
          return;
     }

          //DRAW/UPDATE FULL STATUS BAR:

     if (pus) { pus = 0; u = -1; } else u = 0;

     if (sbar.frag[myconnectindex] != p->frag) { sbar.frag[myconnectindex] = p->frag; u |= 32768; }
     if (sbar.got_access != p->got_access) { sbar.got_access = p->got_access; u |= 16384; }
     if (sbar.last_extra != p->last_extra) { sbar.last_extra = p->last_extra; u |= 1; }
     if (sbar.shield_amount != p->shield_amount) { sbar.shield_amount = p->shield_amount; u |= 2; }
     if (sbar.curr_weapon != p->curr_weapon) { sbar.curr_weapon = p->curr_weapon; u |= (4+8+16+32+64+128+256+512+1024+65536L); }
     for(i=1;i < 10;i++)
     {
          if (sbar.ammo_amount[i] != p->ammo_amount[i]) {
          sbar.ammo_amount[i] = p->ammo_amount[i]; if(i < 9) u |= ((2<<i)+1024); else u |= 65536L+1024; }
          if (sbar.gotweapon[i] != p->gotweapon[i]) { sbar.gotweapon[i] =
          p->gotweapon[i]; if(i < 9 ) u |= ((2<<i)+1024); else u |= 65536L+1024; }
     }
     if (sbar.inven_icon != p->inven_icon) { sbar.inven_icon = p->inven_icon; u |= (2048+4096+8192); }
     if (sbar.holoduke_on != p->holoduke_on) { sbar.holoduke_on = p->holoduke_on; u |= (4096+8192); }
     if (sbar.jetpack_on != p->jetpack_on) { sbar.jetpack_on = p->jetpack_on; u |= (4096+8192); }
     if (sbar.heat_on != p->heat_on) { sbar.heat_on = p->heat_on; u |= (4096+8192); }
     switch(p->inven_icon)
     {
          case 1: inv_percent = p->firstaid_amount; break;
          case 2: inv_percent = ((p->steroids_amount+3)>>2); break;
          case 3: inv_percent = ((p->holoduke_amount+15)/24); break;
          case 4: inv_percent = ((p->jetpack_amount+15)>>4); break;
          case 5: inv_percent = p->heat_amount/12; break;
          case 6: inv_percent = ((p->scuba_amount+63)>>6); break;
          case 7: inv_percent = (p->boot_amount>>1); break;
     }
     if (inv_percent >= 0 && sbar.inv_percent != inv_percent) { sbar.inv_percent = inv_percent; u |= 8192; }
     if (u == 0) return;

     //0 - update health
     //1 - update armor
     //2 - update PISTOL_WEAPON ammo
     //3 - update SHOTGUN_WEAPON ammo
     //4 - update CHAINGUN_WEAPON ammo
     //5 - update RPG_WEAPON ammo
     //6 - update HANDBOMB_WEAPON ammo
     //7 - update SHRINKER_WEAPON ammo
     //8 - update DEVISTATOR_WEAPON ammo
     //9 - update TRIPBOMB_WEAPON ammo
     //10 - update ammo display
     //11 - update inventory icon
     //12 - update inventory on/off
     //13 - update inventory %
     //14 - update keys
     //15 - update kills
     //16 - update FREEZE_WEAPON ammo
#define SBY (200-tilesizy[BOTTOMSTATUSBAR])
     if (numpages == 127)
     {
          // When the frame is non-persistent, redraw all elements so that all the permanent
          // sprites get cleanly evicted.
          u = -1;
     }
     if (u == -1)
     {
          patchstatusbar(0,0,320,200);
          if (ud.multimode > 1 && ud.coop != 1)
                statusbarsprite(277+1,SBY+7-1,65536L,0,KILLSICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
     }
     if (ud.multimode > 1 && ud.coop != 1)
     {
          if (u&32768)
          {
                if (u != -1) patchstatusbar(276,SBY+17,299,SBY+17+10);