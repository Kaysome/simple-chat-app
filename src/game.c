
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
                digitalnumber(287,SBY+17,max(p->frag-p->fraggedself,0),-16,10+16+128);
          }
     }
     else
     {
          if (u&16384)
          {
                if (u != -1) patchstatusbar(275,SBY+18,299,SBY+18+12);
                if (p->got_access&4) statusbarsprite(275,SBY+16,65536L,0,ACCESS_ICON,0,23,10+16+128,0,0,xdim-1,ydim-1);
                if (p->got_access&2) statusbarsprite(288,SBY+16,65536L,0,ACCESS_ICON,0,21,10+16+128,0,0,xdim-1,ydim-1);
                if (p->got_access&1) statusbarsprite(281,SBY+23,65536L,0,ACCESS_ICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
          }
     }
     if (u&(4+8+16+32+64+128+256+512+65536L)) weapon_amounts(p,96,SBY+16,u);

     if (u&1)
     {
          if (u != -1) patchstatusbar(20,SBY+17,43,SBY+17+11);
          if (sprite[p->i].pal == 1 && p->last_extra < 2)
                 digitalnumber(32,SBY+17,1,-16,10+16+128);
          else digitalnumber(32,SBY+17,p->last_extra,-16,10+16+128);
     }
     if (u&2)
     {
          if (u != -1) patchstatusbar(52,SBY+17,75,SBY+17+11);
          digitalnumber(64,SBY+17,p->shield_amount,-16,10+16+128);
     }

     if (u&1024)
     {
          if (u != -1) patchstatusbar(196,SBY+17,220,SBY+17+11);
          if (p->curr_weapon != KNEE_WEAPON)
          {
                if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
                digitalnumber(230-22,SBY+17,p->ammo_amount[i],-16,10+16+128);
          }
     }

     if (u&(2048+4096+8192))
     {
          if (u != -1)
          {
                if (u&(2048+4096)) { patchstatusbar(231,SBY+13,265,SBY+13+18); }
                                  else { patchstatusbar(250,SBY+24,261,SBY+24+6); }
          }
          if (p->inven_icon)
          {
                o = 0;

                if (u&(2048+4096))
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
                     }
                     statusbarsprite(231-o,SBY+13,65536L,0,i,0,0,10+16+128,0,0,xdim-1,ydim-1);
                     minitext(292-30-o,SBY+24,"%",6,10+16+128 + MINITEXT_SBAR);
                     if (p->inven_icon >= 6) minitext(284-35-o,SBY+14,"AUTO",2,10+16+128 + MINITEXT_SBAR);
                }
                if (u&(2048+4096))
                {
                     switch(p->inven_icon)
                     {
                          case 3: j = p->holoduke_on; break;
                          case 4: j = p->jetpack_on; break;
                          case 5: j = p->heat_on; break;
                          default: j = 0x80000000;
                     }
                     if (j > 0) minitext(288-30-o,SBY+14,"ON",0,10+16+128 + MINITEXT_SBAR);
                     else if ((unsigned int)j != 0x80000000) minitext(284-30-o,SBY+14,"OFF",2,10+16+128 + MINITEXT_SBAR);
                }
                if (u&8192)
                {
                     invennum(284-30-o,SBY+28,(char)inv_percent,0,10+128);
                }
          }
     }
}


#define AVERAGEFRAMES 16
static int frameval[AVERAGEFRAMES], framecnt = 0;

void tics(void)
{
    int i;
    char b[10];

    i = totalclock;
    if (i != frameval[framecnt])
    {
        sprintf(b,"%d",(TICRATE*AVERAGEFRAMES)/(i-frameval[framecnt]));
        printext256(windowx1,windowy1,31,-21,b,1);
        frameval[framecnt] = i;
    }

    framecnt = ((framecnt+1)&(AVERAGEFRAMES-1));
}

void coords(short snum)
{
    short y = 0;

    if(ud.coop != 1)
    {
        if(ud.multimode > 1 && ud.multimode < 5)
            y = 8;
        else if(ud.multimode > 4)
            y = 16;
    }

    sprintf(buf,"X= %d",ps[snum].posx);
    printext256(250L,y,31,-1,buf,1);
    sprintf(buf,"Y= %d",ps[snum].posy);
    printext256(250L,y+7L,31,-1,buf,1);
    sprintf(buf,"Z= %d",ps[snum].posz);
    printext256(250L,y+14L,31,-1,buf,1);
    sprintf(buf,"A= %d",ps[snum].ang);
    printext256(250L,y+21L,31,-1,buf,1);
    sprintf(buf,"ZV= %d",ps[snum].poszv);
    printext256(250L,y+28L,31,-1,buf,1);
    sprintf(buf,"OG= %d",ps[snum].on_ground);
    printext256(250L,y+35L,31,-1,buf,1);
    sprintf(buf,"AM= %d",ps[snum].ammo_amount[GROW_WEAPON]);
    printext256(250L,y+43L,31,-1,buf,1);
    sprintf(buf,"LFW= %d",ps[snum].last_full_weapon);
    printext256(250L,y+50L,31,-1,buf,1);
    sprintf(buf,"SECTL= %d",sector[ps[snum].cursectnum].lotag);
    printext256(250L,y+57L,31,-1,buf,1);
    sprintf(buf,"SEED= %d",randomseed);
    printext256(250L,y+64L,31,-1,buf,1);
    sprintf(buf,"THOLD= %d",ps[snum].transporter_hold);
    printext256(250L,y+64L+7,31,-1,buf,1);
}

void operatefta(void)
{
     int i, j, k;

     if(ud.screen_size > 0) j = 200-45; else j = 200-8;
     quotebot = min(quotebot,j);
     quotebotgoal = min(quotebotgoal,j);
     if(ps[myconnectindex].gm&MODE_TYPE) j -= 8;
     quotebotgoal = j; j = quotebot;
     for(i=0;i<MAXUSERQUOTES;i++)
     {
         k = user_quote_time[i]; if (k <= 0) break;

         if (k > 4)
              gametext(320>>1,j,user_quote[i],0,2+8+16);
         else if (k > 2) gametext(320>>1,j,user_quote[i],0,2+8+16+1);
             else gametext(320>>1,j,user_quote[i],0,2+8+16+1+32);
         j -= 8;
     }

     if (ps[screenpeek].fta <= 1) return;

     if (ud.coop != 1 && ud.screen_size > 0 && ud.multimode > 1)
     {
         j = 0; k = 8;
         for(i=connecthead;i>=0;i=connectpoint2[i])
             if (i > j) j = i;

         if (j >= 4 && j <= 8) k += 8;
         else if (j > 8 && j <= 12) k += 16;
         else if (j > 12) k += 24;
     }
     else k = 0;

     if (ps[screenpeek].ftq == 115 || ps[screenpeek].ftq == 116)
     {
         k = quotebot;
         for(i=0;i<MAXUSERQUOTES;i++)
         {
             if (user_quote_time[i] <= 0) break;
             k -= 8;
         }
         k -= 4;
     }

     j = ps[screenpeek].fta;
     if (j > 4)
          gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16);
     else
         if (NAM && ps[screenpeek].ftq == 99) gametext(320>>1,k,"Matt Saettler.  matts@saettler.com",0,2+8+16+1);
     else
         if (j > 2) gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1);
     else
         gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1+32);
}

