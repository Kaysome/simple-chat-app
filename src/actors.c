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

void dointerpolations(int smoothratio)       //Stick at beginning of drawscreen
{
    int i, j, odelta, ndelta;

    ndelta = 0; j = 0;
    for(i=numinterpolations-1;i>=0;i--)
    {
        bakipos[i] = *curipos[i];
        odelta = ndelta; ndelta = (*curipos[i])-oldipos[i];
        if (odelta != ndelta) j = mulscale16(ndelta,smoothratio);
        *curipos[i] = oldipos[i]+j;
    }
}

void restoreinterpolations()  //Stick at end of drawscreen
{
    int i;

    for(i=numinterpolations-1;i>=0;i--) *curipos[i] = bakipos[i];
}

int ceilingspace(short sectnum)
{
    if( (sector[sectnum].ceilingstat&1) && sector[sectnum].ceilingpal == 0 )
    {
        switch(sector[sectnum].ceilingpicnum)
        {
            case MOONSKY1:
            case BIGORBIT1:
                return 1;
        }
    }
    return 0;
}

int floorspace(short sectnum)
{
    if( (sector[sectnum].floorstat&1) && sector[sectnum].ceilingpal == 0 )
    {
        switch(sector[sectnum].floorpicnum)
        {
            case MOONSKY1:
            case BIGORBIT1:
                return 1;
        }
    }
    return 0;
}

void addammo( short weapon,struct player_struct *p,short amount)
{
   p->ammo_amount[weapon] += amount;

   if( p->ammo_amount[weapon] > max_ammo_amount[weapon] )
        p->ammo_amount[weapon] = max_ammo_amount[weapon];
}

void addweaponnoswitch( struct player_struct *p, short weapon)
{
    if ( p->gotweapon[weapon] == 0 )
    {
        p->gotweapon[weapon] = 1;
        if(weapon == SHRINKER_WEAPON)
            p->gotweapon[GROW_WEAPON] = 1;
    }

    switch(weapon)
    {
        case KNEE_WEAPON:
        case TRIPBOMB_WEAPON:
        case HANDREMOTE_WEAPON:
        case HANDBOMB_WEAPON:     break;
        case SHOTGUN_WEAPON:      spritesound(SHOTGUN_COCK,p->i);break;
        case PISTOL_WEAPON:       spritesound(INSERT_CLIP,p->i);break;
        default:      spritesound(SELECT_WEAPON,p->i);break;
    }
}

void addweapon( struct player_struct *p,short weapon)
{
    addweaponnoswitch(p,weapon);
    p->random_club_frame = 0;

    if(p->holster_weapon == 0)
    {
        p->weapon_pos = -1;
        p->last_weapon = p->curr_weapon;
    }
    else
    {
        p->weapon_pos = 10;
        p->holster_weapon = 0;
        p->last_weapon = -1;
    }

    p->kickback_pic = 0;
    p->curr_weapon = weapon;
}

void checkavailinven( struct player_struct *p )
{

    if(p->firstaid_amount > 0)
        p->inven_icon = 1;
    else if(p->steroids_amount > 0)
        p->inven_icon = 2;
    else if(p->holoduke_amount > 0)
        p->inven_icon = 3;
    else if(p->jetpack_amount > 0)
        p->inven_icon = 4;
    else if(p->heat_amount > 0)
        p->inven_icon = 5;
    else if(p->scuba_amount > 0)
        p->inven_icon = 6;
    else if(p->boot_amount > 0)
        p->inven_icon = 7;
    else p->inven_icon = 0;
}

void checkavailweapon( struct player_struct *p )
{
    short i,snum;
    int32 weap;

    if(p->wantweaponfire >= 0)
    {
        weap = p->wantweaponfire;
        p->wantweaponfire = -1;

        if(weap == p->curr_weapon) return;
        else if( p->gotweapon[weap] && p->ammo_amount[weap] > 0 )
        {
            addweapon(p,weap);
            return;
        }
    }

    weap = p->curr_weapon;
    if( p->gotweapon[weap] && p->ammo_amount[weap] > 0 )
        return;
    if( p->gotweapon[weap] && !(p->weaponswitch & 2))
        return;
    
    snum = sprite[p->i].yvel;

    for(i=0;i<10;i++)
    {
        weap = ud.wchoice[snum][i];
        if (VOLUMEONE && weap > 6) continue;

        if(weap == 0) weap = 9;
        else weap--;

        if( weap == 0 || ( p->gotweapon[weap] && p->ammo_amount[weap] > 0 ) )
            break;
    }

    if(i == 10) weap = 0;

    // Found the weapon

    p->last_weapon  = p->curr_weapon;
    p->random_club_frame = 0;
    p->curr_weapon  = weap;
    p->kickback_pic = 0;
    if(p->holster_weapon == 1)
    {
        p->holster_weapon = 0;
        p->weapon_pos = 10;
    }
    else p->weapon_pos   = -1;
}

int ifsquished(short i, short p)
{
    sectortype *sc;
    char squishme;
    int floorceildist;

    if(PN == APLAYER && ud.clipping)
        return 0;

    sc = &sector[SECT];
    floorceildist = sc->floorz - sc->ceilingz;

    if(sc->lotag != 23)
    {
        if(sprite[i].pal == 1)
            squishme = floorceildist < (32<<8) && (sc->lotag&32768) == 0;
        else
            squishme = floorceildist < (12<<8); // && (sc->lotag&32768) == 0;
    }
    else squishme = 0;

    if( squishme )
    {
        FTA(10,&ps[p]);

        if(badguy(&sprite[i])) sprite[i].xvel = 0;

        if(sprite[i].pal == 1)
        {
            hittype[i].picnum = SHOTSPARK1;
            hittype[i].extra = 1;
            return 0;
        }

        return 1;
    }
    return 0;
}

void hitradius( short i, int  r, int  hp1, int  hp2, int  hp3, int  hp4 )
{
    spritetype *s,*sj;
    walltype *wal;
    int d, q, x1, y1;
    int sectcnt, sectend, dasect, startwall, endwall, nextsect;
    short j,k,p,x,nextj,sect;
    unsigned char statlist[] = {0,1,6,10,12,2,5};
    short *tempshort = (short *)tempbuf;

    s = &sprite[i];

    if(s->picnum == RPG && s->xrepeat < 11) goto SKIPWALLCHECK;

    if(s->picnum != SHRINKSPARK)
    {
        tempshort[0] = s->sectnum;
        dasect = s->sectnum;
        sectcnt = 0; sectend = 1;

        do
        {
            dasect = tempshort[sectcnt++];
            if(((sector[dasect].ceilingz-s->z)>>8) < r)
            {
               d = klabs(wall[sector[dasect].wallptr].x-s->x)+klabs(wall[sector[dasect].wallptr].y-s->y);
               if(d < r)
                    checkhitceiling(dasect);
               else
               {
                    d = klabs(wall[wall[wall[sector[dasect].wallptr].point2].point2].x-s->x)+klabs(wall[wall[wall[sector[dasect].wallptr].point2].point2].y-s->y);
                    if(d < r)
                        checkhitceiling(dasect);
               }
           }

           startwall = sector[dasect].wallptr;
           endwall = startwall+sector[dasect].wallnum;
           for(x=startwall,wal=&wall[startwall];x<endwall;x++,wal++)
               if( ( klabs(wal->x-s->x)+klabs(wal->y-s->y) ) < r)
           {
               nextsect = wal->nextsector;
               if (nextsect >= 0)
               {
                   for(dasect=sectend-1;dasect>=0;dasect--)
                       if (tempshort[dasect] == nextsect) break;
                   if (dasect < 0) tempshort[sectend++] = nextsect;
               }
               x1 = (((wal->x+wall[wal->point2].x)>>1)+s->x)>>1;
               y1 = (((wal->y+wall[wal->point2].y)>>1)+s->y)>>1;
               updatesector(x1,y1,&sect);
               if( sect >= 0 && cansee(x1,y1,s->z,sect,s->x,s->y,s->z,s->sectnum ) )
                   checkhitwall(i,x,wal->x,wal->y,s->z,s->picnum);
           }
        }
        while (sectcnt < sectend);
    }

    SKIPWALLCHECK:

    q = -(16<<8)+(TRAND&((32<<8)-1));

    for(x = 0;x<7;x++)
    {
        j = headspritestat[statlist[x]];
        while(j >= 0)
        {
            nextj = nextspritestat[j];
            sj = &sprite[j];

            if( x == 0 || x >= 5 || AFLAMABLE(sj->picnum) )
            {
                if( s->picnum != SHRINKSPARK || (sj->cstat&257) )
                    if( dist( s, sj ) < r )
                    {
                        if( badguy(sj) && !cansee( sj->x, sj->y,sj->z+q, sj->sectnum, s->x, s->y, s->z+q, s->sectnum) )
                            goto BOLT;
                        checkhitsprite( j, i );
                    }
            }
            else if( sj->extra >= 0 && sj != s && ( sj->picnum == TRIPBOMB || badguy(sj) || sj->picnum == QUEBALL || sj->picnum == STRIPEBALL || (sj->cstat&257) || sj->picnum == DUKELYINGDEAD ) )
            {
                if( s->picnum == SHRINKSPARK && sj->picnum != SHARK && ( j == s->owner || sj->xrepeat < 24 ) )
                {
                    j = nextj;
                    continue;
                }
                if( s->picnum == MORTER && j == s->owner)
                {
                    j = nextj;
                    continue;
                }

                if(sj->picnum == APLAYER) sj->z -= PHEIGHT;
                d = dist( s, sj );
                if(sj->picnum == APLAYER) sj->z += PHEIGHT;

                if ( d < r && cansee( sj->x, sj->y, sj->z-(8<<8), sj->sectnum, s->x, s->y, s->z-(12<<8), s->sectnum) )
                {
                    hittype[j].ang = getangle(sj->x-s->x,sj->y-s->y);

                    if ( s->picnum == RPG && sj->extra > 0)
                        hittype[j].picnum = RPG;
                    else
                    {
                        if( s->picnum == SHRINKSPARK )
                            hittype[j].picnum = SHRINKSPARK;
                        else hittype[j].picnum = RADIUSEXPLOSION;
                    }

                    if(s->picnum != SHRINKSPARK)
                    {
                        if ( d < r/3 )
                        {
                            if(hp4 == hp3) hp4++;
                            hittype[j].extra = hp3 + (TRAND%(hp4-hp3));
                        }
                        else if ( d < 2*r/3 )
                        {
                            if(hp3 == hp2) hp3++;
                            hittype[j].extra = hp2 + (TRAND%(hp3-hp2));
                        }
                        else if ( d < r )
                        {
                            if(hp2 == hp1) hp2++;
                            hittype[j].extra = hp1 + (TRAND%(hp2-hp1));
                        }

                        if( sprite[j].picnum != TANK && sprite[j].picnum != ROTATEGUN && sprite[j].picnum != RECON && sprite[j].picnum != BOSS1 && sprite[j].picnum != BOSS2 && sprite[j].picnum != BOSS3 && sprite[j].picnum != BOSS4 )
                        {
                            if(sj->xvel < 0) sj->xvel = 0;
                            sj->xvel += (s->extra<<2);
                        }

                        if( sj->picnum == PODFEM1 || sj->picnum == FEM1 ||
                            sj->picnum == FEM2 || sj->picnum == FEM3 ||
                            sj->picnum == FEM4 || sj->picnum == FEM5 ||
                            sj->picnum == FEM6 || sj->picnum == FEM7 ||
                            sj->picnum == FEM8 || sj->picnum == FEM9 ||
                            sj->picnum == FEM10 || sj->picnum == STATUE ||
                            sj->picnum == STATUEFLASH || sj->picnum == SPACEMARINE || sj->picnum == QUEBALL || sj->picnum == STRIPEBALL)
                                checkhitsprite( j, i );
                    }
                    else if(s->extra == 0) hittype[j].extra = 0;

                    if ( sj->picnum != RADIUSEXPLOSION &&
                        s->owner >= 0 && sprite[s->owner].statnum < MAXSTATUS )
                    {
                        if(sj->picnum == APLAYER)
                        {
                            p = sj->yvel;
                            if(ps[p].newowner >= 0)
                            {
                                ps[p].newowner = -1;
                                ps[p].posx = ps[p].oposx;
                                ps[p].posy = ps[p].oposy;
                                ps[p].posz = ps[p].oposz;
                                ps[p].ang = ps[p].oang;
                                updatesector(ps[p].posx,ps[p].posy,&ps[p].cursectnum);
                                setpal(&ps[p]);

                                k = headspritestat[1];
                                while(k >= 0)
                                {
                                    if(sprite[k].picnum==CAMERA1)
                                        sprite[k].yvel = 0;
                                    k = nextspritestat[k];
                                }
                            }
                        }
                        hittype[j].owner = s->owner;
                    }
                }
            }
            BOLT:
            j = nextj;
        }
    }
}


