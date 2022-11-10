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

//#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "duke3d.h"


#define LOUDESTVOLUME 150

#define MUSIC_ID  -65536

int backflag,numenvsnds;

static int MusicIsWaveform = 0;
static char * MusicPtr = 0;
static int MusicLen = 0;
static int MusicVoice = -1;
static int MusicPaused = 0;


/*
===================
=
= SoundStartup
=
===================
*/

void SoundStartup( void )
{
   int32 status;
    int fxdevicetype;
    void * initdata = 0;

   // if they chose None lets return
    if (FXDevice < 0) {
        return;
    } else if (FXDevice == 0) {
        fxdevicetype = ASS_AutoDetect;
    } else {
        fxdevicetype = FXDevice - 1;
    }
    
    #ifdef _WIN32
    initdata = (void *) win_gethwnd();
    #endif

   status = FX_Init( fxdevicetype, NumVoices, &NumChannels, &NumBits, &MixRate, initdata );
   if ( status == FX_Ok ) {
      FX_SetVolume( FXVolume );
      FX_SetReverseStereo(ReverseStereo);
	  status = FX_SetCallBack( testcallback );
  }

   if ( status != FX_Ok ) {
	  sprintf(buf, "Sound startup error: %s", FX_ErrorString( FX_Error ));
	  gameexit(buf);
   }
	
	FXDevice = 0;
}

/*
===================
=
= SoundShutdown
=
===================
*/

void SoundShutdown( void )
{
   int32 status;

   // if they chose None lets return
   if (FXDevice < 0)
      return;
   
   if (MusicVoice >= 0) {
      MusicShutdown();
   }

   status = FX_Shutdown();
   if ( status != FX_Ok ) {
	  sprintf(buf, "Sound shutdown error: %s", FX_ErrorString( FX_Error ));
      gameexit(buf);
   }
}

/*
===================
=
= MusicStartup
=
===================
*/

void MusicStartup( void )
   {
   int32 status;
   int musicdevicetype;

   // if they chose None lets return
   if (MusicDevice < 0) {
      return;
   } else if (MusicDevice == 0) {
      musicdevicetype = ASS_AutoDetect;
   } else {
      musicdevicetype = MusicDevice - 1;
   }
   
   status = MUSIC_Init( musicdevicetype, MusicParams );

   if ( status == MUSIC_Ok )
      {
      MUSIC_SetVolume( MusicVolume );
      }
   else
   {
      buildprintf("Couldn't find selected sound card, or, error w/ sound card itself.\n");

      SoundShutdown();
      uninittimer();
      uninitengine();
      CONTROL_Shutdown();
      CONFIG_WriteSetup();
      KB_Shutdown();
      uninitgroupfile();
      //unlink("duke3d.tmp");
      exit(-1);
   }
}

/*
===================
=
= MusicShutdown
=
===================
*/

void MusicShutdown( void )
   {
   int32 status;

   // if they chose None lets return
   if (MusicDevice < 0)
      return;
   
   stopmusic();
   
   status = MUSIC_Shutdown();
   if ( status != MUSIC_Ok )
      {
      Error( MUSIC_ErrorString( MUSIC_ErrorCode ));
      }
   }

void MusicPause( int onf )
{
   if (MusicPaused == onf || (MusicIsWaveform && MusicVoice < 0)) {
      return;
   }
   
   if (onf) {
      if (MusicIsWaveform) {
         FX_PauseSound(MusicVoice, TRUE);
      } else {
         MUSIC_Pause();
      }
   } else {
      if (MusicIsWaveform) {
         FX_PauseSound(MusicVoice, FALSE);
      } else {
         MUSIC_Continue();
      }
   }
   
   MusicPaused = onf;
}

void MusicSetVolume(int volume)
{
   if (MusicIsWaveform && MusicVoice >= 0) {
      //FX_SetVoiceVolume(MusicVoice, volume);
   } else if (!MusicIsWaveform) {
      MUSIC_SetVolume(volume);
   }
}

int USRHOOKS_GetMem(char **ptr, unsigned int size )
{
   *ptr = malloc(size);

   if (*ptr == NULL)
      return(USRHOOKS_Error);

   return( USRHOOKS_Ok);

}

int USRHOOKS_FreeMem(char *ptr)
{
   free(ptr);
   return( USRHOOKS_Ok);
}

unsigned char menunum=0;

