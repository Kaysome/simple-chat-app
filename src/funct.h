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

#ifndef __funct_h__
#define __funct_h__

struct player_struct;	// JBF: duke3d.h defines this later

extern void sendscore(char *s);
extern void SoundStartup(void );
extern void SoundShutdown(void );
extern void MusicStartup(void );
extern void MusicShutdown(void );
extern void MusicSetVolume(int);
extern int USRHOOKS_GetMem(char **ptr,unsigned int size);
extern int USRHOOKS_FreeMem(char *ptr);
extern void intomenusounds(void );
extern void playmusic(char *fn);
extern void stopmusic(void);
extern char loadsound(unsigned short num);
extern int xyzsound(short num,short i,int x,int y,int z);
extern void sound(short num);
extern int spritesound(unsigned short num,short i);
extern void stopsound(short num);
extern void stopenvsound(short num,short i);
extern void pan3dsound(void );
extern void testcallback(unsigned int num);
extern void clearsoundlocks(void);
extern short callsound(short sn,short whatsprite);
extern short check_activator_motion(short lotag);
extern char isadoorwall(short dapic);
extern char isanunderoperator(short lotag);
extern char isanearoperator(short lotag);
extern short checkcursectnums(short sect);
extern int ldist(spritetype *s1,spritetype *s2);
extern int dist(spritetype *s1,spritetype *s2);
extern short findplayer(spritetype *s,int *d);
extern short findotherplayer(short p,int *d);
extern void doanimations(void );
extern int getanimationgoal(int *animptr);
extern int setanimation(short animsect,int *animptr,int thegoal,int thevel);
extern void animatecamsprite(void );
extern void animatewalls(void );
extern char activatewarpelevators(short s,short d);
extern void operatesectors(short sn,short ii);
extern void operaterespawns(short low);
extern void operateactivators(short low,short snum);
extern void operatemasterswitches(short low);
extern void operateforcefields(short s,short low);
extern char checkhitswitch(short snum,int w,char switchtype);
extern void activatebysector(short sect,short j);
extern void checkhitwall(short spr,short dawallnum,int x,int y,int z,short atwith);
extern void checkplayerhurt(struct player_struct *p,short j);
extern char checkhitceiling(short sn);
extern void checkhitsprite(short i,short sn);
extern void allignwarpelevators(void );
extern void cheatkeys(short snum);
extern void checksectors(short snum);
extern int32 RTS_AddFile(char *filename);
extern void RTS_Init(char *filename);
extern int32 RTS_NumSounds(void );
extern int32 RTS_SoundLength(int32 lump);
extern char *RTS_GetSoundName(int32 i);
extern void RTS_ReadLump(int32 lump,void *dest);
extern void *RTS_GetSound(int32 lump);
extern void docacheit(void);
extern void xyzmirror(short i,short wn);
extern void vscrn(void );
extern void pickrandomspot(short snum);
extern void resetplayerstats(short snum);
extern void resetweapons(short snum);
extern void resetinventory(short snum);
extern void resetprestat(short snum,unsigned char g);
extern void setupbackdrop(short backpicnum);
extern void cachespritenum(short i);
extern void cachegoodsprites(void );
extern void prelevel(unsigned char g);
extern void newgame(char vn,char ln,char sk);
extern void resetpspritevars(unsigned char g);
extern void resettimevars(void );
extern void genspriteremaps(void );
extern void waitforeverybody(void);
extern char getsound(unsigned short num);
extern void precachenecessarysounds(void );
extern void cacheit(void );
extern void dofrontscreens(const char *);
extern void clearfifo(void);
extern void resetmys(void);
extern int  enterlevel(unsigned char g);
extern void backtomenu(void);
extern void setpal(struct player_struct *p);
extern void incur_damage(struct player_struct *p);
extern void quickkill(struct player_struct *p);
extern void forceplayerangle(struct player_struct *p);
extern void tracers(int x1,int y1,int z1,int x2,int y2,int z2,int n);
extern int hits(short i);
extern int hitasprite(short i,short *hitsp);
extern int hitawall(struct player_struct *p,short *hitw);
extern short aim(spritetype *s,short aang);
extern void shoot(short i,short atwith);
extern void displayloogie(short snum);
extern char animatefist(short gs,short snum);
extern char animateknee(short gs,short snum);
extern char animateknuckles(short gs,short snum);
extern void displaymasks(short snum);
extern char animatetip(short gs,short snum);
extern char animateaccess(short gs,short snum);
extern void displayweapon(short snum);
extern void getinput(short snum);
extern char doincrements(struct player_struct *p);
extern void checkweapons(struct player_struct *p);
extern void processinput(short snum);
extern void cmenu(short cm);
extern void getangplayers(short snum);
//extern int loadpheader(char spot,int32 *vn,int32 *ln,int32 *psk,char *bfn,int32 *numplr);
extern int loadplayer(signed char spot);
extern int saveplayer(si