int movesprite(short spritenum, int xchange, int ychange, int zchange, unsigned int cliptype)
{
    int daz,h, oldx, oldy;
    short retval, dasectnum, cd;
    char bg;

    bg = badguy(&sprite[spritenum]);

    if(sprite[spritenum].statnum == 5 || (bg && sprite[spritenum].xrepeat < 4 ) )
    {
        sprite[spritenum].x += (xchange*TICSPERFRAME)>>2;
        sprite[spritenum].y += (ychange*TICSPERFRAME)>>2;
        sprite[spritenum].z += (zchange*TICSPERFRAME)>>2;
        if(bg)
            setsprite(spritenum,sprite[spritenum].x,sprite[spritenum].y,sprite[spritenum].z);
        return 0;
    }

    dasectnum = sprite[spritenum].sectnum;

    daz = sprite[spritenum].z;
    h = ((tilesizy[sprite[spritenum].picnum]*sprite[spritenum].yrepeat)<<1);
    daz -= h;

    if( bg )
    {
        oldx = sprite[spritenum].x;
        oldy = sprite[spritenum].y;

        if( sprite[spritenum].xrepeat > 60 )
            retval = clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),1024L,(4<<8),(4<<8),cliptype);
        else
        {
            if(sprite[spritenum].picnum == LIZMAN)
                cd = 292L;
            else if( (actortype[sprite[spritenum].picnum]&3) )
                cd = sprite[spritenum].clipdist<<2;
            else
                cd = 192L;

            retval = clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),cd,(4<<8),(4<<8),cliptype);
        }

        if( dasectnum < 0 || ( dasectnum >= 0 &&
            ( ( hittype[spritenum].actorstayput >= 0 && hittype[spritenum].actorstayput != dasectnum ) ||
              ( ( sprite[spritenum].picnum == BOSS2 ) && sprite[spritenum].pal == 0 && sector[dasectnum].lotag != 3 ) ||
              ( ( sprite[spritenum].picnum == BOSS1 || sprite[spritenum].picnum == BOSS2 ) && sector[dasectnum].lotag == 1 ) ||
              ( sector[dasectnum].lotag == 1 && ( sprite[spritenum].picnum == LIZMAN || ( sprite[spritenum].picnum == LIZTROOP && sprite[spritenum].zvel == 0 ) ) )
            ) )
          )
        {
                sprite[spritenum].x = oldx;
                sprite[spritenum].y = oldy;
                if(sector[dasectnum].lotag == 1 && sprite[spritenum].picnum == LIZMAN)
                    sprite[spritenum].ang = (TRAND&2047);
                else if( (hittype[spritenum].temp_data[0]&3) == 1 && sprite[spritenum].picnum != COMMANDER )
                    sprite[spritenum].ang = (TRAND&2047);
                setsprite(spritenum,oldx,oldy,sprite[spritenum].z);
                if(dasectnum < 0) dasectnum = 0;
                return (16384+dasectnum);
        }
        if( (retval&49152) >= 32768 && (hittype[spritenum].cgg==0) ) sprite[spritenum].ang += 768;
    }
    else
    {
        if(sprite[spritenum].statnum == 4)
            retval =
                clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),8L,(4<<8),(4<<8),cliptype);
        else
            retval =
                clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),(int)(sprite[spritenum].clipdist<<2),(4<<8),(4<<8),cliptype);
    }

    if( dasectnum >= 0)
        if ( (dasectnum != sprite[spritenum].sectnum) )
            changespritesect(spritenum,dasectnum);
    daz = sprite[spritenum].z + ((zchange*TICSPERFRAME)>>3);
    if ((daz > hittype[spritenum].ceilingz) && (daz <= hittype[spritenum].floorz))
        sprite[spritenum].z = daz;
    else
        if (retval == 0)
            return(16384+dasectnum);

    return(retval);
}


short ssp(short i,unsigned int cliptype) //The set sprite function
{
    spritetype *s;
    int movetype;

    s = &sprite[i];

    movetype = movesprite(i,
        (s->xvel*(sintable[(s->ang+512)&2047]))>>14,
        (s->xvel*(sintable[s->ang&2047]))>>14,s->zvel,
        cliptype);

    return (movetype==0);
}

void insertspriteq(short i)
{
    if(spriteqamount > 0)
    {
        if(spriteq[spriteqloc] >= 0)
            sprite[spriteq[spriteqloc]].xrepeat = 0;
        spriteq[spriteqloc] = i;
        spriteqloc = (spriteqloc+1)%spriteqamount;
    }
    else sprite[i].xrepeat = sprite[i].yrepeat = 0;
}

void lotsofmoney(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),MONEY,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}

void lotsofmail(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),MAIL,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}

void lotsofpaper(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),PAPER,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}



void guts(spritetype *s,short gtype, short n, short p)
{
    int gutz,floorz;
    short i,a,j;
    char sx,sy;
    signed char pal;

    if(badguy(s) && s->xrepeat < 16)
        sx = sy = 8;
    else sx = sy = 32;

    gutz = s->z-(8<<8);
    floorz = getflorzofslope(s->sectnum,s->x,s->y);

    if( gutz > ( floorz-(8<<8) ) )
        gutz = floorz-(8<<8);

    if(s->picnum == COMMANDER)
        gutz -= (24<<8);

    if( badguy(s) && s->pal == 6)
        pal = 6;
    else pal = 0;

    for(j=0;j<n;j++)
    {
        a = TRAND&2047;
        i = EGS(s->sectnum,s->x+(TRAND&255)-128,s->y+(TRAND&255)-128,gutz-(TRAND&8191),gtype,-32,sx,sy,a,48+(TRAND&31),-512-(TRAND&2047),ps[p].i,5);
        if(PN == JIBS2)
        {
            sprite[i].xrepeat >>= 2;
            sprite[i].yrepeat >>= 2;
        }
        if(pal == 6)
            sprite[i].pal = 6;
    }
}

void gutsdir(spritetype *s,short gtype, short n, short p)
{
    int gutz,floorz;
    short a,j;
    char sx,sy;

    if(badguy(s) && s->xrepeat < 16)
        sx = sy = 8;
    else sx = sy = 32;

    gutz = s->z-(8<<8);
    floorz = getflorzofslope(s->sectnum,s->x,s->y);

    if( gutz > ( floorz-(8<<8) ) )
        gutz = floorz-(8<<8);

    if(s->picnum == COMMANDER)
        gutz -= (24<<8);

    for(j=0;j<n;j++)
    {
        a = TRAND&2047;
        EGS(s->sectnum,s->x,s->y,gutz,gtype,-32,sx,sy,a,256+(TRAND&127),-512-(TRAND&2047),ps[p].i,5);
    }
}

void setsectinterpolate(short i)
{
    int j, k, startwall,endwall;

    startwall = sector[SECT].wallptr;
    endwall = startwall+sector[SECT].wallnum;

    for(j=startwall;j<endwall;j++)
    {
        setinterpolation(&wall[j].x);
        setinterpolation(&wall[j].y);
        k = wall[j].nextwall;
        if(k >= 0)
        {
            setinterpolation(&wall[k].x);
            setinterpolation(&wall[k].y);
            k = wall[k].point2;
            setinterpolation(&wall[k].x);
            setinterpolation(&wall[k].y);
        }
    }
}

void clearsectinterpolate(short i)
{
    short j,startwall,endwall;

    startwall = sector[SECT].wallptr;
    endwall = startwall+sector[SECT].wallnum;
    for(j=startwall;j<endwall;j++)
    {
        stopinterpolation(&wall[j].x);
        stopinterpolation(&wall[j].y);
        if(wall[j].nextwall >= 0)
        {
            stopinterpolation(&wall[wall[j].nextwall].x);
            stopinterpolation(&wall[wall[j].nextwall].y);
        }
    }
}

void ms(short i)
{
    //T1,T2 and T3 are used for all the sector moving stuff!!!

    short startwall,endwall,x;
    int tx,ty,j,k;
    spritetype *s;

    s = &sprite[i];

    s->x += (s->xvel*(sintable[(s->ang+512)&2047]))>>14;
    s->y += (s->xvel*(sintable[s->ang&2047]))>>14;

    j = T2;
    k = T3;

    startwall = sector[s->sectnum].wallptr;
    endwall = startwall+sector[s->sectnum].wallnum;
    for(x=startwall;x<endwall;x++)
    {
        rotatepoint(
            0,0,
            msx[j],msy[j],
            k&2047,&tx,&ty);

        dragpoint(x,s->x+tx,s->y+ty);

        j++;
    }
}