void FTA(short q,struct player_struct *p)
{
    if( ud.fta_on == 1)
    {
        if( p->fta > 0 && q != 115 && q != 116 )
            if( p->ftq == 115 || p->ftq == 116 ) return;

        p->fta = 100;

        if( p->ftq != q || q == 26 )
        // || q == 26 || q == 115 || q ==116 || q == 117 || q == 122 )
        {
            p->ftq = q;
            pub = NUMPAGES;
            pus = NUMPAGES;
            if (p == &ps[screenpeek]) buildprintf("%s\n",fta_quotes[q]);
        }
    }
}

void showtwoscreens(void)
{
    if (VOLUMEONE) {
        setview(0,0,xdim-1,ydim-1);
        flushperms();
        //ps[myconnectindex].palette = palette;
        setgamepalette(&ps[myconnectindex], palette, 1);

        fadepal(0,0,0, 0,64,7);
        clearallviews(0L);
        rotatesprite(0,0,65536L,0,3291,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
        IFISSOFTMODE fadepal(0,0,0, 63,0,-7); else nextpage();

        userack();

        fadepal(0,0,0, 0,64,7);
        rotatesprite(0,0,65536L,0,3290,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
        IFISSOFTMODE fadepal(0,0,0, 63,0,-7); else nextpage();

        userack();
    }
}

void gameexit(const char *t)
{
    if(*t != 0) ps[myconnectindex].palette = &palette[0];

    if(numplayers > 1)
    {
        allowtimetocorrecterrorswhenquitting();
        uninitmultiplayers();
    }

    if(ud.recstat == 1) closedemowrite();
    else if(ud.recstat == 2) { if (frecfilep) fclose(frecfilep); }  // JBF: fixes crash on demo playback

    if(qe)
        goto GOTOHERE;

    if(playerswhenstarted > 1 && ud.coop != 1 && *t == ' ')
    {
        dobonus(1);
        setgamemode(ScreenMode,ScreenWidth,ScreenHeight,ScreenBPP);
    }

    if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y')
        showtwoscreens();

    GOTOHERE:

    Shutdown();

    if(*t != 0)
    {
        if (!(t[0] == ' ' && t[1] == 0)) {
            wm_msgbox("JFDuke3D", "%s", t);
        }
    }

    uninitgroupfile();

    exit(0);
}




short inputloc = 0;
short strget(short x,short y,char *t,short dalen,short c)
{
    short ch, sh, didinput = 0;

    while((ch = KB_Getch()) != 0)
    {
        if(ch == asc_BackSpace)
        {
            if( inputloc > 0 )
            {
                inputloc--;
                *(t+inputloc) = 0;
            }
        }
        else
        {
            if(ch == asc_Enter)
            {
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_kpad_Enter);
                return (1);
            }
            else if(ch == asc_Escape)
            {
                KB_ClearKeyDown(sc_Escape);
                return (-1);
            }
            else if ( ch >= 32 && inputloc < dalen && ch < 127)
            {
                ch = Btoupper(ch);
                if (!(c & STRGET_NUMERIC) || (ch >= '0' && ch <= '9')) {
                    *(t+inputloc) = ch;
                    *(t+inputloc+1) = 0;
                    inputloc++;
                }
                didinput = 1;
            }
        }
    }

    if (!(c & STRGET_NOCONTROLLER) && !didinput)
    {
        extern UserInput uinfo;  // menues.c
        if (uinfo.button0) return (1);
        else if (uinfo.button1) return (-1);
    }

    sh = (signed char)(c & 255);
    if( c & STRGET_NOECHO ) return(0);
    if( c & STRGET_PASSWORD )
    {
        char b[41];
        int ii;
        for(ii=0;ii<inputloc;ii++)
            b[ii] = '*';
        b[ii] = 0;
        x = gametext(x,y,b,sh,2+8+16);
    }
    else x = gametext(x,y,t,sh,2+8+16);
    sh = 4-(sintable[(totalclock<<4)&2047]>>11);
    rotatesprite((x+8)<<16,(y+4)<<16,32768L,0,SPINNINGNUKEICON+((totalclock>>3)%7),sh,0,2+8,0,0,xdim-1,ydim-1);

    return (0);
}

void typemode(void)
{
     short ch, hitstate, i, j;

     if( ps[myconnectindex].gm&MODE_SENDTOWHOM )
     {
          if(sendmessagecommand != -1 || ud.multimode < 3 || movesperpacket == 4)
          {
                tempbuf[0] = 4;
                tempbuf[2] = 0;
                recbuf[0]  = 0;

                if(ud.multimode < 3)
                     sendmessagecommand = 2;

                snprintf(recbuf, sizeof(recbuf), "%s: %s", ud.user_name[myconnectindex], typebuf);
                j = strlen(recbuf);
                memcpy(tempbuf+2,recbuf,j+1);

                if(sendmessagecommand >= ud.multimode || movesperpacket == 4)
                {
                     tempbuf[1] = 255;
                     for(ch=connecthead;ch >= 0;ch=connectpoint2[ch])
                     {
                          if (ch != myconnectindex) sendpacket(ch,tempbuf,j+2);
                          if ((!networkmode) && (myconnectindex != connecthead)) break; //slaves in M/S mode only send to master
                     }
                     adduserquote(recbuf);
                     quotebot += 8;
                     quotebotgoal = quotebot;
                }
                else if(sendmessagecommand >= 0)
                {
                     tempbuf[1] = (char)sendmessagecommand;
                     if ((!networkmode) && (myconnectindex != connecthead))
                          sendmessagecommand = connecthead;
                     sendpacket(sendmessagecommand,(unsigned char *)tempbuf,j+2);
                }

                sendmessagecommand = -1;
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          }
          else if(sendmessagecommand == -1)
          {
                j = 50;
                gametext(320>>1,j,"SEND MESSAGE TO...",0,2+8+16); j += 8;
                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                     if (i == myconnectindex)
                     {
                         minitextshade((320>>1)-40+1,j+1,"A/ENTER - ALL",26,0,2+8+16);
                         minitext((320>>1)-40,j,"A/ENTER - ALL",0,2+8+16); j += 7;
                     }
                     else
                     {
                         sprintf(buf,"      %d - %s",i+1,ud.user_name[i]);
                         minitextshade((320>>1)-40-6+1,j+1,buf,26,0,2+8+16);
                         minitext((320>>1)-40-6,j,buf,0,2+8+16); j += 7;
                     }
                }
                minitextshade((320>>1)-40-4+1,j+1,"    ESC - Abort",26,0,2+8+16);
                minitext((320>>1)-40-4,j,"    ESC - Abort",0,2+8+16); j += 7;

                if (ud.screen_size > 0) j = 200-45; else j = 200-8;
                gametext(320>>1,j,typebuf,0,2+8+16);

                if( KB_KeyWaiting() )
                {
                     i = KB_GetCh();

                     if(i == 'A' || i == 'a' || i == 13)
                          sendmessagecommand = ud.multimode;
                     else if(i >= '1' || i <= (ud.multimode + '1') )
                          sendmessagecommand = i - '1';
                     else
                     {
                        sendmessagecommand = ud.multimode;
                          if(i == 27)
                          {
                              ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
                              sendmessagecommand = -1;
                          }
                          else
                          typebuf[0] = 0;
                     }

                     KB_ClearKeyDown(sc_1);
                     KB_ClearKeyDown(sc_2);
                     KB_ClearKeyDown(sc_3);
                     KB_ClearKeyDown(sc_4);
                     KB_ClearKeyDown(sc_5);
                     KB_ClearKeyDown(sc_6);
                     KB_ClearKeyDown(sc_7);
                     KB_ClearKeyDown(sc_8);
                     KB_ClearKeyDown(sc_A);
                     KB_ClearKeyDown(sc_Escape);
                     KB_ClearKeyDown(sc_Enter);
                }
          }
     }
     else
     {
          if(ud.screen_size > 0) j = 200-45; else j = 200-8;
          hitstate = strget(320>>1,j,typebuf,30,STRGET_SHADE(1)|STRGET_NOCONTROLLER);

          if(hitstate == 1)
          {
                KB_ClearKeyDown(sc_Enter);
                ps[myconnectindex].gm |= MODE_SENDTOWHOM;
          }
          else if(hitstate == -1)
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          else pub = NUMPAGES;
     }
}

