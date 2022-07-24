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
    Bstrcpy(ud.ridecule[3], "Lucky Son of a Bitch.");
    Bstrcpy(ud.ridecule[4], "Hmmm....Payback time.");
    Bstrcpy(ud.ridecule[5], "You bottom dwelling scum sucker.");
    Bstrcpy(ud.ridecule[6], "Damn, you're ugly.");
    Bstrcpy(ud.ridecule[7], "Ha ha ha...Wasted!");
    Bstrcpy(ud.ridecule[8], "You suck!");
    Bstrcpy(ud.ridecule[9], "AARRRGHHHHH!!!");
 
    CONFIG_SetDefaultKeyDefinitions(CONFIG_DEFAULTS_CLASSIC);
    CONFIG_SetMouseDefaults(CONFIG_DEFAULTS_CLASSIC);
    CONFIG_SetJoystickDefaults(CONFIG_DEFAULTS_CLASSIC);

    memset(MouseDigitalFunctions, -1, sizeof(MouseDigitalFunctions));
    for (i=0; i<MAXMOUSEAXES; i++) {
        MouseAnalogueScale[i] = 65536;
        CONTROL_SetAnalogAxisScale( i, MouseAnalogueScale[i], controldevice_mouse );

        MouseDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2] );
        MouseDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( mousedigitaldefaults[i*2+1] );
        CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][0], 0, controldevice_mouse );
        CONTROL_MapDigitalAxis( i, MouseDigitalFunctions[i][1], 1, controldevice_mouse );

        MouseAnalogueAxes[i] = CONFIG_AnalogNameToNum( mouseanalogdefaults[i] );
        CONTROL_MapAnalogAxis( i, MouseAnalogueAxes[i], controldevice_mouse);
    }
    CONTROL_SetMouseSensitivity(32768);

    for (i=0; i<MAXJOYAXES; i++) {
        JoystickAnalogueScale[i] = 65536;
        JoystickAnalogueDead[i] = 1024;
        JoystickAnalogueSaturate[i] = 32767-1024;
        CONTROL_SetAnalogAxisScale( i, JoystickAnalogueScale[i], controldevice_joystick );
        CONTROL_SetJoyAxisDead(i, JoystickAnalogueDead[i]);
        CONTROL_SetJoyAxisSaturate(i, JoystickAnalogueSaturate[i]);
    }
}

void CONFIG_SetDefaultKeyDefinitions(int style)
{
    int numkeydefaults;
    char **keydefaultset;
    int i, f;

    if (style) {
        numkeydefaults = sizeof(keydefaults_modern) / sizeof(char *) / 3;
        keydefaultset = keydefaults_modern;
    } else {
        numkeydefaults = sizeof(keydefaults) / sizeof(char *) / 3;
        keydefaultset = keydefaults;
    }

    memset(KeyboardKeys, 0xff, sizeof(KeyboardKeys));
    for (i=0; i < numkeydefaults; i++) {
        f = CONFIG_FunctionNameToNum( keydefaultset[3*i+0] );
        if (f == -1) continue;
        KeyboardKeys[f][0] = KB_StringToScanCode( keydefaultset[3*i+1] );
        KeyboardKeys[f][1] = KB_StringToScanCode( keydefaultset[3*i+2] );

        if (f == gamefunc_Show_Console) OSD_CaptureKey(KeyboardKeys[f][0]);
        else CONTROL_MapKey( i, KeyboardKeys[f][0], KeyboardKeys[f][1] );
    }
}

void CONFIG_SetMouseDefaults(int style)
{
    char **mousedefaultset, **mouseclickeddefaultset;
    int i;

    if (style) {
        mousedefaultset = mousedefaults_modern;
        mouseclickeddefaultset = mouseclickeddefaults_modern;
        ud.mouseflip = 1;
        if (!ud.mouseaiming) myaimmode = 1; // Enable mouse aiming if aiming type is toggle.
    } else {
        mousedefaultset = mousedefaults;
        mouseclickeddefaultset = mouseclickeddefaults;
        ud.mouseflip = 0;
        if (!ud.mouseaiming) myaimmode = 0; // Disable mouse aiming if aiming type is toggle.
    }

    memset(MouseFunctions, -1, sizeof(MouseFunctions));
    for (i=0; i < MAXMOUSEBUTTONS; i++) {
        MouseFunctions[i][0] = CONFIG_FunctionNameToNum( mousedefaultset[i] );
        CONTROL_MapButton( MouseFunctions[i][0], i, FALSE, controldevice_mouse );
        if (i>=4) continue;

        MouseFunctions[i][1] = CONFIG_FunctionNameToNum( mouseclickeddefaultset[i] );
        CONTROL_MapButton( MouseFunctions[i][1], i, TRUE,  controldevice_mouse );
   }
}

void CONFIG_SetJoystickDefaults(int style)
{
    char **joydefaultset, **joyclickeddefaultset;
    char **joydigitaldefaultset, **joyanalogdefaultset;
    int i;

    if (style) {
        joydefaultset = joystickdefaults_modern;
        joyclickeddefaultset = joystickclickeddefaults_modern;
        joydigitaldefaultset = joystickdigitaldefaults_modern;
        joyanalogdefaultset = joystickanalogdefaults_modern;
    } else {
        joydefaultset = joystickdefaults;
        joyclickeddefaultset = joystickclickeddefaults;
        joydigitaldefaultset = joystickdigitaldefaults;
        joyanalogdefaultset = joystickanalogdefaults;
    }

    memset(JoystickFunctions, -1, sizeof(JoystickFunctions));
    for (i=0; i < MAXJOYBUTTONS; i++) {
        JoystickFunctions[i][0] = CONFIG_FunctionNameToNum( joydefaultset[i] );
        JoystickFunctions[i][1] = CONFIG_FunctionNameToNum( joyclickeddefaultset[i] );
        CONTROL_MapButton( JoystickFunctions[i][0], i, FALSE, controldevice_joystick );
        CONTROL_MapButton( JoystickFunctions[i][1], i, TRUE,  controldevice_joystick );
    }

    memset(JoystickDigitalFunctions, -1, sizeof(JoystickDigitalFunctions));
    for (i=0; i < MAXJOYAXES; i++) {
        JoystickDigitalFunctions[i][0] = CONFIG_FunctionNameToNum( joydigitaldefaultset[i*2] );
        JoystickDigitalFunctions[i][1] = CONFIG_FunctionNameToNum( joydigitaldefaultset[i*2+1] );
        CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][0], 0, controldevice_joystick );
        CONTROL_MapDigitalAxis( i, JoystickDigitalFunctions[i][1], 1, controldevice_joystick );

        JoystickAnalogueAxes[i] = CONFIG_AnalogNameToNum( joyanalogdefaultset[i] );
        CONTROL_MapAnalogAxis(i, JoystickAnalogueAxes[i], controldevice_joystick);
    }
}

/*
===================
=
= CONFIG_ReadKeys
=
===================
*/

void CONFIG_ReadKeys( void )
   {
   int32 i;
   int32 numkeyentries;
   int32 function;
   char keyname1[80];
   char keyname2[80];
   kb_scancode key1,key2;

   if (scripthandle < 0) return;

   numkeyentries = SCRIPT_NumberEntries( scripthandle,"KeyDefinitions" );

   for (i=0;i<numkeyentries;i++)
      {
      function 