void movefta(void)
{
    int x, px, py, sx, sy;
    short i, j, p, psect, ssect, nexti;
    spritetype *s;

    i = headspritestat[2];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        p = findplayer(s,&x);

        ssect = psect = s->sectnum;

        if(sprite[ps[p].i].extra > 0 )
        {
            if( x < 30000 )
            {
                hittype[i].timetosleep++;
                if( hittype[i].timetosleep >= (x>>8) )
                {
                    int rz, orz;
                    if(badguy(s))
                    {
                        px = ps[p].oposx+64-(TRAND&127);
                        py = ps[p].oposy+64-(TRAND&127);
                        updatesector(px,py,&psect);
                        if(psect == -1)
                        {
                            i = nexti;
                            continue;
                        }
                        sx = s->x+64-(TRAND&127);
                        sy = s->y+64-(TRAND&127);
                        updatesector(px,py,&ssect);
                        if(ssect == -1)
                        {
                            i = nexti;
                            continue;
                        }
                        rz = TRAND;
                        orz = TRAND;
                        j = cansee(sx,sy,s->z-(rz%(52<<8)),s->sectnum,px,py,ps[p].oposz-(orz%(32<<8)),ps[p].cursectnum);
                    }
                    else
                    {
                        rz = TRAND;
                        orz = TRAND;
                        j = cansee(s->x,s->y,s->z-((rz&31)<<8),s->sectnum,ps[p].oposx,ps[p].oposy,ps[p].oposz-((orz&31)<<8),ps[p].cursectnum);
                    }

       //             j = 1;

                    if(j) switch(s->picnum)
                    {
                        case RUBBERCAN:
                        case EXPLODINGBARREL:
                        case WOODENHORSE:
                        case HORSEONSIDE:
                        case CANWITHSOMETHING:
                        case CANWITHSOMETHING2:
                        case CANWITHSOMETHING3:
                        case CANWITHSOMETHING4:
                        case FIREBARREL:
                        case FIREVASE:
                        case NUKEBARREL:
                        case NUKEBARRELDENTED:
                        case NUKEBARRELLEAKED:
                        case TRIPBOMB:
                            if (sector[s->sectnum].ceilingstat&1)
                                s->shade = sector[s->sectnum].ceilingshade;
                            else s->shade = sector[s->sectnum].floorshade;

                            hittype[i].timetosleep = 0;
                            changespritestat(i,6);
                            break;
                        default:
                            hittype[i].timetosleep = 0;
                            check_fta_sounds(i);
                            changespritestat(i,1);
                            break;
                    }
                    else hittype[i].timetosleep = 0;
                }
            }
            if( badguy( s ) )
            {
                if (sector[s->sectnum].ceilingstat&1)
                    s->shade = sector[s->sectnum].ceilingshade;
                else s->shade = sector[s->sectnum].floorshade;
            }
        }
        i = nexti;
    }
}

short ifhitsectors(short sectnum)
{
    short i;

    i = headspritestat[5];
    while(i >= 0)
    {
        if( PN == EXPLOSION2 && sectnum == SECT )
            return i;
        i = nextspritestat[i];
    }
    return -1;
}

short ifhitbyweapon(short sn)
{
    short j, p;
    spritetype *npc;

    if( hittype[sn].extra >= 0 )
    {
        if(sprite[sn].extra >= 0 )
        {
            npc = &sprite[sn];

            if(npc->picnum == APLAYER)
            {
                if(ud.god && hittype[sn].picnum != SHRINKSPARK ) return -1;

                p = npc->yvel;
                j = hittype[sn].owner;

                if( j >= 0 &&
                    sprite[j].picnum == APLAYER &&
                    ud.coop == 1 &&
                    ud.ffire == 0 )
                        return -1;

                npc->extra -= hittype[sn].extra;

                if(j >= 0)
                {
                    if(npc->extra <= 0 && hittype[sn].picnum != FREEZEBLAST)
                    {
                        npc->extra = 0;

                        ps[p].wackedbyactor = j;

                        if( sprite[hittype[sn].owner].picnum == APLAYER && p != sprite[hittype[sn].owner].yvel )
                            ps[p].frag_ps = sprite[j].yvel;

                        hittype[sn].owner = ps[p].i;
                    }
                }

                switch(hittype[sn].picnum)
                {
                    case RADIUSEXPLOSION:
                    case RPG:
                    case HYDRENT:
                    case HEAVYHBOMB:
                    case SEENINE:
                    case OOZFILTER:
                    case EXPLODINGBARREL:
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<2;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<2;
                        break;
                    default:
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<1;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<1;
                        break;
                }
            }
            else
            {
                if(hittype[sn].extra == 0 )
                    if( hittype[sn].picnum == SHRINKSPARK && npc->xrepeat < 24 )
                        return -1;

                npc->extra -= hittype[sn].extra;
                if(npc->picnum != RECON && npc->owner >= 0 && sprite[npc->owner].statnum < MAXSTATUS )
                    npc->owner = hittype[sn].owner;
            }

            hittype[sn].extra = -1;
            return hittype[sn].picnum;
        }
    }

    hittype[sn].extra = -1;
    return -1;
}

void movecyclers(void)
{
    short q, j, x, t, s, *c;
    walltype *wal;
    unsigned char cshade;

    for(q=numcyclers-1;q>=0;q--)
    {

        c = &cyclers[q][0];
        s = c[0];

        t = c[3];
        j = t+(sintable[c[1]&2047]>>10);
        cshade = c[2];

        if( j < cshade ) j = cshade;
        else if( j > t )  j = t;

        c[1] += sector[s].extra;
        if(c[5])
        {
            wal = &wall[sector[s].wallptr];
            for(x = sector[s].wallnum;x>0;x--,wal++)
                if( wal->hitag != 1 )
            {
                wal->shade = j;

                if( (wal->cstat&2) && wal->nextwall >= 0)
                    wall[wal->nextwall].shade = j;

            }
            sector[s].floorshade = sector[s].ceilingshade = j;
        }
    }
}

void movedummyplayers(void)
{
    short i, p, nexti;

    i = headspritestat[13];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        p = sprite[OW].yvel;

        if( ps[p].on_crane >= 0 || sector[ps[p].cursectnum].lotag != 1 || sprite[ps[p].i].extra <= 0 )
        {
            ps[p].dummyplayersprite = -1;
            KILLIT(i);
        }
        else
        {
            if(ps[p].on_ground && ps[p].on_warping_sector == 1 && sector[ps[p].cursectnum].lotag == 1 )
            {
                CS = 257;
                SZ = sector[SECT].ceilingz+(27<<8);
                SA = ps[p].ang;
                if(T1 == 8)
                    T1 = 0;
                else T1++;
            }
            else
            {
                if(sector[SECT].lotag != 2) SZ = sector[SECT].floorz;
                CS = (short) 32768;
            }
        }

        SX += (ps[p].posx-ps[p].oposx);
        SY += (ps[p].posy-ps[p].oposy);
        setsprite(i,SX,SY,SZ);

        BOLT:

        i = nexti;
    }
}


short otherp;
void moveplayers(void) //Players
{
    short i , nexti;
    int otherx;
    spritetype *s;
    struct player_struct *p;

    i = headspritestat[10];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        p = &ps[s->yvel];
        if(s->owner >= 0)
        {
            if(p->newowner >= 0 ) //Looking thru the camera
            {
                s->x = p->oposx;
                s->y = p->oposy;
                hittype[i].bposz = s->z = p->oposz+PHEIGHT;
                s->ang = p->oang;
                setsprite(i,s->x,s->y,s->z);
            }
            else
            {
                if(ud.multimode > 1)
                    otherp = findotherplayer(s->yvel,&otherx);
                else
                {
                    otherp = s->yvel;
                    otherx = 0;
                }

                execute(i,s->yvel,otherx);

                if(ud.multimode > 1)
                    if( sprite[ps[otherp].i].extra > 0 )
                {
                    if( s->yrepeat > 32 && sprite[ps[otherp].i].yrepeat < 32)
                    {
                        if( otherx < 1400 && p->knee_incs == 0 )
                        {
                            p->knee_incs = 1;
                            p->weapon_pos = -1;
                            p->actorsqu = ps[otherp].i;
                        }
                    }
                }
                if(ud.god)
                {
                    s->extra = max_player_health;
                    s->cstat = 257;
                    p->jetpack_amount =     1599;
                }


                if( s->extra > 0 )
                {
                    hittype[i].owner = i;

                    if(ud.god == 0)
                        if( ceilingspace(s->sectnum) || floorspace(s->sectnum) )
                            quickkill(p);
                }
                else
                {

                    p->posx = s->x;
                    p->posy = s->y;
                    p->posz = s->z-(20<<8);

                    p->newowner = -1;

                    if( p->wackedbyactor >= 0 && sprite[p->wackedbyactor].statnum < MAXSTATUS )
                    {
                        p->ang += getincangle(p->ang,getangle(sprite[p->wackedbyactor].x-p->posx,sprite[p->wackedbyactor].y-p->posy))>>1;
                        p->ang &= 2047;
                    }

                }
                s->ang = p->ang;
            }
        }
        else
        {
            if(p->holoduke_on == -1)
                KILLIT(i);

            hittype[i].bposx = s->x;
            hittype[i].bposy = s->y;
            hittype[i].bposz = s->z;

            s->cstat = 0;

            if(s->xrepeat < 42)
            {
                s->xrepeat += 4;
                s->cstat |= 2;
            }
            else s->xrepeat = 42;
            if(s->yrepeat < 36)
                s->yrepeat += 4;
            else
            {
                s->yrepeat = 36;
                if(sector[s->sectnum].lotag != 2)
                    makeitfall(i);
                if(s->zvel == 0 && sector[s->sectnum].lotag == 1)
                    s->z += (32<<8);
            }

            if(s->extra < 8)
            {
                s->xvel = 128;
                s->ang = p->ang;
                s->extra++;
                //IFMOVING;     // JBF 20040825: is really "if (ssp(i,CLIPMASK0)) ;" which is probably
        ssp(i,CLIPMASK0);   // not the safest of ideas because a zealous optimiser probably sees
                    // it as redundant, so I'll call the "ssp(i,CLIPMASK0)" explicitly.
            }
            else
            {
                s->ang = 2047-p->ang;
                setsprite(i,s->x,s->y,s->z);
            }
        }

        if (sector[s->sectnum].ceilingstat&1)
            s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
        else
            s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

        BOLT:
        i = nexti;
    }
}


void movefx(void)
{
    short i, j, nexti, p;
    int x, ht;
    spritetype *s;

    i = headspritestat[11];
    while(i >= 0)
    {
        s = &sprite[i];

        nexti = nextspritestat[i];

        switch(s->picnum)
        {
            case RESPAWN:
                if(sprite[i].extra == 66)
                {
                    j = spawn(i,SHT);
//                    sprite[j].pal = sprite[i].pal;
                    KILLIT(i);
                }
                else if(sprite[i].extra > (66-13))
                    sprite[i].extra++;
                break;

            case MUSICANDSFX:

                ht = s->hitag;

                if(T2 != SoundToggle)
                {
                    T2 = SoundToggle;
                    T1 = 0;
                }

                if(s->lotag >= 1000 && s->lotag < 2000)
                {
                    x = ldist(&sprite[ps[screenpeek].i],s);
                    if( x < ht && T1 == 0 )
                    {
                        FX_SetReverb( s->lotag - 1000 );
                        T1 = 1;
                    }
                    if( x >= ht && T1 == 1 )
                    {
                        FX_SetReverb(0);
                        FX_SetReverbDelay(0);
                        T1 = 0;
                    }
                }
                else if(s->lotag < 999 && (unsigned)sector[s->sectnum].lotag < 9 && AmbienceToggle && sector[SECT].floorz != sector[SECT].ceilingz)
                {
                    if( (soundm[s->lotag]&2) )
                    {
                        x = dist(&sprite[ps[screenpeek].i],s);
                        if( x < ht && T1 == 0 && FX_VoiceAvailable(soundpr[s->lotag]-1) )
                        {
                            if(numenvsnds == NumVoices)
                            {
                                j = headspritestat[11];
                                while(j >= 0)
                                {
                                    if( PN == MUSICANDSFX && j != i && sprite[j].lotag < 999 && hittype[j].temp_data[0] == 1 && dist(&sprite[j],&sprite[ps[screenpeek].i]) > x )
                                    {
                                        stopenvsound(sprite[j].lotag,j);
                                        break;
                                    }
                                    j = nextspritestat[j];
                                }
                                if(j == -1) goto BOLT;
                            }
                            spritesound(s->lotag,i);
                            T1 = 1;
                        }
                        if( x >= ht && T1 == 1 )
                        {
                            T1 = 0;
                            stopenvsound(s->lotag,i);
                        }
                    }
                    if( (soundm[s->lotag]&16) )
                    {
                        if(T5 > 0) T5--;
                        else for(p=connecthead;p>=0;p=connectpoint2[p])
                            if( p == myconnectindex && ps[p].cursectnum == s->sectnum )
                        {
                            j = s->lotag+((unsigned)global_random%(s->hitag+1));
                            sound(j);
                            T5 =  26*40 + (global_random%(26*40));
                        }
                    }
                }
                break;
        }
        BOLT:
        i = nexti;
    }
}