void moveclouds(void)
{
    if( totalclock > cloudtotalclock || totalclock < (cloudtotalclock-7))
    {
        short i;

        cloudtotalclock = totalclock+6;

        for(i=0;i<numclouds;i++)
        {
            cloudx[i] += (sintable[(ps[screenpeek].ang+512)&2047]>>9);
            cloudy[i] += (sintable[ps[screenpeek].ang&2047]>>9);

            sector[clouds[i]].ceilingxpanning = cloudx[i]>>6;
            sector[clouds[i]].ceilingypanning = cloudy[i]>>6;
        }
    }
}

void displayrest(int smoothratio)
{
    int a, i, j;
    unsigned char fader=0,fadeg=0,fadeb=0,fadef=0,tintr=0,tintg=0,tintb=0,tintf=0,dotint=0;

    struct player_struct *pp;
    walltype *wal;
    int cposx,cposy,cang;

    pp = &ps[screenpeek];

    // this takes care of fullscreen tint for OpenGL
    if (POLYMOST_RENDERMODE_POLYGL()) {
        if (pp->palette == waterpal) tintr=0,tintg=0,tintb=63,tintf=8;
        else if (pp->palette == slimepal) tintr=0,tintg=63,tintb=0,tintf=8;
    }

    // this does pain tinting etc from the CON
    if( pp->pals_time >= 0 && pp->loogcnt == 0) // JBF 20040101: pals_time > 0 now >= 0
    {
        fader = pp->pals[0];
        fadeg = pp->pals[1];
        fadeb = pp->pals[2];
        fadef = pp->pals_time;
        restorepalette = 1;     // JBF 20040101
        dotint = 1;
    }
    // reset a normal palette
    else if( restorepalette )
    {
        //setbrightness(ud.brightness>>2,&pp->palette[0],0);
        setgamepalette(pp,pp->palette,0);
        restorepalette = 0;
    }
    // loogies courtesy of being snotted on
    else if(pp->loogcnt > 0) {
        //palto(0,64,0,(pp->loogcnt>>1)+128);
        fader = 0;
        fadeg = 64;
        fadeb = 0;
        fadef = pp->loogcnt>>1;
        dotint = 1;
    }
    if (fadef > tintf) {
        tintr = fader;
        tintg = fadeg;
        tintb = fadeb;
        tintf = fadef;
    }

    if(ud.show_help)
    {
        switch(ud.show_help)
        {
            case 1:
                rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
            case 2:
                rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
        }
        if (tintf > 0 || dotint) palto(tintr,tintg,tintb,tintf|128);
        return;
    }

    i = pp->cursectnum;

    show2dsector[i>>3] |= (1<<(i&7));
    wal = &wall[sector[i].wallptr];
    for(j=sector[i].wallnum;j>0;j--,wal++)
    {
        i = wal->nextsector;
        if (i < 0) continue;
        if (wal->cstat&0x0071) continue;
        if (wall[wal->nextwall].cstat&0x0071) continue;
        if (sector[i].lotag == 32767) continue;
        if (sector[i].ceilingz >= sector[i].floorz) continue;
        show2dsector[i>>3] |= (1<<(i&7));
    }

    if(ud.camerasprite == -1)
    {
        if( ud.overhead_on != 2 )
        {
            if(pp->newowner >= 0)
                cameratext(pp->newowner);
            else
            {
                displayweapon(screenpeek);
                if(pp->over_shoulder_on == 0 )
                    displaymasks(screenpeek);
            }
            moveclouds();
        }

        if( ud.overhead_on > 0 )
        {
                smoothratio = min(max(smoothratio,0),65536);
                dointerpolations(smoothratio);
                if( ud.scrollmode == 0 )
                {
                     if(pp->newowner == -1)
                     {
                         if (screenpeek == myconnectindex && numplayers > 1)
                         {
                             cposx = omyx+mulscale16((int)(myx-omyx),smoothratio);
                             cposy = omyy+mulscale16((int)(myy-omyy),smoothratio);
                             cang = omyang+mulscale16((int)(((myang+1024-omyang)&2047)-1024),smoothratio);
                         }
                         else
                         {
                              cposx = pp->oposx+mulscale16((int)(pp->posx-pp->oposx),smoothratio);
                              cposy = pp->oposy+mulscale16((int)(pp->posy-pp->oposy),smoothratio);
                              cang = pp->oang+mulscale16((int)(((pp->ang+1024-pp->oang)&2047)-1024),smoothratio);
                         }
                    }
                    else
                    {
                        cposx = pp->oposx;
                        cposy = pp->oposy;
                        cang = pp->oang;
                    }
                }
                else
                {

                     ud.fola += ud.folavel>>3;
                     ud.folx += (ud.folfvel*sintable[(512+2048-ud.fola)&2047])>>14;
                     ud.foly += (ud.folfvel*sintable[(512+1024-512-ud.fola)&2047])>>14;

                     cposx = ud.folx;
                     cposy = ud.foly;
                     cang = ud.fola;
                }

                if(ud.overhead_on == 2)
                {
                    clearview(0L);
                    drawmapview(cposx,cposy,pp->zoom,cang);
                }
                drawoverheadmap( cposx,cposy,pp->zoom,cang);

                restoreinterpolations();

                if(ud.overhead_on == 2)
                {
                    if(ud.screen_size > 0)
                    {
                        a = 159;
                        a += tilesizy[BOTTOMSTATUSBAR] - mulscale16(tilesizy[BOTTOMSTATUSBAR], sbarscale);
                    }
                    else a = 194;

                    minitext(1,a-6,volume_names[ud.volume_number],0,2+8+16);
                    minitext(1,a,level_names[ud.volume_number*11 + ud.level_number],0,2+8+16);
                }
        }
    }

    coolgaugetext(screenpeek);
    operatefta();

    if( (KB_KeyPressed(sc_Escape) || BUTTON(gamefunc_Show_Menu)) && ud.overhead_on == 0
        && ud.show_help == 0
        && ps[myconnectindex].newowner == -1)
    {
        if( (ps[myconnectindex].gm&MODE_MENU) == MODE_MENU && current_menu < 51)
        {
            KB_ClearKeyDown(sc_Escape);
            CONTROL_ClearButton(gamefunc_Show_Menu);
            ps[myconnectindex].gm &= ~MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 1;
                totalclock = ototalclock;
                cameraclock = totalclock;
                cameradist = 65536L;
            }
            walock[TILE_SAVESHOT] = 199;
            vscrn();
        }
        else if( (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU &&
            ps[myconnectindex].newowner == -1 &&
            (ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
        {
            KB_ClearKeyDown(sc_Escape);
            CONTROL_ClearButton(gamefunc_Show_Menu);
            FX_StopAllSounds();
            clearsoundlocks();

            intomenusounds();

            ps[myconnectindex].gm |= MODE_MENU;

            if(ud.multimode < 2 && ud.recstat != 2) ready2send = 0;

            if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
            else cmenu(0);
            screenpeek = myconnectindex;
        }
    }

    if(ps[myconnectindex].newowner == -1 && ud.overhead_on == 0 && ud.crosshair && ud.camerasprite == -1)
        rotatesprite((160-(ps[myconnectindex].look_ang>>1))<<16,100L<<16,65536L,0,CROSSHAIR,0,0,2+1,windowx1,windowy1,windowx2,windowy2);

    if(ps[myconnectindex].gm&MODE_TYPE)
        typemode();
    else
        menus();

    if( ud.pause_on==1 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
        menutext(160,100,0,0,"GAME PAUSED");

    if(ud.coords)
        coords(screenpeek);
    if(ud.tickrate)
        tics();

    // JBF 20040124: display level stats in screen corner
    if(ud.levelstats && (ps[myconnectindex].gm&MODE_MENU) == 0) {
        if (ud.screen_size > 0)
        {
            i = 159;
            i += tilesizy[BOTTOMSTATUSBAR] - mulscale16(tilesizy[BOTTOMSTATUSBAR], sbarscale);
        }
        else i = 194;
        if (ud.overhead_on == 2) i -= 6+6+4;

                    // minitext(1,a+6,volume_names[ud.volume_number],0,2+8+16);
                    // minitext(1,a+12,level_names[ud.volume_number*11 + ud.level_number],0,2+8+16);

        sprintf(buf,"Time: %d:%02d",
            (ps[myconnectindex].player_par/(26*60)),
            (ps[myconnectindex].player_par/26)%60);
        minitext(1,i-6-6,buf,0,2+8+16);

        if(ud.player_skill > 3 )
            sprintf(buf,"Kills: %d",ps[myconnectindex].actors_killed);
        else
            sprintf(buf,"Kills: %d/%d",ps[myconnectindex].actors_killed,
                ps[myconnectindex].max_actors_killed>ps[myconnectindex].actors_killed?
                ps[myconnectindex].max_actors_killed:ps[myconnectindex].actors_killed);
        minitext(1,i-6,buf,0,2+8+16);

        sprintf(buf,"Secrets: %d/%d", ps[myconnectindex].secret_rooms,ps[myconnectindex].max_secret_rooms);
        minitext(1,i,buf,0,2+8+16);
    }
    if (tintf > 0 || dotint) palto(tintr,tintg,tintb,tintf|128);
}


void view(struct player_struct *pp, int *vx, int *vy,int *vz,short *vsectnum, short ang, short horiz)
{
     spritetype *sp;
     int i, nx, ny, nz, hx, hy, hitx, hity, hitz;
     short bakcstat, hitsect, hitwall, hitsprite, daang;

     nx = (sintable[(ang+1536)&2047]>>4);
     ny = (sintable[(ang+1024)&2047]>>4);
     nz = (horiz-100)*128;

     sp = &sprite[pp->i];

     bakcstat = sp->cstat;
     sp->cstat &= (short)~0x101;

     updatesectorz(*vx,*vy,*vz,vsectnum);
     hitscan(*vx,*vy,*vz,*vsectnum,nx,ny,nz,&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

     if(*vsectnum < 0)
     {
        sp->cstat = bakcstat;
        return;
     }

     hx = hitx-(*vx); hy = hity-(*vy);
     if (klabs(nx)+klabs(ny) > klabs(hx)+klabs(hy))
     {
         *vsectnum = hitsect;
         if (hitwall >= 0)
         {
             daang = getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,
                                    wall[wall[hitwall].point2].y-wall[hitwall].y);

             i = nx*sintable[daang]+ny*sintable[(daang+1536)&2047];
             if (klabs(nx) > klabs(ny)) hx -= mulscale28(nx,i);
                                          else hy -= mulscale28(ny,i);
         }
         else if (hitsprite < 0)
         {
             if (klabs(nx) > klabs(ny)) hx -= (nx>>5);
                                          else hy -= (ny>>5);
         }
         if (klabs(nx) > klabs(ny)) i = divscale16(hx,nx);
                                      else i = divscale16(hy,ny);
         if (i < cameradist) cameradist = i;
     }
     *vx = (*vx)+mulscale16(nx,cameradist);
     *vy = (*vy)+mulscale16(ny,cameradist);
     *vz = (*vz)+mulscale16(nz,cameradist);

     cameradist = min(cameradist+((totalclock-cameraclock)<<10),65536);
     cameraclock = totalclock;

     updatesectorz(*vx,*vy,*vz,vsectnum);

     sp->cstat = bakcstat;
}

    //REPLACE FULLY
void drawbackground(void)
{
    short dapicnum;
    int x,y,x1,y1,x2,y2,rx;
#define ROUND16(f) (((f)>>16)+(((f)&0x8000)>>15))

    flushperms();

    switch(ud.m_volume_number)
    {
        default:dapicnum = BIGHOLE;break;
        case 1:dapicnum = BIGHOLE;break;
        case 2:dapicnum = BIGHOLE;break;
    }

    if (tilesizx[dapicnum] == 0 || tilesizy[dapicnum] == 0) {
        pus = pub = NUMPAGES;
        return;
    }

    y1 = 0; y2 = ydim;
    if( ready2send || (ps[myconnectindex].gm&MODE_GAME) || ud.recstat == 2 )
    {
        if(ud.coop != 1)
        {
            if (ud.multimode > 1) y1 += tilesizy[FRAGBAR];
            if (ud.multimode > 4) y1 += tilesizy[FRAGBAR];
            if (ud.multimode > 8) y1 += tilesizy[FRAGBAR];
            if (ud.multimode > 12) y1 += tilesizy[FRAGBAR];
        }
        y1 = scale(y1 * sbarscale, ydim, 200)>>16;
        if (ud.screen_size >= 8)
            y2 = ROUND16(scale(200<<16,ydim,200)-scale(tilesizy[BOTTOMSTATUSBAR] * sbarscale,ydim,200) + 32768);
    } else {
        // when not rendering a game, fullscreen wipe
        for(y=0;y<ydim;y+=tilesizy[dapicnum])
            for(x=0;x<xdim;x+=tilesizx[dapicnum])
                rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,0,xdim-1,ydim-1);
        return;
    }

    if(ud.screen_size > 8)
    {
        // across top
        for (y=y1-(y1%tilesizy[dapicnum]); y<windowy1; y+=tilesizy[dapicnum])
            for (x=0; x<xdim; x+=tilesizx[dapicnum])
                rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,y1,xdim-1,windowy1-1);

        // sides
        rx = windowx2-windowx2%tilesizx[dapicnum];
        for (y=windowy1-windowy1%tilesizy[dapicnum]; y<windowy2; y+=tilesizy[dapicnum])
            for (x=0; x<windowx1 || x+rx<xdim; x+=tilesizx[dapicnum]) {
                rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,windowy1,windowx1-1,windowy2);
                rotatesprite((x+rx)<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,windowx2+1,windowy1,xdim-1,windowy2);
            }

        // along bottom
        for (y=windowy2-(windowy2%tilesizy[dapicnum]); y<y2; y+=tilesizy[dapicnum])
            for (x=0; x<xdim; x+=tilesizx[dapicnum])
                rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,windowy2+1,xdim-1,y2-1);
    }

    // draw in the bits to the left and right of the status bar and fragbar
    if (ud.screen_size >= 8 || ud.multimode > 1) {
        int cl, cr, barw;
        barw = scale(320*sbarscale,ydim<<16,200*pixelaspect);
        cl = ((xdim<<16)-barw)>>17;
        cr = ((xdim<<16)+barw)>>17;

        for(y=0; y<y1; y+=tilesizy[dapicnum])
            for(x=0;x<=xdim-1; x+=tilesizx[dapicnum]) {
                if (x<cl)
                    rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,0,cl,y1);
                else if (x+tilesizx[dapicnum]>cr)
                    rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,cr,0,xdim-1,y1);
            }

        if (ud.screen_size >= 8)
            for(y=y2-y2%tilesizy[dapicnum]; y<ydim-1; y+=tilesizy[dapicnum])
                for(x=0;x<=xdim-1; x+=tilesizx[dapicnum]) {
                    if (x<cl)
                        rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,y2,cl,ydim-1);
                    else if (x+tilesizx[dapicnum]>cr)
                        rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,cr,y2,xdim-1,ydim-1);
                }
    }

     if(ud.screen_size > 8)
     {
          y = 0;
          if(ud.coop != 1)
          {
             if (ud.multimode > 1) y += 8;
             if (ud.multimode > 4) y += 8;
          }

          x1 = max(windowx1-4,0);
          y1 = max(windowy1-4,y);
          x2 = min(windowx2+4,xdim-1);
          y2 = min(windowy2+4,scale(ydim,200-mulscale16(tilesizy[BOTTOMSTATUSBAR], sbarscale),200)-1);

          for(y=y1+4;y<y2-4;y+=64)
          {
                rotatesprite(x1<<16,y<<16,65536L,0,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite((x2+1)<<16,(y+64)<<16,65536L,1024,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          for(x=x1+4;x<x2-4;x+=64)
          {
                rotatesprite((x+64)<<16,y1<<16,65536L,512,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite(x<<16,(y2+1)<<16,65536L,1536,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          rotatesprite(x1<<16,y1<<16,65536L,0,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,y1<<16,65536L,512,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,(y2+1)<<16,65536L,1024,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite(x1<<16,(y2+1)<<16,65536L,1536,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
     }

     pus = pub = NUMPAGES;
}


// Floor Over Floor

// If standing in sector with SE42
// then draw viewing to SE41 and raise all =hi SE43 cielings.

// If standing in sector with SE43
// then draw viewing to SE40 and lower all =hi SE42 floors.

// If standing in sector with SE44
// then draw viewing to SE40.

// If standing in sector with SE45
// then draw viewing to SE41.

#define FOFTILE 13
#define FOFTILEX 32
#define FOFTILEY 32
int tempsectorz[MAXSECTORS];
int tempsectorpicnum[MAXSECTORS];
//short tempcursectnum;

void SE40_Draw(int spnum,int x,int y,int z,short a,short h,int smoothratio)
{
 int i=0,j=0,k=0;
 int floor1=0,floor2=0,ok=0,fofmode=0;
 int offx,offy;

 if(sprite[spnum].ang!=512) return;

 i = FOFTILE;    //Effect TILE
 if (!(gotpic[i>>3]&(1<<(i&7)))) return;
 gotpic[i>>3] &= ~(1<<(i&7));

 floor1=spnum;

 if(sprite[spnum].lotag==42) fofmode=40;
 if(sprite[spnum].lotag==43) fofmode=41;
 if(sprite[spnum].lotag==44) fofmode=40;
 if(sprite[spnum].lotag==45) fofmode=41;

// fofmode=sprite[spnum].lotag-2;

// sectnum=sprite[j].sectnum;
// sectnum=cursectnum;
 ok++;

/*  recursive?
 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].sectnum==sectnum &&
     sprite[j].picnum==1 &&
     sprite[j].lotag==110
    ) { DrawFloorOverFloor(j); break;}
 }
*/

// if(ok==0) { Message("no fof",RED); return; }

 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].picnum==1 &&
     sprite[j].lotag==fofmode &&
     sprite[j].hitag==sprite[floor1].hitag
    ) { floor1=j; fofmode=sprite[j].lotag; ok++; break;}
 }
// if(ok==1) { Message("no floor1",RED); return; }

 if(fofmode==40) k=41; else k=40;

 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].picnum==1 &&
     sprite[j].lotag==k &&
     sprite[j].hitag==sprite[floor1].hitag
    ) {floor2=j; ok++; break;}
 }

// if(ok==2) { Message("no floor2",RED); return; }

 for(j=0;j<MAXSPRITES;j++)  // raise ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==40)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].floorz;
      sector[sprite[j].sectnum].floorz+=(((z-sector[sprite[j].sectnum].floorz)/32768)+1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].floorpicnum;
      sector[sprite[j].sectnum].floorpicnum=13;
     }
     if(k==41)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingz;
      sector[sprite[j].sectnum].ceilingz+=(((z-sector[sprite[j].sectnum].ceilingz)/32768)-1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingpicnum;
      sector[sprite[j].sectnum].ceilingpicnum=13;
     }
    }
 }

 i=floor1;
 offx=x-sprite[i].x;
 offy=y-sprite[i].y;
 i=floor2;
 drawrooms(offx+sprite[i].x,offy+sprite[i].y,z,a,h,sprite[i].sectnum);
 animatesprites(x,y,a,smoothratio);
 drawmasks();

 for(j=0;j<MAXSPRITES;j++)  // restore ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==40)
     {sector[sprite[j].sectnum].floorz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].floorpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
     if(k==41)
     {sector[sprite[j].sectnum].ceilingz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].ceilingpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
    }// end if
 }// end for

} // end SE40




