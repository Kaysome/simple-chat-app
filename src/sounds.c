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
    