void movefallers(void)
{
    short i, nexti, sect, j;
    spritetype *s;
    int x;

    i = headspritestat[12];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        sect = s->sectnum;

        if( T1 == 0 )
        {
            s->z -= (16<<8);
            T2 = s->ang;
            x = s->extra;
            IFHIT
            {
                if( j == FIREEXT || j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
                {
                    if(s->extra <= 0)
                    {
                        T1 = 1;
                        j = headspritestat[12];
                        while(j >= 0)
                        {
                            if(sprite[j].hitag == SHT)
                            {
                                hittype[j].temp_data[0] = 1;
                                sprite[j].cstat &= (65535-64);
                                if(sprite[j].picnum == CEILINGSTEAM || sprite[j].picnum == STEAM)
                                    sprite[j].cstat |= 32768;
                            }
                            j = nextspritestat[j];
                        }
                    }
                }
                else
                {
                    hittype[i].extra = 0;
                    s->extra = x;
                }
            }
            s->ang = T2;
            s->z += (16<<8);
        }
        else if(T1 == 1)
        {
            if(s->lotag > 0)
            {
                s->lotag-=3;
                if(s->lotag <= 0)
                {
                    s->xvel = (32+(TRAND&63));
                    s->zvel = -(1024+(TRAND&1023));
                }
            }
            else
            {
                if( s->xvel > 0)
                {
                    s->xvel -= 8;
                    ssp(i,CLIPMASK0);
                }

                if( floorspace(s->sectnum) ) x = 0;
                else
                {
                    if(ceilingspace(s->sectnum))
                        x = gc/6;
                    else
                        x = gc;
                }

                if( s->z < (sector[sect].floorz-FOURSLEIGHT) )
                {
                    s->zvel += x;
                    if(s->zvel > 6144)
                        s->zvel = 6144;
                    s->z += s->zvel;
                }
                if( (sector[sect].floorz-s->z) < (16<<8) )
                {
                    j = 1+(TRAND&7);
                    for(x=0;x<j;x++) RANDOMSCRAP;
                    KILLIT(i);
                }
            }
        }

        BOLT:
        i = nexti;
    }
}