void se40code(int x,int y,int z,int a,int h, int smoothratio)
{
    int i;

    i = headspritestat[15];
    while(i >= 0)
    {
        switch(sprite[i].lotag)
        {
//            case 40:
//            case 41:
//                SE40_Draw(i,x,y,a,smoothratio);
//                break;
            case 42:
            case 43:
            case 44:
            case 45:
                if(ps[screenpeek].cursectnum == sprite[i].sectnum)
                    SE40_Draw(i,x,y,z,a,h,smoothratio);
                break;
        }
        i = nextspritestat[i];
    }
}

static int oyrepeat=-1;

void displayrooms(short snum,int smoothratio)
{
    int cposx,cposy,cposz,dst,j,fz,cz;
    short sect, cang, k, choriz;
    struct player_struct *p;
    int tposx,tposy,i;
    short tang;
    int tiltcx,tiltcy,tiltcs=0; // JBF 20030807

    p = &ps[snum];

    if(pub > 0)
    {
        drawbackground();
        pub = 0;
    }

    if( ud.overhead_on == 2 || ud.show_help || p->cursectnum == -1)
        return;

    smoothratio = min(max(smoothratio,0),65536);

    visibility = p->visibility;

    if(ud.pause_on || ps[snum].on_crane > -1) smoothratio = 65536;

    sect = p->cursectnum;
    if(sect < 0 || sect >= MAXSECTORS) return;

    dointerpolations(smoothratio);

    animatecamsprite();

    if(ud.camerasprite >= 0)
    {
        spritetype *s;

        s = &sprite[ud.camerasprite];

        if(s->yvel < 0) s->yvel = -100;
        else if(s->yvel > 199) s->yvel = 300;

        cang = hittype[ud.camerasprite].tempang+mulscale16((int)(((s->ang+1024-hittype[ud.camerasprite].tempang)&2047)-1024),smoothratio);

        se40code(s->x,s->y,s->z,cang,s->yvel,smoothratio);

        drawrooms(s->x,s->y,s->z-(4<<8),cang,s->yvel,s->sectnum);
        animatesprites(s->x,s->y,cang,smoothratio);
        drawmasks();
    }
    else
    {
        i = divscale22(1,sprite[p->i].yrepeat+28);
        if (i != oyrepeat)
        {
            oyrepeat = i;
            setaspect(oyrepeat,yxaspect);
        }

        if(screencapt)
        {
            walock[TILE_SAVESHOT] = 199;
            if (waloff[TILE_SAVESHOT] == 0)
                allocache((void **)&waloff[TILE_SAVESHOT],200*320,&walock[TILE_SAVESHOT]);
            setviewtotile(TILE_SAVESHOT,200L,320L);
        }
        else if( POLYMOST_RENDERMODE_CLASSIC() && ( ( ud.screen_tilting && p->rotscrnang ) || ud.detail==0 ) )
        {
                if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;

        if (xres <= 320 && yres <= 240) {   // JBF 20030807: Increased tilted-screen quality
            tiltcs = 1;
            tiltcx = 320;
            tiltcy = 200;
        } else {
            tiltcs = 2;
            tiltcx = 640;
            tiltcy = 480;
        }

                walock[TILE_TILT] = 255;
                if (waloff[TILE_TILT] == 0)
                    allocache((void **)&waloff[TILE_TILT],tiltcx*tiltcx,&walock[TILE_TILT]);
                if ((tang&1023) == 0)
                    setviewtotile(TILE_TILT,tiltcy>>(1-ud.detail),tiltcx>>(1-ud.detail));
                else
                    setviewtotile(TILE_TILT,tiltcx>>(1-ud.detail),tiltcx>>(1-ud.detail));
                if ((tang&1023) == 512)
                {     //Block off unscreen section of 90¯ tilted screen
                    j = ((tiltcx-(60*tiltcs))>>(1-ud.detail));
                    for(i=((60*tiltcs)>>(1-ud.detail))-1;i>=0;i--)
                    {
                        startumost[i] = 1; startumost[i+j] = 1;
                        startdmost[i] = 0; startdmost[i+j] = 0;
                    }
                }

                i = (tang&511); if (i > 256) i = 512-i;
                i = sintable[i+512]*8 + sintable[i]*5L;
                setaspect(i>>1,yxaspect);
        }
#if USE_POLYMOST
        else if (POLYMOST_RENDERMODE_POLYMOST() /*&& (p->rotscrnang || p->orotscrnang)*/) {
            setrollangle(p->orotscrnang + mulscale16(((p->rotscrnang - p->orotscrnang + 1024)&2047)-1024,smoothratio));
            p->orotscrnang = p->rotscrnang; // JBF: save it for next time
        }
#endif

          if ( (snum == myconnectindex) && (numplayers > 1) )
                  {
                                cposx = omyx+mulscale16((int)(myx-omyx),smoothratio);
                                cposy = omyy+mulscale16((int)(myy-omyy),smoothratio);
                                cposz = omyz+mulscale16((int)(myz-omyz),smoothratio);
                                cang = omyang+mulscale16((int)(((myang+1024-omyang)&2047)-1024),smoothratio);
                                choriz = omyhoriz+omyhorizoff+mulscale16((int)(myhoriz+myhorizoff-omyhoriz-omyhorizoff),smoothratio);
                                sect = mycursectnum;
                  }
                  else
                  {
                                cposx = p->oposx+mulscale16((int)(p->posx-p->oposx),smoothratio);
                                cposy = p->oposy+mulscale16((int)(p->posy-p->oposy),smoothratio);
                                cposz = p->oposz+mulscale16((int)(p->posz-p->oposz),smoothratio);
                                cang = p->oang+mulscale16((int)(((p->ang+1024-p->oang)&2047)-1024),smoothratio);
                                choriz = p->ohoriz+p->ohorizoff+mulscale16((int)(p->horiz+p->horizoff-p->ohoriz-p->ohorizoff),smoothratio);
                  }
                  cang += p->look_ang;

                  if (p->newowner >= 0)
                  {
                                cang = p->ang+p->look_ang;
                                choriz = p->horiz+p->horizoff;
                                cposx = p->posx;
                                cposy = p->posy;
                                cposz = p->posz;
                                sect = sprite[p->newowner].sectnum;
                                smoothratio = 65536L;
                  }

                  else if( p->over_shoulder_on == 0 )
                                cposz += p->opyoff+mulscale16((int)(p->pyoff-p->opyoff),smoothratio);
                  else view(p,&cposx,&cposy,&cposz,&sect,cang,choriz);

        cz = hittype[p->i].ceilingz;
        fz = hittype[p->i].floorz;

        if(earthquaketime > 0 && p->on_ground == 1)
        {
            cposz += 256-(((earthquaketime)&1)<<9);
            cang += (2-((earthquaketime)&2))<<2;
        }

        if(sprite[p->i].pal == 1) cposz -= (18<<8);

        if(p->newowner >= 0)
            choriz = 100+sprite[p->newowner].shade;
        else if(p->spritebridge == 0)
        {
            if( cposz < ( p->truecz + (4<<8) ) ) cposz = cz + (4<<8);
            else if( cposz > ( p->truefz - (4<<8) ) ) cposz = fz - (4<<8);
        }

        if (sect >= 0)
        {
            getzsofslope(sect,cposx,cposy,&cz,&fz);
            if (cposz < cz+(4<<8)) cposz = cz+(4<<8);
            if (cposz > fz-(4<<8)) cposz = fz-(4<<8);
        }

        if(choriz > 299) choriz = 299;
        else if(choriz < -99) choriz = -99;

        se40code(cposx,cposy,cposz,cang,choriz,smoothratio);

        if ((gotpic[MIRROR>>3]&(1<<(MIRROR&7))) > 0)
        {
            dst = 0x7fffffff; i = 0;
            for(k=0;k<mirrorcnt;k++)
            {
                j = klabs(wall[mirrorwall[k]].x-cposx);
                j += klabs(wall[mirrorwall[k]].y-cposy);
                if (j < dst) dst = j, i = k;
            }

            if( wall[mirrorwall[i]].overpicnum == MIRROR )
            {
                preparemirror(cposx,cposy,cposz,cang,choriz,mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);

                j = visibility;
                visibility = (j>>1) + (j>>2);

                drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]+MAXSECTORS);

                display_mirror = 1;
                animatesprites(tposx,tposy,tang,smoothratio);
                display_mirror = 0;

                drawmasks();
                completemirror();   //Reverse screen x-wise in this function
                visibility = j;
            }
            gotpic[MIRROR>>3] &= ~(1<<(MIRROR&7));
        }

        drawrooms(cposx,cposy,cposz,cang,choriz,sect);
        animatesprites(cposx,cposy,cang,smoothratio);
        drawmasks();

        if(screencapt == 1)
        {
            setviewback();
            screencapt = 0;
//            walock[TILE_SAVESHOT] = 1;
        }
        else if( POLYMOST_RENDERMODE_CLASSIC() && ( ( ud.screen_tilting && p->rotscrnang) || ud.detail==0 ) )
        {
            if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;

            setviewback();
            picanm[TILE_TILT] &= 0xff0000ff;
                i = (tang&511); if (i > 256) i = 512-i;
                i = sintable[i+512]*8 + sintable[i]*5L;
                if ((1-ud.detail) == 0) i >>= 1;
            i>>=(tiltcs-1); // JBF 20030807
                rotatesprite(160<<16,100<<16,i,tang+512,TILE_TILT,0,0,4+2+64,windowx1,windowy1,windowx2,windowy2);
            walock[TILE_TILT] = 199;
        }
    }

    restoreinterpolations();

    if (totalclock < lastvisinc)
    {
        if (klabs(p->visibility-ud.const_visibility) > 8)
            p->visibility += (ud.const_visibility-p->visibility)>>2;
    }
    else p->visibility = ud.const_visibility;
}