void intomenusounds(void)
{
    short menusnds[] =
    {
        LASERTRIP_EXPLODE,
        DUKE_GRUNT,
        DUKE_LAND_HURT,
        CHAINGUN_FIRE,
        SQUISHED,
        KICK_HIT,
        PISTOL_RICOCHET,
        PISTOL_BODYHIT,
        PISTOL_FIRE,
        SHOTGUN_FIRE,
        BOS1_WALK,
        RPG_EXPLODE,
        PIPEBOMB_BOUNCE,
        PIPEBOMB_EXPLODE,
        NITEVISION_ONOFF,
        RPG_SHOOT,
        SELECT_WEAPON
    };
    sound(menusnds[menunum++]);
    menunum %= 17;
}

void playmusic(char *fn)
{
    int fp;
    char * testfn, * extension;

    if(MusicToggle == 0) return;
    if(MusicDevice < 0) return;

    stopmusic();
    
    testfn = (char *) malloc( strlen(fn) + 5 );
    strcpy(testfn, fn);
    extension = strrchr(testfn, '.');

    do {
       if (extension && !Bstrcasecmp(extension, ".mid")) {
	  // we've been asked to load a .mid file, but first
	  // let's see if there's an ogg with the same base name
	  // lying around
	  strcpy(extension, ".ogg");
	  fp = kopen4load(testfn, 0);
	  if (fp >= 0) {
             free(testfn);
	     break;
	  }
       }
       free(testfn);

       // just use what we've been given
       fp = kopen4load(fn, 0);
    } while (0);

    if (fp < 0) return;

    MusicLen = kfilelength( fp );
    MusicPtr = (char *) malloc(MusicLen);
    kread( fp, MusicPtr, MusicLen);
    kclose( fp );
    
    if (!memcmp(MusicPtr, "MThd", 4)) {
       MUSIC_PlaySong( MusicPtr, MusicLen, MUSIC_LoopSong );
       MusicIsWaveform = 0;
    } else {
       MusicVoice = FX_PlayLoopedAuto(MusicPtr, MusicLen, 0, 0, 0,
                                      MusicVolume, MusicVolume, MusicVolume,
				      FX_MUSIC_PRIORITY, MUSIC_ID);
       MusicIsWaveform = 1;
    }

    MusicPaused = 0;
}

void stopmusic(void)
{
    if (MusicIsWaveform && MusicVoice >= 0) {
       FX_StopSound(MusicVoice);
       MusicVoice = -1;
    } else if (!MusicIsWaveform) {
       MUSIC_StopSong();
    }

    MusicPaused = 0;

    if (MusicPtr) {
       free(MusicPtr);
       MusicPtr = 0;
       MusicLen = 0;
    }
}

char loadsound(unsigned short num)
{
    int   fp, l;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
    if (FXDevice < 0) return 0;

    fp = kopen4load(sounds[num],loadfromgrouponly);
    if(fp == -1)
    {
        sprintf(&fta_quotes[113][0],"Sound %s(#%d) not found.",sounds[num],num);
        FTA(113,&ps[myconnectindex]);
        return 0;
    }

    l = kfilelength( fp );
    soundsiz[num] = l;

    Sound[num].lock = 200;

    allocache((void **)&Sound[num].ptr,l,&Sound[num].lock);
    kread( fp, Sound[num].ptr , l);
    kclose( fp );
    return 1;
}

int xyzsound(short num,short i,int x,int y,int z)
{
    int sndist, cx, cy, cz, j,k;
    short pitche,pitchs,cs;
    int voice, sndang, ca, pitch;

//    if(num != 358) return 0;

    if( num >= NUM_SOUNDS ||
        FXDevice < 0 ||
        ( (soundm[num]&8) && ud.lockout ) ||
        SoundToggle == 0 ||
        Sound[num].num > 3 ||
        FX_VoiceAvailable(soundpr[num]) == 0 ||
        (ps[myconnectindex].timebeforeexit > 0 && ps[myconnectindex].timebeforeexit <= 26*3) ||
        ps[myconnectindex].gm&MODE_MENU) return -1;

    if( soundm[num]&128 )
    {
        sound(num);
        return 0;
    }

    if( soundm[num]&4 )
    {
        if(VoiceToggle==0 || (ud.multimode > 1 && PN == APLAYER && sprite[i].yvel != screenpeek && ud.coop != 1) ) return -1;

        for(j=0;j<NUM_SOUNDS;j++)
          for(k=0;k<Sound[j].num;k++)
            if( (Sound[j].num > 0) && (soundm[j]&4) )
              return -1;
    }

    cx = ps[screenpeek].oposx;
    cy = ps[screenpeek].oposy;
    cz = ps[screenpeek].oposz;
    cs = ps[screenpeek].cursectnum;
    ca = ps[screenpeek].ang+ps[screenpeek].look_ang;

    sndist = FindDistance3D((cx-x),(cy-y),(cz-z)>>4);

    if( 