void movestandables(void)
{
    short i, j, k, m, nexti, nextj, p=0, sect;
    int l=0, x, *t;
    spritetype *s;

    i = headspritestat[6];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        t = &hittype[i].temp_data[0];
        s = &sprite[i];
        sect = s->sectnum;

        if( sect < 0 ) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        IFWITHIN(CRANE,CRANE+3)
        {
            //t[0] = state
            //t[1] = checking sector number

            if(s->xvel) getglobalz(i);

            if( t[0] == 0 ) //Waiting to check the sector
            {
                j = headspritesect[t[1]];
                while(j>=0)
                {
                    nextj = nextspritesect[j];
                    switch( sprite[j].statnum )
                    {
                        case 1:
                        case 2:
                        case 6:
                        case 10:
                            s->ang = getangle(msx[t[4]+1]-s->x,msy[t[4]+1]-s->y);
                            setsprite(j,msx[t[4]+1],msy[t[4]+1],sprite[j].z);
                            t[0]++;
                            goto BOLT;
                    }
                    j = nextj;
                }
            }

            else if(t[0]==1)
            {
                if( s->xvel < 184 )
                {
                    s->picnum = CRANE+1;
                    s->xvel += 8;
                }
                //IFMOVING; // JBF 20040825: see my rant above about this
        ssp(i,CLIPMASK0);
                if(sect == t[1])
                    t[0]++;
            }
            else if(t[0]==2 || t[0]==7)
            {
                s->z += (1024+512);

                if(t[0]==2)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                        if(s->picnum > CRANE) s->picnum--;

                    if( (sector[sect].floorz - s->z) < (4096+1024))
                        t[0]++;
                }
                if(t[0]==7)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                    {
                        if(s->picnum > CRANE) s->picnum--;
                        else
                        {
                            if(s->owner==-2)
                            {
                                spritesound(DUKE_GRUNT,ps[p].i);
                                p = findplayer(s,&x);
                                if(ps[p].on_crane == i)
                                    ps[p].on_crane = -1;
                            }
                            t[0]++;
                            s->owner = -1;
                        }
                    }
                }
            }
            else if(t[0]==3)
            {
                s->picnum++;
                if( s->picnum == (CRANE+2) )
                {
                    p = checkcursectnums(t[1]);
                    if(p >= 0 && ps[p].on_ground)
                    {
                        s->owner = -2;
                        ps[p].on_crane = i;
                        spritesound(DUKE_GRUNT,ps[p].i);
                        ps[p].ang = s->ang+1024;
                    }
                    else
                    {
                        j = headspritesect[t[1]];
                        while(j>=0)
                        {
                            switch( sprite[j].statnum )
                            {
                                case 1:
                                case 6:
                                    s->owner = j;
                                    break;
                            }
                            j = nextspritesect[j];
                        }
                    }

                    t[0]++;//Grabbed the sprite
                    t[2]=0;
                    goto BOLT;
                }
            }
            else if(t[0]==4) //Delay before going up
            {
                t[2]++;
                if(t[2] > 10)
                    t[0]++;
            }
            else if(t[0]==5 || t[0] == 8)
            {
                if(t[0]==8 && s->picnum < (CRANE+2))
                    if( (sector[sect].floorz-s->z) > 8192)
                        s->picnum++;

                if(s->z < msx[t[4]+2])
                {
                    t[0]++;
                    s->xvel = 0;
                }
                else
                    s->z -= (1024+512);
            }
            else if(t[0]==6)
            {
                if( s->xvel < 192 )
                    s->xvel += 8;
                s->ang = getangle(msx[t[4]]-s->x,msy[t[4]]-s->y);
                //IFMOVING; // JBF 20040825: see my rant above about this
        ssp(i,CLIPMASK0);
                if( ((s->x-msx[t[4]])*(s->x-msx[t[4]])+(s->y-msy[t[4]])*(s->y-msy[t[4]]) ) < (128*128) )
                    t[0]++;
            }

            else if(t[0]==9)
                t[0] = 0;

            setsprite(msy[t[4]+2],s->x,s->y,s->z-(34<<8));

            if(s->owner != -1)
            {
                p = findplayer(s,&x);

                IFHIT
                {
                    if(s->owner == -2)
                        if(ps[p].on_crane == i)
                            ps[p].on_crane = -1;
                    s->owner = -1;
                    s->picnum = CRANE;
                    goto BOLT;
                }

                if(s->owner >= 0)
                {
                    setsprite(s->owner,s->x,s->y,s->z);

                    hittype[s->owner].bposx = s->x;
                    hittype[s->owner].bposy = s->y;
                    hittype[s->owner].bposz = s->z;

                    s->zvel = 0;
                }
                else if(s->owner == -2)
                {
                    ps[p].oposx = ps[p].posx = s->x-(sintable[(ps[p].ang+512)&2047]>>6);
                    ps[p].oposy = ps[p].posy = s->y-(sintable[ps[p].ang&2047]>>6);
                    ps[p].oposz = ps[p].posz = s->z+(2<<8);
                    setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz);
                    ps[p].cursectnum = sprite[ps[p].i].sectnum;
                }
            }

            goto BOLT;
        }

        IFWITHIN(WATERFOUNTAIN,WATERFOUNTAIN+3)
        {
            if(t[0] > 0)
            {
                if( t[0] < 20 )
                {
                    t[0]++;

                    s->picnum++;

                    if( s->picnum == ( WATERFOUNTAIN+3 ) )
                        s->picnum = WATERFOUNTAIN+1;
                }
                else
                {
                    p = findplayer(s,&x);

                    if(x > 512)
                    {
                        t[0] = 0;
                        s->picnum = WATERFOUNTAIN;
                    }
                    else t[0] = 1;
                }
            }
            goto BOLT;
        }

        if( AFLAMABLE(s->picnum) )
        {
            if(T1 == 1)
            {
                T2++;
                if( (T2&3) > 0) goto BOLT;

                if( s->picnum == TIRE && T2 == 32 )
                {
                    s->cstat = 0;
                    j = spawn(i,BLOODPOOL);
                    sprite[j].shade = 127;
                }
                else
                {
                    if(s->shade < 64) s->shade++;
                    else KILLIT(i);
                }

                j = s->xrepeat-(TRAND&7);
                if(j < 10)
                {
                    KILLIT(i);
                }

                s->xrepeat = j;

                j = s->yrepeat-(TRAND&7);
                if(j < 4) { KILLIT(i); }
                s->yrepeat = j;
            }
            if(s->picnum == BOX)
            {
                makeitfall(i);
                hittype[i].ceilingz = sector[s->sectnum].ceilingz;
            }
            goto BOLT;
        }

        if(s->picnum == TRIPBOMB)
        {
            if(T3 > 0)
            {
                T3--;
                if(T3 == 8)
                {
                    spritesound(LASERTRIP_EXPLODE,i);
                    for(j=0;j<5;j++) RANDOMSCRAP;
                    x = s->extra;
                    hitradius( i, tripbombblastradius, x>>2,x>>1,x-(x>>2),x);

                    j = spawn(i,EXPLOSION2);
                    sprite[j].ang = s->ang;
                    sprite[j].xvel = 348;
                    ssp(j,CLIPMASK0);

                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == LASERLINE && s->hitag == sprite[j].hitag)
                            sprite[j].xrepeat = sprite[j].yrepeat = 0;
                        j = nextspritestat[j];
                    }
                    KILLIT(i);
                }
                goto BOLT;
            }
            else
            {
                x = s->extra;
                s->extra = 1;
                l = s->ang;
                IFHIT { T3 = 16; }
                s->extra = x;
                s->ang = l;
            }

            if( T1 < 32 )
            {
                p = findplayer(s,&x);
                if( x > 768 ) T1++;
                else if(T1 > 16) T1++;
            }
            if( T1 == 32 )
            {
                l = s->ang;
                s->ang = T6;

                T4 = s->x;T5 = s->y;
                s->x += sintable[(T6+512)&2047]>>9;
                s->y += sintable[(T6)&2047]>>9;
                s->z -= (3<<8);
                setsprite(i,s->x,s->y,s->z);

                x = hitasprite(i,&m);

                hittype[i].lastvx = x;

                s->ang = l;

                k = 0;

                while(x > 0)
                {
                    j = spawn(i,LASERLINE);
                    setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                    sprite[j].hitag = s->hitag;
                    hittype[j].temp_data[1] = sprite[j].z;

                    s->x += sintable[(T6+512)&2047]>>4;
                    s->y += sintable[(T6)&2047]>>4;

                    if( x < 1024 )
                    {
                        sprite[j].xrepeat = x>>5;
                        break;
                    }
                    x -= 1024;
                }

                T1++;
                s->x = T4;s->y = T5;
                s->z += (3<<8);
                setsprite(i,s->x,s->y,s->z);
                T4 = 0;
                if( m >= 0 )
                {
                    T3 = 13;
                    spritesound(LASERTRIP_ARMING,i);
                }
                else T3 = 0;
            }
            if(T1 == 33)
            {
                T2++;


                T4 = s->x;T5 = s->y;
                s->x += sintable[(T6+512)&2047]>>9;
                s->y += sintable[(T6)&2047]>>9;
                s->z -= (3<<8);
                setsprite(i,s->x,s->y,s->z);

                x = hitasprite(i,&m);

                s->x = T4;s->y = T5;
                s->z += (3<<8);
                setsprite(i,s->x,s->y,s->z);

                if( hittype[i].lastvx != x )
                {
                    T3 = 13;
                    spritesound(LASERTRIP_ARMING,i);
                }
            }
            goto BOLT;
        }


        if( s->picnum >= CRACK1 && s->picnum <= CRACK4 )
        {
            if(s->hitag > 0)
            {
                t[0] = s->cstat;
                t[1] = s->ang;
                j = ifhitbyweapon(i);
                if(j == FIREEXT || j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE ) )
                            if(sprite[j].shade != -32)
                                sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }

                    goto DETONATE;
                }
                else
                {
                    s->cstat = t[0];
                    s->ang = t[1];
                    s->extra = 0;
                }
            }
            goto BOLT;
        }

        if( s->picnum == FIREEXT )
        {
            j = ifhitbyweapon(i);
            if( j == -1 ) goto BOLT;

            for(k=0;k<16;k++)
            {
                j = EGS(SECT,SX,SY,SZ-(TRAND%(48<<8)),SCRAP3+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(sprite[i].zvel>>2),i,5);
                sprite[j].pal = 2;
            }

            spawn(i,EXPLOSION2);
            spritesound(PIPEBOMB_EXPLODE,i);
            spritesound(GLASS_HEAVYBREAK,i);

            if(s->hitag > 0)
            {
                j = headspritestat[6];
                while(j >= 0)
                {
                    if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE ) )
                        if(sprite[j].shade != -32)
                            sprite[j].shade = -32;
                    j = nextspritestat[j];
                }

                x = s->extra;
                spawn(i,EXPLOSION2);
                hitradius( i, pipebombblastradius,x>>2, x-(x>>1),x-(x>>2), x);
                spritesound(PIPEBOMB_EXPLODE,i);

                goto DETONATE;
            }
            else
            {
                hitradius(i,seenineblastradius,10,15,20,25);
                KILLIT(i);
            }
            goto BOLT;
        }

        if(s->picnum == OOZFILTER || s->picnum == SEENINE || s->picnum == SEENINEDEAD || s->picnum == (SEENINEDEAD+1) )
        {
            if(s->shade != -32 && s->shade != -33)
            {
                if(s->xrepeat)
                    j = (ifhitbyweapon(i) >= 0);
                else
                    j = 0;

                if( j || s->shade == -31 )
                {
                    if(j) s->lotag = 0;

                    t[3] = 1;

                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == SEENINE || sprite[j].picnum == OOZFILTER ) )
                            sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }
                }
            }
            else
            {
                if(s->shade == -32)
                {
                    if(s->lotag > 0)
                    {
                        s->lotag-=3;
                        if(s->lotag <= 0) s->lotag = -99;
                    }
                    else
                        s->shade = -33;
                }
                else
                {
                    if( s->xrepeat > 0 )
                    {
                        T3++;
                        if(T3 == 3)
                        {
                            if( s->picnum == OOZFILTER )
                            {
                                T3 = 0;
                                goto DETONATE;
                            }
                            if( s->picnum != (SEENINEDEAD+1) )
                            {
                                T3 = 0;

                                if(s->picnum == SEENINEDEAD) s->picnum++;
                                else if(s->picnum == SEENINE)
                                    s->picnum = SEENINEDEAD;
                            }
                            else goto DETONATE;
                        }
                        goto BOLT;
                    }

                    DETONATE:

                    earthquaketime = 16;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( s->hitag == sprite[j].hitag )
                        {
                            if(sprite[j].lotag == 13)
                            {
                                if( hittype[j].temp_data[2] == 0 )
                                    hittype[j].temp_data[2] = 1;
                            }
                            else if(sprite[j].lotag == 8)
                                hittype[j].temp_data[4] = 1;
                            else if(sprite[j].lotag == 18)
                            {
                                if(hittype[j].temp_data[0] == 0)
                                    hittype[j].temp_data[0] = 1;
                            }
                            else if(sprite[j].lotag == 21)
                                hittype[j].temp_data[0] = 1;
                        }
                        j = nextspritestat[j];
                    }

                    s->z -= (32<<8);

                    if( ( t[3] == 1 && s->xrepeat ) || s->lotag == -99 )
                    {
                        x = s->extra;
                        spawn(i,EXPLOSION2);
                        hitradius( i,seenineblastradius,x>>2, x-(x>>1),x-(x>>2), x);
                        spritesound(PIPEBOMB_EXPLODE,i);
                    }

                    if(s->xrepeat)
                        for(x=0;x<8;x++) RANDOMSCRAP;

                    KILLIT(i);
                }
            }
            goto BOLT;
        }

        if(s->picnum == MASTERSWITCH)
        {
            if(s->yvel == 1)
                {
                    s->hitag--;
                    if(s->hitag <= 0)
                    {
                        operatesectors(sect,i);

                        j = headspritesect[sect];
                        while(j >= 0)
                        {
                            if(sprite[j].statnum == 3)
                            {
                                switch(sprite[j].lotag)
                                {
                                    case 2:
                                    case 21:
                                    case 31:
                                    case 32:
                                    case 36:
                                        hittype[j].temp_data[0] = 1;
                                        break;
                                    case 3:
                                        hittype[j].temp_data[4] = 1;
                                        break;
                                }
                            }
                            else if(sprite[j].statnum == 6)
                            {
                                switch(sprite[j].picnum)
                                {
                                    case SEENINE:
                                    case OOZFILTER:
                                        sprite[j].shade = -31;
                                        break;
                                }
                            }
                            j = nextspritesect[j];
                        }
                        KILLIT(i);
                    }
                }
                goto BOLT;
        }

        switch(s->picnum)
        {
            case VIEWSCREEN:
            case VIEWSCREEN2:

                if(s->xrepeat == 0) KILLIT(i);

                p = findplayer(s, &x);

                if( x < 2048 )
                {
                    if( SP == 1 )
                        camsprite = i;
                }
                else if( camsprite != -1 && T1 == 1)
                {
                    camsprite = -1;
                    T1 = 0;
                    //loadtile(s->picnum);
            //invalidatetile(s->picnum,-1,255);
            walock[TILE_VIEWSCR] = 199;
                }

                goto BOLT;

            case TRASH:

                if(s->xvel == 0) s->xvel = 1;
                IFMOVING
                {
                    makeitfall(i);
                    if(TRAND&1) s->zvel -= 256;
                    if( klabs(s->xvel) < 48 )
                        s->xvel += (TRAND&3);
                }
                else KILLIT(i);
                break;

            case SIDEBOLT1:
            case SIDEBOLT1+1:
            case SIDEBOLT1+2:
            case SIDEBOLT1+3:
                p = findplayer(s, &x);
                if( x > 20480 ) goto BOLT;

                CLEAR_THE_BOLT2:
                if(t[2])
                {
                    t[2]--;
                    goto BOLT;
                }
                if( (s->xrepeat|s->yrepeat) == 0 )
                {
                    s->xrepeat=t[0];
                    s->yrepeat=t[1];
                }
                if( (TRAND&8) == 0 )
                {
                    t[0]=s->xrepeat;
                    t[1]=s->yrepeat;
                    t[2] = global_random&4;
                    s->xrepeat=s->yrepeat=0;
                    goto CLEAR_THE_BOLT2;
                }
                s->picnum++;

                if(l&1) s->cstat ^= 2;

                if( (TRAND&1) && sector[sect].floorpicnum == HURTRAIL )
                    spritesound(SHORT_CIRCUIT,i);

                if(s->picnum == SIDEBOLT1+4) s->picnum = SIDEBOLT1;

                goto BOLT;

            case BOLT1:
            case BOLT1+1:
            case BOLT1+2:
            case BOLT1+3:
                p = findplayer(s, &x);
                if( x > 20480 ) goto BOLT;

                if( t[3] == 0 )
                    t[3]=sector[sect].floorshade;

                CLEAR_THE_BOLT:
                if(t[2])
                {
                    t[2]--;
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                    goto BOLT;
                }
                if( (s->xrepeat|s->yrepeat) == 0 )
                {
                    s->xrepeat=t[0];
                    s->yrepeat=t[1];
                }
                else if( (TRAND&8) == 0 )
                {
                    t[0]=s->xrepeat;
                    t[1]=s->yrepeat;
                    t[2] = global_random&4;
                    s->xrepeat=s->yrepeat=0;
                    goto CLEAR_THE_BOLT;
                }
                s->picnum++;

                l = global_random&7;
                s->xrepeat=l+8;

                if(l&1) s->cstat ^= 2;

                if( s->picnum == (BOLT1+1) && (TRAND&7) == 0 && sector[sect].floorpicnum == HURTRAIL )
                    spritesound(SHORT_CIRCUIT,i);

                if(s->picnum==BOLT1+4) s->picnum=BOLT1;

                if(s->picnum&1)
                {
                    sector[sect].floorshade = 0;
                    sector[sect].ceilingshade = 0;
                }
                else
                {
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                }
                goto BOLT;

            case WATERDRIP:

                if( t[1] )
                {
                    t[1]--;
                    if(t[1] == 0)
                        s->cstat &= 32767;
                }
                else
                {
                    makeitfall(i);
                    ssp(i,CLIPMASK0);
                    if(s->xvel > 0) s->xvel -= 2;

                    if(s->zvel == 0)
                    {
                        s->cstat |= 32768;

                        if(s->pal != 2 && s->hitag == 0)
                            spritesound(SOMETHING_DRIPPING,i);

                        if(sprite[s->owner].picnum != WATERDRIP)
                        {
                            KILLIT(i);
                        }
                        else
                        {
                            hittype[i].bposz = s->z = t[0];
                            t[1] = 48+(TRAND&31);
                        }
                    }
                }


                goto BOLT;

            case DOORSHOCK:
                j = klabs(sector[sect].ceilingz-sector[sect].floorz)>>9;
                s->yrepeat = j+4;
                s->xrepeat = 16;
                s->z = sector[sect].floorz;
                goto BOLT;

            case TOUCHPLATE:
                if( t[1] == 1 && s->hitag >= 0) //Move the sector floor
                {
                    x = sector[sect].floorz;

                    if(t[3] == 1)
                    {
                        if(x >= t[2])
                        {
                            sector[sect].floorz = x;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz += sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0) ps[p].posz += sector[sect].extra;
                        }
                    }
                    else
                    {
                        if(x <= s->z)
                        {
                            sector[sect].floorz = s->z;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz -= sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0)
                                ps[p].posz -= sector[sect].extra;
                        }
                    }
                    goto BOLT;
                }

                if(t[5] == 1) goto BOLT;

                p = checkcursectnums(sect);
                if( p >= 0 && ( ps[p].on_ground || s->ang == 512) )
                {
                    if( t[0] == 0 && !check_activator_motion(s->lotag) )
                    {
                        t[0] = 1;
                        t[1] = 1;
                        t[3] = !t[3];
                        operatemasterswitches(s->lotag);
                        operateactivators(s->lotag,p);
                        if(s->hitag > 0)
                        {
                            s->hitag--;
                            if(s->hitag == 0) t[5] = 1;
                        }
                    }
                }
                else t[0] = 0;

                if(t[1] == 1)
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(j != i && sprite[j].picnum == TOUCHPLATE && sprite[j].lotag == s->lotag)
                        {
                            hittype[j].temp_data[1] = 1;
                            hittype[j].temp_data[3] = t[3];
                        }
                        j = nextspritestat[j];
                    }
                }
                goto BOLT;

            case CANWITHSOMETHING:
            case CANWITHSOMETHING2:
            case CANWITHSOMETHING3:
            case CANWITHSOMETHING4:
                makeitfall(i);
                IFHIT
                {
                    spritesound(VENT_BUST,i);
                    for(j=0;j<10;j++)
                        RANDOMSCRAP;

                    if(s->lotag) spawn(i,s->lotag);

                    KILLIT(i);
                }
                goto BOLT;

            case EXPLODINGBARREL:
            case WOODENHORSE:
            case HORSEONSIDE:
            case FLOORFLAME:
            case FIREBARREL:
            case FIREVASE:
            case NUKEBARREL:
            case NUKEBARRELDENTED:
            case NUKEBARRELLEAKED:
            case TOILETWATER:
            case RUBBERCAN:
            case STEAM:
            case CEILINGSTEAM:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
            case WATERBUBBLEMAKER:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
        }

        BOLT:
        i = nexti;
    }
}