short LocateTheLocator(short n,short sn)
{
    short i;

    i = headspritestat[7];
    while(i >= 0)
    {
        if( (sn == -1 || sn == SECT) && n == SLT )
            return i;
        i = nextspritestat[i];
    }
    return -1;
}

short EGS(short whatsect,int s_x,int s_y,int s_z,short s_pn,signed char s_s,signed char s_xr,signed char s_yr,short s_a,short s_ve,int s_zv,short s_ow,signed char s_ss)
{
    short i;
    spritetype *s;

    i = insertsprite(whatsect,s_ss);

    if( i < 0 )
        gameexit(" Too many sprites spawned.");

    hittype[i].bposx = s_x;
    hittype[i].bposy = s_y;
    hittype[i].bposz = s_z;

    s = &sprite[i];

    s->x = s_x;
    s->y = s_y;
    s->z = s_z;
    s->cstat = 0;
    s->picnum = s_pn;
    s->shade = s_s;
    s->xrepeat = s_xr;
    s->yrepeat = s_yr;
    s->pal = 0;

    s->ang = s_a;
    s->xvel = s_ve;
    s->zvel = s_zv;
    s->owner = s_ow;
    s->xoffset = 0;
    s->yoffset = 0;
    s->yvel = 0;
    s->clipdist = 0;
    s->pal = 0;
    s->lotag = 0;

    hittype[i].picnum = sprite[s_ow].picnum;

    hittype[i].lastvx = 0;
    hittype[i].lastvy = 0;

    hittype[i].timetosleep = 0;
    hittype[i].actorstayput = -1;
    hittype[i].extra = -1;
    hittype[i].owner = s_ow;
    hittype[i].cgg = 0;
    hittype[i].movflag = 0;
    hittype[i].tempang = 0;
    hittype[i].dispicnum = 0;
    hittype[i].floorz = hittype[s_ow].floorz;
    hittype[i].ceilingz = hittype[s_ow].ceilingz;

    T1=T3=T4=T6=0;
    if( actorscrptr[s_pn] )
    {
        s->extra = *actorscrptr[s_pn];
        T5 = *(actorscrptr[s_pn]+1);
        T2 = *(actorscrptr[s_pn]+2);
        s->hitag = *(actorscrptr[s_pn]+3);
    }
    else
    {
        T2=T5=0;
        s->extra = 0;
        s->hitag = 0;
    }

    if (show2dsector[SECT>>3]&(1<<(SECT&7))) show2dsprite[i>>3] |= (1<<(i&7));
    else show2dsprite[i>>3] &= ~(1<<(i&7));

    clearbufbyte(&spriteext[i], sizeof(spriteexttype), 0);
/*
    if(s->sectnum < 0)
    {
        s->xrepeat = s->yrepeat = 0;
        changespritestat(i,5);
    }
*/
    return(i);
}

