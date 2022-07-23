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
#include "osd.h"

// we load this in to get default button and key assignments
// as well as setting up function mappings

#define __SETUP__   // JBF 20031211
#include "_functio.h"

//
// Sound variables
//
int32 FXDevice;
int32 MusicDevice;
int32 FXVolume;
int32 MusicVolume;
int32 SoundToggle;
int32 MusicToggle;
int32 VoiceToggle;
int32 AmbienceToggle;
int32 NumVoices;
int32 NumChannels;
int32 NumBits;
int32 MixRate;
int32 ReverseStereo;
char MusicParams[BMAX_PATH] = {0};

int32 UseJoystick = 1, UseMouse = 1;
int32 RunMode;
int32 AutoAim;  // JBF 20031125
int32 ShowOpponentWeapons;

// JBF 20031211: Store the input settings because
// (currently) jmact can't regurgitate them
byte KeyboardKeys[NUMGAMEFUNCTIONS][2];
int32 MouseFunctions[MAXMOUSEBUTTONS][2];
int32 MouseDigitalFunctions[MAXMOUSEAXES][2];
int32 MouseAnalogueAxes[MAXMOUSEAXES];
int32 MouseAnalogueScale[MAXMOUSEAXES];
int32 JoystickFunctions[MAXJOYBUTTONS][2];
int32 JoystickDigitalFunctions[MAXJOYAXES][2];
int32 JoystickAnalogueAxes[MAXJOYAXES];
int32 JoystickAnalogueScale[MAXJOYAXES];
int32 JoystickAnalogueDead[MAXJOYAXES];
int32 JoystickAnalogueSaturate[MAXJOYAXES];

//
// Screen variables
//

int32 ScreenMode = 1;
int32 ScreenWidth = 640;
int32 ScreenHeight = 480;
int32 ScreenBPP = 8;
int32 ForceSetup = 1;

static char setupfilename[256]={SETUPFILENAME};
static int32 scripthandle = -1;
static int32 setupread = 0;


/*
===================
=
= CONFIG_FunctionNameToNum
=
===================
*/

int32 CONFIG_FunctionNameToNum( const char * func )
   {
   int32 i;

   for (i=0;i<NUMGAMEFUNCTIONS;i++)
      {
      if (!Bstrcasecmp(func,gamefunctions[i]))
         {
         return i;
         }
      }
   return -1;
   }

/*
===================
=
= CONFIG_FunctionNumToName
=
===================
*/

const char * CONFIG_FunctionNumToName( int32 func )
   {
   if ((unsigned)func >= (unsigned)NUMGAMEFUNCTIONS)
      {
      return NULL;
      }
   else
      {
      return gamefunctions[func];
      }
   }

/*
===================
=
= CONFIG_AnalogNameToNum
=
===================
*/


int32 CONFIG_AnalogNameToNum( const char * func )
   {

   if (!Bstrcasecmp(func,"analog_turning"))
      {
      return analog_turning;
      }
   if (!Bstrcasecmp(func,"analog_strafing"))
      {
      return analog_strafing;
      }
   if (!Bstrcasecmp(func,"analog_moving"))
      {
      return analog_moving;
      }
   if (!Bstrcasecmp(func,"analog_lookingupanddown"))
      {
      return analog_lookingupanddown;
      }

   return -1;
   }


const char * CONFIG_AnalogNumToName( int32 func )
   {
   switch (func) {
    case analog_turning:
        return "analog_turning";
    case analog_strafing:
        return "analog_strafing";
    case analog_moving:
        return "analog_moving";
    case analog_lookingupanddown:
        return "analog_lookingupanddown";
   }

   return NULL;
   }


/*
===================
=
= CONFIG_SetDefaults
=
===================
*/

void CONFIG_SetDefaults( void )
{
    int32 i;

    FXDevice = 0;
    MusicDevice = 0;
    NumVoices = 16;
    NumChannels = 2;
    NumBits = 16;
    MixRate = 44100;
    SoundToggle = 1;
    MusicToggle = 1;
    VoiceToggle = 1;
    AmbienceToggle = 1;
    FXVolume = 220;
    MusicVolume = 200;
    ReverseStereo = 0;
    MusicParams[0] = 0;
    myaimmode = ps[0].aim_mode = 0;
    ud.mouseaiming = 0;
    ud.weaponswitch = 3;    // new+empty
    AutoAim = 1;
    UseJoystick = 1;
    UseMouse = 1;
    ud.mouseflip = 0;
    ud.runkey_mode = 0;
    ud.statusbarscale = 8;
    ud.screen_size = 8;
    ud.screen_tilting = 1;
    ud.shadows = 1;
    ud.detail = 1;
    ud.lockout = 0;
    ud.pwlockout[0] = '\0';
    ud.crosshair = 0;
    ud.m_marker = 1;
    ud.m_ffire = 1;
    ud.levelstats = 0;
    ShowOpponentWeapons = 0;
    Bstrcpy(ud.rtsname, "DUKE.RTS");
    Bstrcpy(myname, "Duke");

    Bstrcpy(ud.ridecule[0], "An inspiration for birth control.");
    Bstrcpy(ud.ridecule[1], "You're gonna die for that!");
    Bstrcpy(ud.ridecule[2], "It hurts to be you.");
    Bstrcpy(ud.ridecule[3], "Lucky Son of a Bitch.