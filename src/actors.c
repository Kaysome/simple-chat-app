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
            