char wallswitchcheck(short i)
{
    switch(PN)
    {
        case HANDPRINTSWITCH:
        case HANDPRINTSWITCH+1:
        case ALIENSWITCH:
        case ALIENSWITCH+1:
        case MULTISWITCH:
        case MULTISWITCH+1:
        case MULTISWITCH+2:
        case MULTISWITCH+3:
        case ACCESSSWITCH:
        case ACCESSSWITCH2:
        case PULLSWITCH:
        case PULLSWITCH+1:
        case HANDSWITCH:
        case HANDSWITCH+1:
        case SLOTDOOR:
        case SLOTDOOR+1:
        case LIGHTSWITCH:
        case LIGHTSWITCH+1:
        case SPACELIGHTSWITCH:
        case SPACELIGHTSWITCH+1:
        case SPACEDOORSWITCH:
        case SPACEDOORSWITCH+1:
        case FRANKENSTINESWITCH:
        case FRANKENSTINESWITCH+1:
        case LIGHTSWITCH2:
        case LIGHTSWITCH2+1:
        case POWERSWITCH1:
        case POWERSWITCH1+1:
        case LOCKSWITCH1:
        case LOCKSWITCH1+1:
        case POWERSWITCH2:
        case POWERSWITCH2+1:
        case DIPSWITCH:
        case DIPSWITCH+1:
        case DIPSWITCH2:
        case DIPSWITCH2+1:
        case TECHSWITCH:
        case TECHSWITCH+1:
        case DIPSWITCH3:
        case DIPSWITCH3+1:
            return 1;
    }
    return 0;
}