void bounce(short i)
{
    int k, l, daang, dax, day, daz, xvect, yvect, zvect;
    short hitsect;
    spritetype *s = &sprite[i];

    xvect = mulscale10(s->xvel,sintable[(s->ang+512)&2047]);
    yvect = mulscale10(s->xvel,sintable[s->ang&2047]);
    zvect = s->zvel;

    hitsect = s->sectnum;

    k = sector[hitsect].wallptr; l = wall[k].point2;
    daang = getangle(wall[l].x-wall[k].x,wall[l].y-wall[k].y);

    if ( s->z < (hittype[i].floorz+hittype[i].ceilingz)>>1)
        k = sector[hitsect].ceilingheinum;
    else
        k = sector[hitsect].floorheinum;

    dax = mulscale14(k,sintable[(daang)&2047]);
    day = mulscale14(k,sintable[(daang+1536)&2047]);
    daz = 4096;

    k = xvect*dax+yvect*day+zvect*daz;
    l = dax*dax+day*day+daz*daz;
    if ((klabs(k)>>14) < l)
    {
        k = divscale17(k,l);
        xvect -= mulscale16(dax,k);
        yvect -= mulscale16(day,k);
        zvect -= mulscale16(daz,k);
    }

    s->zvel = zvect;
    s->xvel = ksqrt(dmulscale8(xvect,xvect,yvect,yvect));
    s->ang = getangle(xvect,yvect);
}

void moveweapons(void)
{
    short i, j, k, nexti, p, q;
    int dax,day,daz, x, ll;
    unsigned int qq;
    spritetype *s;

    i = headspritestat[4];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        if(s->sectnum < 0) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case RADIUSEXPLOSION:
            case KNEE:
                KILLIT(i);
            case TONGUE:
                T1 = sintable[(T2)&2047]>>9;
                T2 += 32;
                if(T2 > 2047) KILLIT(i);

                if(sprite[s->owner].statnum == MAXSTATUS)
                    if(badguy(&sprite[s->owner]) == 0)
                        KILLIT(i);

                s->ang = sprite[s->owner].ang;
                s->x = sprite[s->owner].x;
                s->y = sprite[s->owner].y;
                if(sprite[s->owner].picnum == APLAYER)
                    s->z = sprite[s->owner].z-(34<<8);
                for(k=0;k<T1;k++)
                {
                    q = EGS(s->sectnum,
                        s->x+((k*sintable[(s->ang+512)&2047])>>9),
                        s->y+((k*sintable[s->ang&2047])>>9),
                        s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),TONGUE,-40+(k<<1),
                        8,8,0,0,0,i,5);
                    sprite[q].cstat = 128;
                    sprite[q].pal = 8;
                }
                q = EGS(s->sectnum,
                    s->x+((k*sintable[(s->ang+512)&2047])>>9),
                    s->y+((k*sintable[s->ang&2047])>>9),
                    s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),INNERJAW,-40,
                    32,32,0,0,0,i,5);
                sprite[q].cstat = 128;
                if( T2 > 512 && T2 < (1024) )
                    sprite[q].picnum = INNERJAW+1;

                goto BOLT;

            case FREEZEBLAST:
                if(s->yvel < 1 || s->extra < 2 || (s->xvel|s->zvel) == 0)
                {
                    j = spawn(i,TRANSPORTERSTAR);
                    sprite[j].pal = 1;
                    sprite[j].xrepeat = 32;
                    sprite[j].yrepeat = 32;
                    KILLIT(i);
                }
                // fall through
            case SHRINKSPARK:
            case RPG:
            case FIRELASER:
            case SPIT:
            case COOLEXPLOSION1:

                if( s->picnum == COOLEXPLOSION1 )
                    if( !issoundplaying(WIERDSHOT_FLY, 1) )
                        spritesound(WIERDSHOT_FLY,i);

                p = -1;

                if(s->picnum == RPG && sector[s->sectnum].lotag == 2)
                {
                    k = s->xvel>>1;
                    ll = s->zvel>>1;
                }
                else
                {
                    k = s->xvel;
                    ll = s->zvel;
                }

                dax = s->x; day = s->y; daz = s->z;

                getglobalz(i);
                qq = CLIPMASK1;

                switch(s->picnum)
                {
                    case RPG:
                        if(hittype[i].picnum != BOSS2 && s->xrepeat >= 10 && sector[s->sectnum].lotag != 2)
                        {
                            j = spawn(i,SMALLSMOKE);
                            sprite[j].z += (1<<8);
                        }
                        break;
                }

                j = movesprite(i,
                    (k*(sintable[(s->ang+512)&2047]))>>14,
                    (k*(sintable[s->ang&2047]))>>14,ll,qq);

                if(s->picnum == RPG && s->yvel >= 0)
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;

                if(s->sectnum < 0) { KILLIT(i); }

                if( (j&49152) != 49152)
                    if(s->picnum != FREEZEBLAST)
                {
                    if(s->z < hittype[i].ceilingz)
                    {
                        j = 16384|(s->sectnum);
                        s->zvel = -1;
                    }
                    else
                        if( ( s->z > hittype[i].floorz && sector[s->sectnum].lotag != 1 ) ||
                            ( s->z > hittype[i].floorz+(16<<8) && sector[s->sectnum].lotag == 1 ) )
                    {
                        j = 16384|(s->sectnum);
                        if(sector[s->sectnum].lotag != 1)
                            s->zvel = 1;
                    }
                }

                if(s->picnum == FIRELASER)
                {
                    for(k=-3;k<2;k++)
                    {
                        x = EGS(s->sectnum,
                            s->x+((k*sintable[(s->ang+512)&2047])>>9),
                            s->y+((k*sintable[s->ang&2047])>>9),
                            s->z+((k*ksgn(s->zvel))*klabs(s->zvel/24)),FIRELASER,-40+(k<<2),
                            s->xrepeat,s->yrepeat,0,0,0,s->owner,5);

                        sprite[x].cstat = 128;
                        sprite[x].pal = s->pal;
                    }
                }
                else if(s->picnum == SPIT) if(s->zvel < 6144)
                    s->zvel += gc-112;

                if( j != 0 )
                {
                    if(s->picnum == COOLEXPLOSION1)
                    {
                        if( (j&49152) == 49152 && sprite[j&(MAXSPRITES-1)].picnum != APLAYER)
                            goto BOLT;
                        s->xvel = 0;
                        s->zvel = 0;
                    }

                    if( (j&49152) == 49152 )
                    {
                        j &= (MAXSPRITES-1);

                        if(s->picnum == FREEZEBLAST && sprite[j].pal == 1 )
                            if( badguy(&sprite[j]) || sprite[j].picnum == APLAYER )
                        {
                            j = spawn(i,TRANSPORTERSTAR);
                            sprite[j].pal = 1;
                            sprite[j].xrepeat = 32;
                            sprite[j].yrepeat = 32;

                            KILLIT(i);
                        }

                        checkhitsprite(j,i);

                        if(sprite[j].picnum == APLAYER)
                        {
                            p = sprite[j].yvel;
                            spritesound(PISTOL_BODYHIT,j);

                            if(s->picnum == SPIT)
                            {
                                ps[p].horiz += 32;
                                ps[p].return_to_center = 8;

                                if(ps[p].loogcnt == 0)
                                {
                                    if(!isspritemakingsound(ps[p].i, DUKE_LONGTERM_PAIN))
                                        spritesound(DUKE_LONGTERM_PAIN,ps[p].i);

                                    j = 3+(TRAND&3);
                                    ps[p].numloogs = j;
                                    ps[p].loogcnt = 24*4;
                                    for(x=0;x < j;x++)
                                    {
                                        ps[p].loogiex[x] = TRAND%xdim;
                                        ps[p].loogiey[x] = TRAND%ydim;
                                    }
                                }
                            }
                        }
                    }
                    else if( (j&49152) == 32768 )
                    {
                        j &= (MAXWALLS-1);

                        if(s->picnum != RPG && s->picnum != FREEZEBLAST && s->picnum != SPIT && ( wall[j].overpicnum == MIRROR || wall[j].picnum == MIRROR ) )
                        {
                            k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                            s->owner = i;
                            spawn(i,TRANSPORTERSTAR);
                            goto BOLT;
                        }
                        else
                        {
                            setsprite(i,dax,day,daz);
                            checkhitwall(i,j,s->x,s->y,s->z,s->picnum);

                            if(s->picnum == FREEZEBLAST)
                            {
                                if( wall[j].overpicnum != MIRROR && wall[j].picnum != MIRROR )
                                {
                                    s->extra >>= 1;
                                    s->yvel--;
                                }

                                k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                                s->ang = ((k<<1) - s->ang)&2047;
                                goto BOLT;
                            }
                        }
                    }
                    else if( (j&49152) == 16384)
                    {
                        setsprite(i,dax,day,daz);

                        if(s->zvel < 0)
                        {
                            if( sector[s->sectnum].ceilingstat&1 )
                                if(sector[s->sectnum].ceilingpal == 0)
                                    KILLIT(i);

                            checkhitceiling(s->sectnum);
                        }

                        if(s->picnum == FREEZEBLAST)
                        {
                            bounce(i);
                            ssp(i,qq);
                            s->extra >>= 1;
                            if(s->xrepeat > 8)
                                s->xrepeat -= 2;
                            if(s->yrepeat > 8)
                                s->yrepeat -= 2;
                            s->yvel--;
                            goto BOLT;
                        }
                    }

                    if(s->picnum != SPIT)
                    {
                        if(s->picnum == RPG)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;

                            if(s->xrepeat < 10)
                            {
                                sprite[k].xrepeat = 6;
                                sprite[k].yrepeat = 6;
                            }
                            else if( (j&49152) == 16384)
                            {
                                if( s->zvel > 0)
                                    spawn(i,EXPLOSION2BOT);
                                else { sprite[k].cstat |= 8; sprite[k].z += (48<<8); }
                            }
                        }
                        else if(s->picnum == SHRINKSPARK)
                        {
                            spawn(i,SHRINKEREXPLOSION);
                            spritesound(SHRINKER_HIT,i);
                            hitradius(i,shrinkerblastradius,0,0,0,0);
                        }
                        else if( s->picnum != COOLEXPLOSION1 && s->picnum != FREEZEBLAST && s->picnum != FIRELASER)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].xrepeat = sprite[k].yrepeat = s->xrepeat>>1;
                            if( (j&49152) == 16384)
                            {
                                if( s->zvel < 0)
                                    { sprite[k].cstat |= 8; sprite[k].z += (72<<8); }
                            }
                        }
                        if( s->picnum == RPG )
                        {
                            spritesound(RPG_EXPLODE,i);

                            if(s->xrepeat >= 10)
                            {
                                x = s->extra;
                                hitradius( i,rpgblastradius, x>>2,x>>1,x-(x>>2),x);
                            }
                            else
                            {
                                x = s->extra+(global_random&3);
                                hitradius( i,(rpgblastradius>>1),x>>2,x>>1,x-(x>>2),x);
                            }
                        }
                    }
                    if(s->picnum != COOLEXPLOSION1) KILLIT(i);
                }
                if(s->picnum == COOLEXPLOSION1)
                {
                    s->shade++;
                    if(s->shade >= 40) KILLIT(i);
                }
                else if(s->picnum == RPG && sector[s->sectnum].lotag == 2 && s->xrepeat >= 10 && rnd(140))
                    spawn(i,WATERBUBBLE);

                goto BOLT;


            case SHOTSPARK1:
                p = findplayer(s,&x);
                execute(i,p,x);
                goto BOLT;
        }
        BOLT:
        i = nexti;
    }
}


