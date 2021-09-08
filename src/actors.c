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

extern char numenvsnds,actor_tog;

void updateinterpolations()  //Stick at beginning of domovethings
{
    int i;

    for(i=numinterpolations-1;i>=0;i--) oldipos[i] = *curipos[i];
}


void setinterpolation(int *posptr)
{
    int i;

    if (numinterpolations >= MAXINTERPOLATIONS) return;
    for(i=numinterpolations-1;i>=0;i--)
        if (curipos[i] == posptr) return;
    curipos[numinterpolations] = posptr;
    oldipos[numinterpolations] = *posptr;
    numinterpolations++;
}

void stopinterpolation(int *posptr)
{
    int i;

    for(i=numinterpolations-1;i>=startofdynamicinterpolations;i--)
        if (curipos[i] == posptr)
        {
            numinterpolations--;
            oldipos[i] = oldipos[numinterpolations];
            bakipos[i] = bakipos[numinterpolations];
            curipos[i] = curipos[numinterpolations];
        }
}

void dointerpolations(int smoothratio)       //Stick at beginning of dr