int tempwallptr;
short spawn( short j, short pn )
{
    short i, s, startwall, endwall, sect, clostest=0;
    int x, y, d;
    spritetype *sp;

    if(j >= 0)
    {
        i = EGS(sprite[j].sectnum,sprite[j].x,sprite[j].y,sprite[j].z
            ,pn,0,0,0,0,0,0,j,0);
        hittype[i].picnum = sprite[j].picnum;
    }
    else
    {
        i = pn;

        hittype[i].picnum = PN;
        hittype[i].timetosleep = 0;
        hittype[i].extra = -1;

        hittype[i].bposx = SX;
        hittype[i].bposy = SY;
        hittype[i].bposz = SZ;

        OW = hittype[i].owner = i;
        hittype[i].cgg = 0;
        hittype[i].movflag = 0;
        hittype[i].tempang = 0;
        hittype[i].dispicnum = 0;
        hittype[i].floorz = sector[SECT].floorz;
        hittype[i].ceilingz = sector[SECT].ceilingz;

        hittype[i].lastvx = 0;
        hittype[i].lastvy = 0;
        hittype[i].actorstayput = -1;

        T1 = T2 = T3 = T4 = T5 = T6 = 0;

        if( PN != SPEAKER && PN != LETTER && PN != DUCK && PN != TARGET && PN != TRIPBOMB && PN != VIEWSCREEN && PN != VIEWSCREEN2 && (CS&48) )
            if( !(PN >= CRACK1 && PN <= CRACK4) )
        {
            if(SS == 127) return i;
            if( wallswitchcheck(i) == 1 && (CS&16) )
            {
                if( PN != ACCESSSWITCH && PN != ACCESSSWITCH2 && sprite[i].pal)
                {
                    if( (ud.multimode < 2) || (ud.multimode > 1 && ud.coop==1) )
                    {
                        sprite[i].xrepeat = sprite[i].yrepeat = 0;
                        sprite[i].cstat = SLT = SHT = 0;
                        return i;
                    }
                }
                CS |= 257;
                if( sprite[i].pal && PN != ACCESSSWITCH && PN != ACCESSSWITCH2)
                    sprite[i].pal = 0;
                return i;
            }