void movetransports(void)
{
    char warpspriteto;
    short i, j, k, l, p, sect, sectlotag, nexti, nextj;
    int ll,onfloorz,q;

    i = headspritestat[9]; //Transporters

    while(i >= 0)
    {
        sect = SECT;
        sectlotag = sector[sect].lotag;

        nexti = nextspritestat[i];

        if(OW == i)
        {
            i = nexti;
            continue;
        }

        onfloorz = T5;

        if(T1 > 0) T1--;

        j = headspritesect[sect];
        while(j >= 0)
        {
            nextj = nextspritesect[j];

            switch(sprite[j].statnum)
            {
                case 10:    // Player

                    if( sprite[j].owner != -1 )
                    {
                        p = sprite[j].yvel;

                        ps[p].on_warping_sector = 1;

                        if( ps[p].transporter_hold == 0 && ps[p].jumping_counter == 0 )
                        {
                            if(ps[p].on_ground && sectlotag == 0 && onfloorz && ps[p].jetpack_on == 0 )
                            {
                                if(sprite[i].pal == 0)
                                {
                                    spawn(i,TRANSPORTERBEAM);
                                    spritesound(TELEPORTER,i);
                                }

                                for(k=connecthead;k>=0;k=connectpoint2[k])
                                    if(ps[k].cursectnum == sprite[OW].sectnum)
                                {
                                    ps[k].frag_ps = p;
                                    sprite[ps[k].i].extra = 0;
                                }

                                ps[p].ang = sprite[OW].ang;

                                if(sprite[OW].owner != OW)
                                {
                                    T1 = 13;
                                    hittype[OW].temp_data[0] = 13;
                                    ps[p].transporter_hold = 13;
                                }

                                ps[p].bobposx = ps[p].oposx = ps[p].posx = sprite[OW].x;
                                ps[p].bobposy = ps[p].oposy = ps[p].posy = sprite[OW].y;
                                ps[p].oposz = ps[p].posz = sprite[OW].z-PHEIGHT;

                                changespritesect(j,sprite[OW].sectnum);
                                ps[p].cursectnum = sprite[j].sectnum;

                                if(sprite[i].pal == 0)
                                {
                                    k = spawn(OW,TRANSPORTERBEAM);
                                    spritesound(TELEPORTER,k);
                                }

                                break;
                            }
                        }
                        else if( !(sectlotag == 1 && ps[p].on_ground == 1)  ) break;

                        if(onfloorz == 0 && klabs(SZ-ps[p].posz) < 6144 )
                            if( (ps[p].jetpack_on == 0 ) || (ps[p].jetpack_on && (sync[p].bits&1) ) ||
                                (ps[p].jetpack_on && (sync[p].bits&2) ) )
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if( ps[p].jetpack_on && ( (sync[p].bits&1) || ps[p].jetpack_on < 11 ) )
                                ps[p].posz = sprite[OW].z-6144;
                            else ps[p].posz = sprite[OW].z+6144;
                            ps[p].oposz = ps[p].posz;

                            hittype[ps[p].i].bposx = ps[p].posx;
                            hittype[ps[p].i].bposy = ps[p].posy;
                            hittype[ps[p].i].bposz = ps[p].posz;

                            changespritesect(j,sprite[OW].sectnum);
                            ps[p].cursectnum = sprite[OW].sectnum;

                            break;
                        }

                        k = 0;

                        if( onfloorz && sectlotag == 1 && ps[p].on_ground && ps[p].posz > (sector[sect].floorz-(16<<8)) && ( (sync[p].bits&2) || ps[p].poszv > 2048 ) )
//                        if( onfloorz && sectlotag == 1 && ps[p].posz > (sector[sect].floorz-(6<<8)) )
                        {
                            k = 1;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            if(sprite[ps[p].i].extra > 0)
                                spritesound(DUKE_UNDERWATER,j);
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].ceilingz+(7<<8);

                            ps[p].posxv = 4096-(TRAND&8192);
                            ps[p].posyv = 4096-(TRAND&8192);

                        }

                        if( onfloorz && sectlotag == 2 && ps[p].posz < (sector[sect].ceilingz+(6<<8)) )
                        {
                            k = 1;
//                            if( sprite[j].extra <= 0) break;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(DUKE_GASP,j);

                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].floorz-(7<<8);

                            ps[p].jumping_toggle = 1;
                            ps[p].jumping_counter = 0;
                        }

                        if(k == 1)
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if(sprite[OW].owner != OW)
                                ps[p].transporter_hold = -2;
                            ps[p].cursectnum = sprite[OW].sectnum;

                            changespritesect(j,sprite[OW].sectnum);
                            setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz+PHEIGHT);

                            setpal(&ps[p]);

                            if( (TRAND&255) < 32 )
                                spawn(j,WATERSPLASH2);

                            if(sectlotag == 1)
                                for(l = 0;l < 9;l++)
                            {
                                q = spawn(ps[p].i,WATERBUBBLE);
                                sprite[q].z += TRAND&16383;
                            }
                        }
                    }
                    break;

                case 1:
                    switch(sprite[j].picnum)
                    {
                        case SHARK:
                        case COMMANDER:
                        case OCTABRAIN:
                        case GREENSLIME:
                        case GREENSLIME+1:
                        case GREENSLIME+2:
                        case GREENSLIME+3:
                        case GREENSLIME+4:
                        case GREENSLIME+5:
                        case GREENSLIME+6:
                        case GREENSLIME+7:
                            if(sprite[j].extra > 0)
                                goto JBOLT;
                    }
                    // fall through
                case 4:
                case 5:
                case 12:
                case 13:

                    ll = klabs(sprite[j].zvel);

                    {
                        warpspriteto = 0;
                        if( ll && sectlotag == 2 && sprite[j].z < (sector[sect].ceilingz+ll) )
                            warpspriteto = 1;

                        if( ll && sectlotag == 1 && sprite[j].z > (sector[sect].floorz-ll) )
                            warpspriteto = 1;

                        if( sectlotag == 0 && ( onfloorz || klabs(sprite[j].z-SZ) < 4096) )
                        {
                            if( sprite[OW].owner != OW && onfloorz && T1 > 0 && sprite[j].statnum != 5 )
                            {
                                T1++;
                                goto BOLT;
                            }
                            warpspriteto = 1;
                        }

                        if( warpspriteto ) switch(sprite[j].picnum)
                        {
                            case TRANSPORTERSTAR:
                            case TRANSPORTERBEAM:
                            case TRIPBOMB:
                            case BULLETHOLE:
                            case WATERSPLASH2:
                            case BURNING:
                            case BURNING2:
                            case FIRE:
                            case FIRE2:
                            case TOILETWATER:
                            case LASERLINE:
                                goto JBOLT;
                            case PLAYERONWATER:
                                if(sectlotag == 2)
                                {
                                    sprite[j].cstat &= 32767;
                                    break;
                                }
                                // fall through
                            default:
                                if(sprite[j].statnum == 5 && !(sectlotag == 1 || sectlotag == 2) )
                                    break;
                                // fall through

                            case WATERBUBBLE:
//                                if( rnd(192) && sprite[j].picnum == WATERBUBBLE)
  //                                 break;

                                if(sectlotag > 0)
                                {
                                    k = spawn(j,WATERSPLASH2);
                                    if( sectlotag == 1 && sprite[j].statnum == 4 )
                                    {
                                        sprite[k].xvel = sprite[j].xvel>>1;
                                        sprite[k].ang = sprite[j].ang;
                                        ssp(k,CLIPMASK0);
                                    }
                                }

                                switch(sectlotag)
                                {
                                    case 0:
                                        if(onfloorz)
                                        {
                                            if( sprite[j].statnum == 4 || ( checkcursectnums(sect) == -1 && checkcursectnums(sprite[OW].sectnum)  == -1 ) )
                                            {
                                                sprite[j].x += (sprite[OW].x-SX);
                                                sprite[j].y += (sprite[OW].y-SY);
                                                sprite[j].z -= SZ - sector[sprite[OW].sectnum].floorz;
                                                sprite[j].ang = sprite[OW].ang;

                                                hittype[j].bposx = sprite[j].x;
                                                hittype[j].bposy = sprite[j].y;
                                                hittype[j].bposz = sprite[j].z;

                                                if(sprite[i].pal == 0)
                                                {
                                                    k = spawn(i,TRANSPORTERBEAM);
                                                    spritesound(TELEPORTER,k);

                                                    k = spawn(OW,TRANSPORTERBEAM);
                                                    spritesound(TELEPORTER,k);
                                                }

                                                if( sprite[OW].owner != OW )
                                                {
                                                    T1 = 13;
                                                    hittype[OW].temp_data[0] = 13;
                                                }

                                                changespritesect(j,sprite[OW].sectnum);
                                            }
                                        }
                                        else
                                        {
                                            sprite[j].x += (sprite[OW].x-SX);
                                            sprite[j].y += (sprite[OW].y-SY);
                                            sprite[j].z = sprite[OW].z+4096;

                                            hittype[j].bposx = sprite[j].x;
                                            hittype[j].bposy = sprite[j].y;
                                            hittype[j].bposz = sprite[j].z;

                                            changespritesect(j,sprite[OW].sectnum);
                                        }
                                        break;
                                    case 1:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].ceilingz+ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
                                    case 2:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].floorz-ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
                                }

                                break;
                        }
                }
                break;

            }
            JBOLT:
            j = nextj;
        }
        BOLT:
        i = nexti;
    }
}



