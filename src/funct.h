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
extern void operatemasterswitches(sh