void moveactors(void)
{
    int x, m, l, *t;
    short a, i, j, nexti, nextj, sect, p;
    spritetype *s;
    unsigned short k;
    char namBoom = 0;

    i = headspritestat[1];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];

        sect = s->sectnum;

        if( s->xrepeat == 0 || sect < 0 || sect >= MAXSECTORS)
            KILLIT(i);

        t = &hittype[i].temp_data[0];

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;


        switch(s->picnum)
        {
            case DUCK:
            case TARGET:
                if(s->cstat&32)
                {
                    t[0]++;
                    if(t[0] > 60)
                    {
                        t[0] = 0;
                        s->cstat = 128+257+16;
                        s->extra = 1;
                    }
                }
                else
                {
                    j = ifhitbyweapon(i);
                    if( j >= 0 )
                    {
                        s->cstat = 32+128;
                        k = 1;

                        j = headspritestat[1];
                        while(j >= 0)
                        {
                            if( sprite[j].lotag == s->lotag &&
                                sprite[j].picnum == s->picnum )
                            {
                                if( ( sprite[j].hitag && !(sprite[j].cstat&32) ) ||
                                    ( !sprite[j].hitag && (sprite[j].cstat&32) )
                                  )
                                {
                                    k = 0;
                                    break;
                                }
                            }

                            j = nextspritestat[j];
                        }

                        if(k == 1)
                        {
                            operateactivators(s->lotag,-1);
                            operateforcefields(i,s->lotag);
                            operatemasterswitches(s->lotag);
                        }
                    }
                }
                goto BOLT;

            case RESPAWNMARKERRED:
            case RESPAWNMARKERYELLOW:
            case RESPAWNMARKERGREEN:
                T1++;
                if(T1 > respawnitemtime)
                {
                    KILLIT(i);
                }
                if( T1 >= (respawnitemtime>>1) && T1 < ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERYELLOW;
                else if( T1 > ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERGREEN;
                makeitfall(i);
                break;

            case HELECOPT:
            case DUKECAR:

                s->z += s->zvel;
                t[0]++;

                if(t[0] == 4) spritesound(WAR_AMBIENCE2,i);

                if( t[0] > (26*8) )
                {
                    sound(RPG_EXPLODE);
                    for(j=0;j<32;j++) RANDOMSCRAP;
                    earthquaketime = 16;
                    KILLIT(i);
                }
                else if((t[0]&3) == 0)
                    spawn(i,EXPLOSION2);
                ssp(i,CLIPMASK0);
                break;
            case RAT:
                makeitfall(i);
                IFMOVING
                {
                    if( (TRAND&255) < 3 ) spritesound(RATTY,i);
                    s->ang += (TRAND&31)-15+(sintable[(t[0]<<8)&2047]>>11);
                }
                else
                {
                    T1++;
                    if(T1 > 1) { KILLIT(i); }
                    else s->ang = (TRAND&2047);
                }
                if(s->xvel < 128)
                    s->xvel+=2;
                s->ang += (TRAND&3)-6;
                break;
            case QUEBALL:
            case STRIPEBALL:
                if(s->xvel)
                {
                    j = headspritestat[0];
                    while(j >= 0)
                    {
                        nextj = nextspritestat[j];
                        if( sprite[j].picnum == POCKET && ldist(&sprite[j],s) < 52 ) KILLIT(i);
                        j = nextj;
                    }

                    j = clipmove(&s->x,&s->y,&s->z,&s->sectnum,
                        (((s->xvel*(sintable[(s->ang+512)&2047]))>>14)*TICSPERFRAME)<<11,
                        (((s->xvel*(sintable[s->ang&2047]))>>14)*TICSPERFRAME)<<11,
                        24L,(4<<8),(4<<8),CLIPMASK1);

                    if(j&49152)
                    {
                        if( (j&49152) == 32768 )
                        {
                            j &= (MAXWALLS-1);
                            k = getangle(
                                wall[wall[j].point2].x-wall[j].x,
                                wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                        }
                        else if( (j&49152) == 49152 )
                        {
                            j &= (MAXSPRITES-1);
                            checkhitsprite(i,j);
                        }
                    }
                    s->xvel --;
                    if(s->xvel < 0) s->xvel = 0;
                    if( s->picnum == STRIPEBALL )
                    {
                        s->cstat = 257;
                        s->cstat |= 4&s->xvel;
                        s->cstat |= 8&s->xvel;
                    }
                }
                else
                {
                    p = findplayer(s,&x);

                    if( x < 1596)
                    {

//                        if(s->pal == 12)
                        {
                            j = getincangle(ps[p].ang,getangle(s->x-ps[p].posx,s->y-ps[p].posy));
                            if( j > -64 && j < 64 && (sync[p].bits&(1<<29)) )
                                if(ps[p].toggle_key_flag == 1)
                            {
                                a = headspritestat[1];
                                while(a >= 0)
                                {
                                    if(sprite[a].picnum == QUEBALL || sprite[a].picnum == STRIPEBALL)
                                    {
                                        j = getincangle(ps[p].ang,getangle(sprite[a].x-ps[p].posx,sprite[a].y-ps[p].posy));
                                        if( j > -64 && j < 64 )
                                        {
                                            findplayer(&sprite[a],&l);
                                            if(x > l) break;
                                        }
                                    }
                                    a = nextspritestat[a];
                                }
                                if(a == -1)
                                {
                                    if(s->pal == 12)
                                        s->xvel = 164;
                                    else s->xvel = 140;
                                    s->ang = ps[p].ang;
                                    ps[p].toggle_key_flag = 2;
                                }
                            }
                        }
                    }
                    if( x < 512 && s->sectnum == ps[p].cursectnum )
                    {
                        s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                        s->xvel = 48;
                    }
                }

                break;
            case FORCESPHERE:

                if(s->yvel == 0)
                {
                    s->yvel = 1;

                    for(l=512;l<(2048-512);l+= 128)
                        for(j=0;j<2048;j += 128)
                    {
                        k = spawn(i,FORCESPHERE);
                        sprite[k].cstat = 257+128;
                        sprite[k].clipdist = 64;
                        sprite[k].ang = j;
                        sprite[k].zvel = sintable[l&2047]>>5;
                        sprite[k].xvel = sintable[(l+512)&2047]>>9;
                        sprite[k].owner = i;
                    }
                }

                if(t[3] > 0)
                {
                    if(s->zvel < 6144)
                        s->zvel += 192;
                    s->z += s->zvel;
                    if(s->z > sector[sect].floorz)
                        s->z = sector[sect].floorz;
                    t[3]--;
                    if(t[3] == 0)
                        KILLIT(i);
                }
                else if(t[2] > 10)
                {
                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].owner == i && sprite[j].picnum == FORCESPHERE)
                            hittype[j].temp_data[1] = 1+(TRAND&63);
                        j = nextspritestat[j];
                    }
                    t[3] = 64;
                }

                goto BOLT;

            case RECON:

                getglobalz(i);

                if (sector[s->sectnum].ceilingstat&1)
                   s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
                else s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

                if( s->z < sector[sect].ceilingz+(32<<8) )
                    s->z = sector[sect].ceilingz+(32<<8);

                if( ud.multimode < 2 )
                {
                    if( actor_tog == 1)
                    {
                        s->cstat = (short)32768;
                        goto BOLT;
                    }
                    else if(actor_tog == 2) s->cstat = 257;
                }
                IFHIT
                {
                    if( s->extra < 0 && t[0] != -1 )
                    {
                        t[0] = -1;
                        s->extra = 0;
                    }
                    spritesound(RECO_PAIN,i);
                    RANDOMSCRAP;
                }

                if(t[0] == -1)
                {
                    s->z += 1024;
                    t[2]++;
                    if( (t[2]&3) == 0) spawn(i,EXPLOSION2);
                    getglobalz(i);
                    s->ang += 96;
                    s->xvel = 128;
                    j = ssp(i,CLIPMASK0);
                    if(j != 1 || s->z > hittype[i].floorz)
                    {
                        for(l=0;l<16;l++)
                            RANDOMSCRAP;
                        spritesound(LASERTRIP_EXPLODE,i);
                        spawn(i,PIGCOP);
                        ps[myconnectindex].actors_killed++;
                        KILLIT(i);
                    }
                    goto BOLT;
                }
                else
                {
                    if( s->z > hittype[i].floorz-(48<<8) )
                        s->z = hittype[i].floorz-(48<<8);
                }

                p = findplayer(s,&x);
                j = s->owner;

                // 3 = findplayerz, 4 = shoot

                if( t[0] >= 4 )
                {
                    t[2]++;
                    if( (t[2]&15) == 0 )
                    {
                        a = s->ang;
                        s->ang = hittype[i].tempang;
                        spritesound(RECO_ATTACK,i);
                        shoot(i,FIRELASER);
                        s->ang = a;
                    }
                    if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                    {
                        t[0] = 0;
                        t[2] = 0;
                    }
                    else hittype[i].tempang +=
                        getincangle(hittype[i].tempang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))/3;
                }
                else if(t[0] == 2 || t[0] == 3)
                {
                    t[3] = 0;
                    if(s->xvel > 0) s->xvel -= 16;
                    else s->xvel = 0;

                    if(t[0] == 2)
                    {
                        l = ps[p].posz-s->z;
                        if( klabs(l) < (48<<8) ) t[0] = 3;
                        else s->z += ksgn(ps[p].posz-s->z)<<10;
                    }
                    else
                    {
                        t[2]++;
                        if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                        {
                            t[0] = 1;
                            t[2] = 0;
                        }
                        else if( (t[2]&15) == 0 )
                        {
                            spritesound(RECO_ATTACK,i);
                            shoot(i,FIRELASER);
                        }
                    }
                    s->ang += getincangle(s->ang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>2;
                }

                if( t[0] != 2 && t[0] != 3 )
                {
                    l = ldist(&sprite[j],s);
                    if(l <= 1524)
                    {
                        a = s->ang;
                        s->xvel >>= 1;
           