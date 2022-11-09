
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
#include <assert.h>

int conversion = 13;    // by default we think we're 1.3d until compilation informs us otherwise

extern short otherp;

static short total_lines,line_number;
static char checking_ifelse,parsing_state;
static short num_squigilly_brackets;

static short g_i,g_p;
static int g_x;
static int *g_t;
static spritetype *g_sp;

static char compilefile[255] = "(none)";    // file we're currently compiling

enum labeltypes {
    LABEL_ANY    = -1,
    LABEL_DEFINE = 1,
    LABEL_STATE  = 2,
    LABEL_ACTOR  = 4,
    LABEL_ACTION = 8,
    LABEL_AI     = 16,
    LABEL_MOVE   = 32,
};

static const char *labeltypenames[] = {
    "define",
    "state",
    "actor",
    "action",
    "ai",
    "move"
};

static void translatelabeltype(int type, char *buf)
{
    int i;
    
    buf[0] = 0;
    for (i=0;i<6;i++) {
        if (!(type & (1<<i))) continue;
        if (buf[0]) Bstrcat(buf, " or ");
        Bstrcat(buf, labeltypenames[i]);
    }
}

#define NUMKEYWORDS     (int)(sizeof(keyw)/sizeof(keyw[0]))

static const char *keyw[/*NUMKEYWORDS*/] =
{
    "definelevelname",  // 0
    "actor",            // 1    [#]
    "addammo",          // 2    [#]
    "ifrnd",            // 3    [C]
    "enda",             // 4    [:]
    "ifcansee",         // 5    [C]
    "ifhitweapon",      // 6    [#]
    "action",           // 7    [#]
    "ifpdistl",         // 8    [#]
    "ifpdistg",         // 9    [#]
    "else",             // 10   [#]
    "strength",         // 11   [#]
    "break",            // 12   [#]
    "shoot",            // 13   [#]
    "palfrom",          // 14   [#]
    "sound",            // 15   [filename.voc]
    "fall",             // 16   []
    "state",            // 17
    "ends",             // 18
    "define",           // 19
    "//",               // 20
    "ifai",             // 21
    "killit",           // 22
    "addweapon",        // 23
    "ai",               // 24
    "addphealth",       // 25
    "ifdead",           // 26
    "ifsquished",       // 27
    "sizeto",           // 28
    "{",                // 29
    "}",                // 30
    "spawn",            // 31
    "move",             // 32
    "ifwasweapon",      // 33
    "ifaction",         // 34
    "ifactioncount",    // 35
    "resetactioncount", // 36
    "debris",           // 37
    "pstomp",           // 38
    "/*",               // 39
    "cstat",            // 40
    "ifmove",           // 41
    "resetplayer",      // 42
    "ifonwater",        // 43
    "ifinwater",        // 44
    "ifcanshoottarget", // 45
    "ifcount",          // 46
    "resetcount",       // 47
    "addinventory",     // 48
    "ifactornotstayput",// 49
    "hitradius",        // 50
    "ifp",              // 51
    "count",            // 52
    "ifactor",          // 53
    "music",            // 54
    "include",          // 55
    "ifstrength",       // 56
    "definesound",      // 57
    "guts",             // 58
    "ifspawnedby",      // 59
    "gamestartup",      // 60
    "wackplayer",       // 61
    "ifgapzl",          // 62
    "ifhitspace",       // 63
    "ifoutside",        // 64
    "ifmultiplayer",    // 65
    "operate",          // 66
    "ifinspace",        // 67
    "debug",            // 68
    "endofgame",        // 69
    "ifbulletnear",     // 70
    "ifrespawn",        // 71
    "iffloordistl",     // 72
    "ifceilingdistl",   // 73
    "spritepal",        // 74
    "ifpinventory",     // 75
    "betaname",         // 76
    "cactor",           // 77
    "ifphealthl",       // 78
    "definequote",      // 79
    "quote",            // 80
    "ifinouterspace",   // 81
    "ifnotmoving",      // 82
    "respawnhitag",     // 83
    "tip",              // 84
    "ifspritepal",      // 85
    "money",            // 86
    "soundonce",        // 87
    "addkills",         // 88
    "stopsound",        // 89
    "ifawayfromwall",   // 90
    "ifcanseetarget",   // 91
    "globalsound",      // 92
    "lotsofglass",      // 93
    "ifgotweaponce",    // 94
    "getlastpal",       // 95
    "pkick",            // 96
    "mikesnd",          // 97
    "useractor",        // 98
    "sizeat",           // 99
    "addstrength",      // 100   [#]
    "cstator",          // 101
    "mail",             // 102
    "paper",            // 103
    "tossweapon",       // 104
    "sleeptime",        // 105
    "nullop",           // 106
    "definevolumename", // 107
    "defineskillname",  // 108
    "ifnosounds",       // 109
    "clipdist",         // 110
    "ifangdiffl",       // 111
};


short getincangle(short a,short na)
{
    a &= 2047;
    na &= 2047;

    if(klabs(a-na) < 1024)
        return (na-a);
    else
    {
        if(na > 1024) na -= 2048;
        if(a > 1024) a -= 2048;

        na -= 2048;
        a -= 2048;
        return (na-a);
    }
}

char ispecial(char c)
{
    if(c == 0x0a)
    {
        line_number++;
        return 1;
    }

    if(c == ' ' || c == 0x0d)
        return 1;

    return 0;
}

char isaltok(char c)
{
    return ( isalnum(c) || c == '{' || c == '}' || c == '/' || c == '*' || c == '-' || c == '_' || c == '.');
}

void getglobalz(short i)
{
    int hz,lz,zr;

    spritetype *s = &sprite[i];

    if( s->statnum == 10 || s->statnum == 6 || s->statnum == 2 || s->statnum == 1 || s->statnum == 4)
    {
        if(s->statnum == 4)
            zr = 4L;
        else zr = 127L;

        getzrange(s->x,s->y,s->z-(FOURSLEIGHT),s->sectnum,&hittype[i].ceilingz,&hz,&hittype[i].floorz,&lz,zr,CLIPMASK0);

        if( (lz&49152) == 49152 && (sprite[lz&(MAXSPRITES-1)].cstat&48) == 0 )
        {
            lz &= (MAXSPRITES-1);
            if( badguy(&sprite[lz]) && sprite[lz].pal != 1)
            {
                if( s->statnum != 4 )
                {
                    hittype[i].dispicnum = -4; // No shadows on actors
                    s->xvel = -256;
                    ssp(i,CLIPMASK0);
                }
            }
            else if(sprite[lz].picnum == APLAYER && badguy(s) )
            {
                hittype[i].dispicnum = -4; // No shadows on actors
                s->xvel = -256;
                ssp(i,CLIPMASK0);
            }
            else if(s->statnum == 4 && sprite[lz].picnum == APLAYER)
                if(s->owner == lz)
            {
                hittype[i].ceilingz = sector[s->sectnum].ceilingz;
                hittype[i].floorz   = sector[s->sectnum].floorz;
            }
        }
    }
    else
    {
        hittype[i].ceilingz = sector[s->sectnum].ceilingz;
        hittype[i].floorz   = sector[s->sectnum].floorz;
    }
}


void makeitfall(short i)
{
    spritetype *s = &sprite[i];
    int hz,lz,c;

    if( floorspace(s->sectnum) )
        c = 0;
    else
    {
        if( ceilingspace(s->sectnum) || sector[s->sectnum].lotag == 2)
            c = gc/6;
        else c = gc;
    }

    if( ( s->statnum == 1 || s->statnum == 10 || s->statnum == 2 || s->statnum == 6 ) )
        getzrange(s->x,s->y,s->z-(FOURSLEIGHT),s->sectnum,&hittype[i].ceilingz,&hz,&hittype[i].floorz,&lz,127L,CLIPMASK0);
    else
    {
        hittype[i].ceilingz = sector[s->sectnum].ceilingz;
        hittype[i].floorz   = sector[s->sectnum].floorz;
    }

    if( s->z < hittype[i].floorz-(FOURSLEIGHT) )
    {
        if( sector[s->sectnum].lotag == 2 && s->zvel > 3122 )
            s->zvel = 3144;
        if(s->zvel < 6144)
            s->zvel += c;
        else s->zvel = 6144;
        s->z += s->zvel;
    }
    if( s->z >= hittype[i].floorz-(FOURSLEIGHT) )
    {
        s->z = hittype[i].floorz - FOURSLEIGHT;
        s->zvel = 0;
    }
}


void getlabel(void)
{
    int i;

    while( isalnum(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        textptr++;
        if( *textptr == 0)
            return;
    }

    i = 0;
    while( ispecial(*textptr) == 0 ) {
        if (i < MAXLABELLEN - 1) {
            label[labelcnt * MAXLABELLEN + i] = *textptr;
            i++;
        }
        textptr++;
    }
    label[labelcnt * MAXLABELLEN + i] = 0;
}

int keyword(void)
{
    int i;
    char *temptextptr;

    temptextptr = textptr;

    while( isaltok(*temptextptr) == 0 )
    {
        temptextptr++;
        if( *temptextptr == 0 )
            return 0;
    }

    i = 0;
    while( isaltok(*temptextptr) )
    {
        buf[i] = *(temptextptr++);
        i++;
    }
    buf[i] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
        if( strcmp( buf,keyw[i]) == 0 )
            return i;

    return -1;
}

int transword(void) //Returns its code #
{
    int i, l;

    while( isaltok(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        if( *textptr == 0 )
            return -1;
        textptr++;
    }

    l = 0;
    while( isaltok(*(textptr+l)) )
    {
        buf[l] = textptr[l];
        l++;
    }
    buf[l] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
    {
        if( strcmp( buf,keyw[i]) == 0 )
        {
            *scriptptr = i;
            textptr += l;
            scriptptr++;
            return i;
        }
    }

    textptr += l;

    if( buf[0] == '{' && buf[1] != 0)
        buildprintf("  * ERROR!(L%d %s) Expecting a SPACE or CR between '{' and '%s'.\n",line_number,compilefile,buf+1);
    else if( buf[0] == '}' && buf[1] != 0)
        buildprintf("  * ERROR!(L%d %s) Expecting a SPACE or CR between '}' and '%s'.\n",line_number,compilefile,buf+1);
    else if( buf[0] == '/' && buf[1] == '/' && buf[2] != 0 )
        buildprintf("  * ERROR!(L%d %s) Expecting a SPACE between '//' and '%s'.\n",line_number,compilefile,buf+2);
    else if( buf[0] == '/' && buf[1] == '*' && buf[2] != 0 )
        buildprintf("  * ERROR!(L%d %s) Expecting a SPACE between '/*' and '%s'.\n",line_number,compilefile,buf+2);
    else if( buf[0] == '*' && buf[1] == '/' && buf[2] != 0 )
        buildprintf("  * ERROR!(L%d %s) Expecting a SPACE between '*/' and '%s'.\n",line_number,compilefile,buf+2);
    else buildprintf("  * ERROR!(L%d %s) Expecting key word, but found '%s'.\n",line_number,compilefile,buf);

    error++;
    return -1;
}

int transnum(int type)
{
    int i, l;

    while( isaltok(*textptr) == 0 )
    {
        if(*textptr == 0x0a) line_number++;
        textptr++;
        if( *textptr == 0 )
            return -1;  // eof
    }


    l = 0;
    while( isaltok(*(textptr+l)) )
    {
        buf[l] = textptr[l];
        l++;
    }
    buf[l] = 0;

    for(i=0;i<NUMKEYWORDS;i++)
        if( strcmp( buf,keyw[i]) == 0 )
        {
            error++;
            buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,buf);
            textptr+=l;
        }


    for(i=0;i<labelcnt;i++)
    {
        if( !Bstrcmp(buf,label+(i * MAXLABELLEN)) )
        {
            char el[64], gl[64];

            if (labeltype[i] & type) {
                *(scriptptr++) = labelcode[i];
                textptr += l;
                return labeltype[i];
            }
            *(scriptptr++) = 0;
            textptr += l;
            translatelabeltype(type, el);
            translatelabeltype(labeltype[i], gl);
            buildprintf("  * WARNING!(L%d %s) Expected a '%s' label but found a '%s' label instead.\n",line_number,compilefile,el,gl);
            return -1;  // valid label name, but wrong type
        }
    }

    if( isdigit(*textptr) == 0 && *textptr != '-')
    {
        buildprintf("  * ERROR!(L%d %s) Parameter '%s' is undefined.\n",line_number,compilefile,buf);
        error++;
        textptr+=l;
        return -1;  // error!
    }

    *scriptptr = atoi(textptr);
    scriptptr++;

    textptr += l;

    return 0;   // literal value
}


/**
 * Encode a scriptptr into a form suitable for portably
 * inserting into bytecode. We store the pointer as INT_MIN
 * plus the offset from the start of the script buffer, just
 * to make it perhaps a little more obvious what is happening.
 */
int encodescriptptr(int *scptr)
{
    int offs = (int)(scptr - script);
    assert(offs >= 0);
    assert(offs < MAXSCRIPTSIZE);
    return INT_MIN+offs;
}

/**
 * Decode an encoded representation of a scriptptr
 */
int *decodescriptptr(int scptr)
{
    assert(scptr <= 0);
    return script + (scptr - INT_MIN);
}

char parsecommand(void)
{
    int i, j, k, *tempscrptr, tw;
    char done;

    if ((unsigned)(scriptptr-script) > MAXSCRIPTSIZE) {
        Bsprintf(buf,"FATAL ERROR: Compiled size of CON code exceeds maximum size!\n"
            "Please notify JonoF so the maximum may be increased in a future release.");
        gameexit(buf);
    }

    if( error > 12 || ( *textptr == '\0' ) || ( *(textptr+1) == '\0' ) ) return 1;

    tw = transword();

    switch(tw)
    {
        default:
        case -1:
            return 0; //End
        case 39:    //multi-line comment
            scriptptr--;
            j = line_number;
            do
            {
                textptr++;
                if(*textptr == 0x0a) line_number++;
                if( *textptr == 0 )
                {
                    buildprintf("  * ERROR!(L%d %s) Found '/*' with no '*/'.\n",j,compilefile);
                    error++;
                    return 0;
                }
            }
            while( *textptr != '*' || *(textptr+1) != '/' );
            textptr+=2;
            return 0;
        case 17:    //state
            if( parsing_actor == 0 && parsing_state == 0 )
            {
                getlabel();
                scriptptr--;
                labelcode[labelcnt] = encodescriptptr(scriptptr);
                labeltype[labelcnt] = LABEL_STATE;
                labelcnt++;

                parsing_state = 1;

                return 0;
            }

            getlabel();

            for(i=0;i<NUMKEYWORDS;i++)
                if( strcmp( label+(labelcnt * MAXLABELLEN),keyw[i]) == 0 )
                {
                    error++;
                    buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                    return 0;
                }

            for(j=0;j<labelcnt;j++)
            {
                if( !Bstrcmp(label+(j * MAXLABELLEN),label+(labelcnt * MAXLABELLEN)) )
                {
                    if (labeltype[j] & LABEL_STATE) {
                        *scriptptr = labelcode[j];
                        break;
                    } else {
                        char gl[64];

                        translatelabeltype(labeltype[j], gl);
                        buildprintf("  * WARNING!(L%d %s) Expected a state label, found a %s instead. Neutering.\n",
                            line_number,compilefile,gl);
                        *(scriptptr-1) = 106;   // nullop
                        return 0;
                    }
                }
            }

            if(j==labelcnt)
            {
                buildprintf("  * ERROR!(L%d %s) State '%s' not found.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                error++;
            }
            scriptptr++;
            return 0;

        case 15:    //sound
        case 92:    //globalsound
        case 87:    //soundonce
        case 89:    //stopsound
        case 93:    //lotsofglass
            transnum(LABEL_DEFINE);
            return 0;

        case 18:    //ends
            if( parsing_state == 0 )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'ends' with no 'state'.\n",line_number,compilefile);
                error++;
            }
//            else
            {
                if( num_squigilly_brackets > 0 )
                {
                    buildprintf("  * ERROR!(L%d %s) Found more '{' than '}' before 'ends'.\n",line_number,compilefile);
                    error++;
                }
                if( num_squigilly_brackets < 0 )
                {
                    buildprintf("  * ERROR!(L%d %s) Found more '}' than '{' before 'ends'.\n",line_number,compilefile);
                    error++;
                }
                parsing_state = 0;
            }
            return 0;
        case 19:    //define
            getlabel();
            // Check to see it's already defined

            for(i=0;i<NUMKEYWORDS;i++)
                if( strcmp( label+(labelcnt * MAXLABELLEN),keyw[i]) == 0 )
                {
                    error++;
                    buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                    return 0;
                }

            for(i=0;i<labelcnt;i++)
            {
                if( strcmp(label+(labelcnt * MAXLABELLEN),label+(i * MAXLABELLEN)) == 0 )
                {
                    warning++;
                    buildprintf("  * WARNING.(L%d %s) Duplicate definition '%s' ignored.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                    break;
                }
            }

            transnum(LABEL_DEFINE);
            if(i == labelcnt) {
                labelcode[labelcnt] = *(scriptptr-1);
                labeltype[labelcnt] = LABEL_DEFINE;
                labelcnt++;
            }
            scriptptr -= 2;
            return 0;
        case 14:    //palfrom

            for(j = 0;j < 4;j++)
            {
                if( keyword() == -1 )
                    transnum(LABEL_DEFINE);
                else break;
            }

            while(j < 4)
            {
                *scriptptr = 0;
                scriptptr++;
                j++;
            }
            return 0;

        case 32:    //move
            if( parsing_actor || parsing_state )
            {
                transnum(LABEL_MOVE);

                j = 0;
                while(keyword() == -1)
                {
                    transnum(LABEL_DEFINE);
                    scriptptr--;
                    j |= *scriptptr;
                }
                *scriptptr = j;
                scriptptr++;
            }
            else
            {
                scriptptr--;
                getlabel();
                // Check to see it's already defined

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt * MAXLABELLEN),keyw[i]) == 0 )
                    {
                        error++;
                        buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        return 0;
                    }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt * MAXLABELLEN),label+(i * MAXLABELLEN)) == 0 )
                    {
                        warning++;
                        buildprintf("  * WARNING.(L%d %s) Duplicate move '%s' ignored.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        break;
                    }
                if(i == labelcnt) {
                    labelcode[labelcnt] = encodescriptptr(scriptptr);
                    labeltype[labelcnt] = LABEL_MOVE;
                    labelcnt++;
                }
                for(j=0;j<2;j++)
                {
                    if(keyword() >= 0) break;
                    transnum(LABEL_DEFINE);
                }
                for(k=j;k<2;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 54:    //music
            {
                scriptptr--;
                transnum(LABEL_DEFINE); // Volume Number (0/4)
                scriptptr--;

                k = *scriptptr-1;

                if(k >= 0) // if it's background music
                {
                    i = 0;
                    while(keyword() == -1)
                    {
                        while( isaltok(*textptr) == 0 )
                        {
                            if(*textptr == 0x0a) line_number++;
                            textptr++;
                            if( *textptr == 0 ) break;
                        }
                        j = 0;
                        while( isaltok(*(textptr+j)) )
                        {
                            music_fn[k][i][j] = textptr[j];
                            j++;
                        }
                        music_fn[k][i][j] = '\0';
                        textptr += j;
                        if(i > 9) break;
                        i++;
                    }
                }
                else
                {
                    i = 0;
                    while(keyword() == -1)
                    {
                        while( isaltok(*textptr) == 0 )
                        {
                            if(*textptr == 0x0a) line_number++;
                            textptr++;
                            if( *textptr == 0 ) break;
                        }
                        j = 0;
                        while( isaltok(*(textptr+j)) )
                        {
                            env_music_fn[i][j] = textptr[j];
                            j++;
                        }
                        env_music_fn[i][j] = '\0';

                        textptr += j;
                        if(i > 9) break;
                        i++;
                    }
                }
            }
            return 0;
        case 55:    //include
            scriptptr--;
            while( isaltok(*textptr) == 0 )
            {
                if(*textptr == 0x0a) line_number++;
                textptr++;
                if( *textptr == 0 ) break;
            }
            j = 0;
            while( isaltok(*textptr) )
            {
                buf[j] = *(textptr++);
                j++;
            }
            buf[j] = '\0';

            {
                short temp_line_number;
                char  temp_ifelse_check;
                char *origtptr, *mptr;
                char parentcompilefile[255];
                int fp;

                fp = kopen4load(buf,loadfromgrouponly);
                if(fp < 0)
                {
                    error++;
                    buildprintf("  * ERROR!(L%d %s) Could not find '%s'.\n",line_number,compilefile,buf);
                    return 0;
                }

                j = kfilelength(fp);

                mptr = Bmalloc(j+1);
                if (!mptr) {
                    kclose(fp);
                    error++;
                    buildprintf("  * ERROR!(L%d %s) Could not allocate %d bytes to include '%s'.\n",
                        line_number,compilefile,j,buf);
                    return 0;
                }

                buildprintf("Including: %s (%d bytes)\n",buf, j);
                kread(fp, mptr, j);
                kclose(fp);
                mptr[j] = 0;

                origtptr = textptr;

                strcpy(parentcompilefile, compilefile);
                strcpy(compilefile, buf);
                temp_line_number = line_number;
                line_number = 1;
                temp_ifelse_check = checking_ifelse;
                checking_ifelse = 0;

                textptr = mptr;

                do done = parsecommand(); while (!done);

                strcpy(compilefile, parentcompilefile);
                total_lines += line_number;
                line_number = temp_line_number;
                checking_ifelse = temp_ifelse_check;
                
                textptr = origtptr;
                
                Bfree(mptr);
            }

            return 0;
        case 24:    //ai
            if( parsing_actor || parsing_state )
                transnum(LABEL_AI);
            else
            {
                scriptptr--;
                getlabel();

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt * MAXLABELLEN),keyw[i]) == 0 )
                    {
                        error++;
                        buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        return 0;
                    }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt * MAXLABELLEN),label+(i * MAXLABELLEN)) == 0 )
                    {
                        warning++;
                        buildprintf("  * WARNING.(L%d %s) Duplicate ai '%s' ignored.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        break;
                    }

                if(i == labelcnt) {
                    labelcode[labelcnt] = encodescriptptr(scriptptr);
                    labeltype[labelcnt] = LABEL_AI;
                    labelcnt++;
                }

                for(j=0;j<3;j++)
                {
                    if(keyword() >= 0) break;
                    if(j == 1)
                        transnum(LABEL_ACTION);
                    else if(j == 2)
                    {
                        transnum(LABEL_MOVE);
                        k = 0;
                        while(keyword() == -1)
                        {
                            transnum(LABEL_DEFINE);
                            scriptptr--;
                            k |= *scriptptr;
                        }
                        *scriptptr = k;
                        scriptptr++;
                        return 0;
                    }
                }
                for(k=j;k<3;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 7:     //action
            if( parsing_actor || parsing_state )
                transnum(LABEL_ACTION);
            else
            {
                scriptptr--;
                getlabel();
                // Check to see it's already defined

                for(i=0;i<NUMKEYWORDS;i++)
                    if( strcmp( label+(labelcnt * MAXLABELLEN),keyw[i]) == 0 )
                    {
                        error++;
                        buildprintf("  * ERROR!(L%d %s) Symbol '%s' is a key word.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        return 0;
                    }

                for(i=0;i<labelcnt;i++)
                    if( strcmp(label+(labelcnt * MAXLABELLEN),label+(i * MAXLABELLEN)) == 0 )
                    {
                        warning++;
                        buildprintf("  * WARNING.(L%d %s) Duplicate action '%s' ignored.\n",line_number,compilefile,label+(labelcnt * MAXLABELLEN));
                        break;
                    }

                if(i == labelcnt) {
                    labelcode[labelcnt] = encodescriptptr(scriptptr);
                    labeltype[labelcnt] = LABEL_ACTION;
                    labelcnt++;
                }

                for(j=0;j<5;j++)
                {
                    if(keyword() >= 0) break;
                    transnum(LABEL_DEFINE);
                }
                for(k=j;k<5;k++)
                {
                    *scriptptr = 0;
                    scriptptr++;
                }
            }
            return 0;

        case 1:     //actor
            if( parsing_state )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'actor' within 'state'.\n",line_number,compilefile);
                error++;
            }

            if( parsing_actor )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'actor' within 'actor'.\n",line_number,compilefile);
                error++;
            }

            num_squigilly_brackets = 0;
            scriptptr--;
            parsing_actor = scriptptr;

            transnum(LABEL_DEFINE);
            scriptptr--;
            actorscrptr[*scriptptr] = parsing_actor;

            for(j=0;j<4;j++)
            {
                *(parsing_actor+j) = 0;
                if(j == 3)
                {
                    j = 0;
                    while(keyword() == -1)
                    {
                        transnum(LABEL_DEFINE);
                        scriptptr--;
                        j |= *scriptptr;
                    }
                    *scriptptr = j;
                    scriptptr++;
                    break;
                }
                else
                {
                    if(keyword() >= 0)
                    {
                        for (i=4-j; i>0; i--) *(scriptptr++) = 0;
                        break;
                    }
                    switch (j)
                    {
                        case 0: transnum(LABEL_DEFINE); break;
                        case 1: transnum(LABEL_ACTION); break;
                        case 2: transnum(LABEL_MOVE|LABEL_DEFINE); break;
                    }
                    *(parsing_actor+j) = *(scriptptr-1);
                }
            }

            checking_ifelse = 0;

            return 0;

        case 98:    //useractor

            if( parsing_state )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'useritem' within 'state'.\n",line_number,compilefile);
                error++;
            }

            if( parsing_actor )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'useritem' within 'actor'.\n",line_number,compilefile);
                error++;
            }

            num_squigilly_brackets = 0;
            scriptptr--;
            parsing_actor = scriptptr;

            transnum(LABEL_DEFINE);
            scriptptr--;
            j = *scriptptr;

            transnum(LABEL_DEFINE);
            scriptptr--;
            actorscrptr[*scriptptr] = parsing_actor;
            actortype[*scriptptr] = j;

            for(j=0;j<4;j++)
            {
                *(parsing_actor+j) = 0;
                if(j == 3)
                {
                    j = 0;
                    while(keyword() == -1)
                    {
                        transnum(LABEL_DEFINE);
                        scriptptr--;
                        j |= *scriptptr;
                    }
                    *scriptptr = j;
                    scriptptr++;
                    break;
                }
                else
                {
                    if(keyword() >= 0)
                    {
                        for (i=4-j; i>0; i--) *(scriptptr++) = 0;
                        break;
                    }
                    switch (j)
                    {
                        case 0: transnum(LABEL_DEFINE); break;
                        case 1: transnum(LABEL_ACTION); break;
                        case 2: transnum(LABEL_MOVE|LABEL_DEFINE); break;
                    }
                    *(parsing_actor+j) = *(scriptptr-1);
                }
            }

            checking_ifelse = 0;

            return 0;

        case 11:    //strength
        case 13:    //shoot
        case 25:    //addphealth
        case 31:    //spawn
        case 40:    //cstat
        case 52:    //count
        case 69:    //endofgame
        case 74:    //spritepal
        case 77:    //cactor
        case 80:    //quote
        case 86:    //money
        case 88:    //addkills
        case 68:    //debug
        case 100:   //addstrength
        case 101:   //cstator
        case 102:   //mail
        case 103:   //paper
        case 105:   //sleeptime
        case 110:   //clipdist
            transnum(LABEL_DEFINE);
            return 0;

        case 2:     //addammo
        case 23:    //addweapon
        case 28:    //sizeto
        case 99:    //sizeat
        case 37:    //debris
        case 48:    //addinventory
        case 58:    //guts
            transnum(LABEL_DEFINE);
            transnum(LABEL_DEFINE);
            break;
        case 50:    //hitradius
            transnum(LABEL_DEFINE);
            transnum(LABEL_DEFINE);
            transnum(LABEL_DEFINE);
            transnum(LABEL_DEFINE);
            transnum(LABEL_DEFINE);
            break;
        case 10:    //else
            if( checking_ifelse )
            {
                checking_ifelse--;
                tempscrptr = scriptptr;
                scriptptr++; //Leave a spot for the fail location
                parsecommand();
                *tempscrptr = encodescriptptr(scriptptr);
            }
            else
            {
                scriptptr--;
                error++;
                buildprintf("  * ERROR!(L%d %s) Found 'else' with no 'if'.\n",line_number,compilefile);
            }

            return 0;

        case 75:    //ifpinventory
            transnum(LABEL_DEFINE);
            // fall through
        case 3:     //ifrnd
        case 8:     //ifpdistl
        case 9:     //ifpdistg
        case 21:    //ifai
        case 33:    //ifwasweapon
        case 34:    //ifaction
        case 35:    //ifactioncount
        case 41:    //ifmove
        case 46:    //ifcount
        case 53:    //ifactor
        case 56:    //ifstrength
        case 59:    //ifspawnedby
        case 62:    //ifgapzl
        case 72:    //iffloordistl
        case 73:    //ifceilingdistl
        case 78:    //ifphealthl
        case 85:    //ifspritepal
        case 94:    //ifgotweaponce
        case 111:   //ifangdiffl
            switch (tw) {
                case 21: transnum(LABEL_AI); break;
                case 34: transnum(LABEL_ACTION); break;
                case 41: transnum(LABEL_MOVE); break;
                default: transnum(LABEL_DEFINE); break;
            }
            // fall through
        case 43:    //ifonwater
        case 44:    //ifinwater
        case 49:    //ifactornotstayput
        case 5:     //ifcansee
        case 6:     //ifhitweapon
        case 27:    //ifsquished
        case 26:    //ifdead
        case 45:    //ifcanshoottarget
        case 51:    //ifp
        case 63:    //ifhitspace
        case 64:    //ifoutside
        case 65:    //ifmultiplayer
        case 67:    //ifinspace
        case 70:    //ifbulletnear
        case 71:    //ifrespawn
        case 81:    //ifinouterspace
        case 82:    //ifnotmoving
        case 90:    //ifawayfromwall
        case 91:    //ifcanseetarget
        case 109:   //ifnosounds

            if(tw == 51)
            {
                j = 0;
                do
                {
                    transnum(LABEL_DEFINE);
                    scriptptr--;
                    j |= *scriptptr;
                }
                while(keyword() == -1);
                *scriptptr = j;
                scriptptr++;
            }

            tempscrptr = scriptptr;
            scriptptr++; //Leave a spot for the fail location

            do
            {
                j = keyword();
                if(j == 20 || j == 39)
                    parsecommand();
            } while(j == 20 || j == 39);

            parsecommand();

            *tempscrptr = encodescriptptr(scriptptr);

            checking_ifelse++;
            return 0;
        case 29:    //{
            num_squigilly_brackets++;
            do
                done = parsecommand();
            while( done == 0 );
            return 0;
        case 30:    //}
            num_squigilly_brackets--;
            if( num_squigilly_brackets < 0 )
            {
                buildprintf("  * ERROR!(L%d %s) Found more '}' than '{'.\n",line_number,compilefile);
                error++;
            }
            return 1;
        case 76:    //betaname
            scriptptr--;
            j = 0;
            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
            {
                betaname[j] = *textptr;
                j++; textptr++;
            }
            betaname[j] = 0;
            return 0;
        case 20:    //single-line comment
            scriptptr--; //Negate the rem
            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
                textptr++;

            // line_number++;
            return 0;

        case 107:   //definevolumename
            scriptptr--;
            transnum(LABEL_DEFINE);
            scriptptr--;
            j = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            if (j < 0 || j >= 4)
            {
                buildprintf("  * ERROR!(L%d %s) Volume number exceeds maximum volume count.\n",line_number,compilefile);
                error++;
                while( *textptr != 0x0a && *textptr != 0 ) textptr++;
                break;
            }
        
            i = 0;

            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
            {
                volume_names[j][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    buildprintf("  * ERROR!(L%d %s) Volume name exceeds character size limit of 32.\n",line_number,compilefile);
                    error++;
                    while( *textptr != 0x0a && *textptr != 0 ) textptr++;       // JBF 20040127: end of file checked
                    break;
                }
            }
            volume_names[j][i] = '\0';
            return 0;
        case 108:   //defineskillname
            scriptptr--;
            transnum(LABEL_DEFINE);
            scriptptr--;
            j = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            if (j < 0 || j >= 5)
            {
                buildprintf("  * ERROR!(L%d %s) Skill number exceeds maximum skill count.\n",line_number,compilefile);
                error++;
                while( *textptr != 0x0a && *textptr != 0 ) textptr++;
                break;
            }
        
            i = 0;

            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
            {
                skill_names[j][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    buildprintf("  * ERROR!(L%d %s) Skill name exceeds character size limit of 32.\n",line_number,compilefile);
                    error++;
                    while( *textptr != 0x0a && *textptr != 0 ) textptr++;       // JBF 20040127: end of file checked
                    break;
                }
            }
            skill_names[j][i] = '\0';
            return 0;

        case 0:     //definelevelname
            scriptptr--;
            transnum(LABEL_DEFINE);
            scriptptr--;
            j = *scriptptr;
            transnum(LABEL_DEFINE);
            scriptptr--;
            k = *scriptptr;
            while( *textptr == ' ' ) textptr++;

            if (j < 0 || j >= 4)
            {
                buildprintf("  * WARNING!(L%d %s) Volume number exceeds maximum volume count.\n",line_number,compilefile);
                warning++;
                while( *textptr != 0x0a && *textptr != 0 ) textptr++;
                break;
            }
            if (k < 0 || k >= 11)
            {
                buildprintf("  * WARNING!(L%d %s) Level number exceeds maximum levels-per-episode count.\n",
                line_number,compilefile);
                warning++;
                while( *textptr != 0x0a && *textptr != 0 ) textptr++;
                break;
            }
        
            i = 0;
            while( *textptr != ' ' && *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )   // JBF 20040127: end of file checked
            {
                level_file_names[j*11+k][i] = *textptr;
                textptr++,i++;
                if(i > 127)
                {
                    buildprintf("  * ERROR!(L%d %s) Level file name exceeds character size limit of 128.\n",line_number,compilefile);
                    error++;
                    while( *textptr != ' ' && *textptr != 0) textptr++;     // JBF 20040127: end of file checked
                    break;
                }
            }
            level_names[j*11+k][i] = '\0';

            while( *textptr == ' ' ) textptr++;

            partime[j*11+k] =
                (((*(textptr+0)-'0')*10+(*(textptr+1)-'0'))*26*60)+
                (((*(textptr+3)-'0')*10+(*(textptr+4)-'0'))*26);

            textptr += 5;
            while( *textptr == ' ' ) textptr++;

            designertime[j*11+k] =
                (((*(textptr+0)-'0')*10+(*(textptr+1)-'0'))*26*60)+
                (((*(textptr+3)-'0')*10+(*(textptr+4)-'0'))*26);

            textptr += 5;
            while( *textptr == ' ' ) textptr++;

            i = 0;

            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
            {
                level_names[j*11+k][i] = toupper(*textptr);
                textptr++,i++;
                if(i >= 32)
                {
                    buildprintf("  * ERROR!(L%d %s) Level name exceeds character size limit of 32.\n",line_number,compilefile);
                    error++;
                    while( *textptr != 0x0a && *textptr != 0 ) textptr++;       // JBF 20040127: end of file checked
                    break;
                }
            }
            level_names[j*11+k][i] = '\0';
            return 0;

        case 79:    //definequote
            scriptptr--;
            transnum(LABEL_DEFINE);
            k = *(scriptptr-1);
            if(k >= NUMOFFIRSTTIMEACTIVE)
            {
                buildprintf("  * ERROR!(L%d %s) Quote amount exceeds limit of %d characters.\n",line_number,compilefile,NUMOFFIRSTTIMEACTIVE);
                error++;
            }
            scriptptr--;
            i = 0;
            while( *textptr == ' ' )
                textptr++;

            while( *textptr != 0x0a && *textptr != 0x0d && *textptr != 0 )      // JBF 20040127: end of file checked
            {
                fta_quotes[k][i] = *textptr;
                textptr++,i++;
                if(i >= 64)
                {
                    buildprintf("  * ERROR!(L%d %s) Quote exceeds character size limit of 64.\n",line_number,compilefile);
                    error++;
                    while( *textptr != 0x0a && *textptr != 0 ) textptr++;       // JBF 20040127: end of file checked
                    break;
                }
            }
            fta_quotes[k][i] = '\0';
            return 0;
        case 57:    //definesound
            scriptptr--;
            transnum(LABEL_DEFINE);
            k = *(scriptptr-1);
            if(k >= NUM_SOUNDS)
            {
                buildprintf("  * ERROR!(L%d %s) Exceeded sound limit of %d.\n",line_number,compilefile,NUM_SOUNDS);
                error++;
            }
            scriptptr--;
            i = 0;
            while( *textptr == ' ')
                textptr++;

            while( *textptr != ' ' && *textptr != 0 )       // JBF 20040127: end of file checked
            {
                sounds[k][i] = *textptr;
                textptr++,i++;
                if(i >= 13)
                {
                    buildprintf("%s\n",sounds[k]);
                    buildprintf("  * ERROR!(L%d %s) Sound filename exceeded limit of 13 characters.\n",line_number,compilefile);
                    error++;
                    while( *textptr != ' ' && *textptr != 0 ) textptr++;        // JBF 20040127: end of file checked
                    break;
                }
            }
            sounds[k][i] = '\0';

            transnum(LABEL_DEFINE);
            soundps[k] = *(scriptptr-1);
            scriptptr--;
            transnum(LABEL_DEFINE);
            soundpe[k] = *(scriptptr-1);
            scriptptr--;
            transnum(LABEL_DEFINE);
            soundpr[k] = *(scriptptr-1);
            scriptptr--;
            transnum(LABEL_DEFINE);
            soundm[k] = *(scriptptr-1);
            scriptptr--;
            transnum(LABEL_DEFINE);
            soundvo[k] = *(scriptptr-1);
            scriptptr--;
            return 0;

        case 4:     //enda
            if( parsing_actor == 0 )
            {
                buildprintf("  * ERROR!(L%d %s) Found 'enda' without defining 'actor'.\n",line_number,compilefile);
                error++;
            }
//            else
            {
                if( num_squigilly_brackets > 0 )
                {
                    buildprintf("  * ERROR!(L%d %s) Found more '{' than '}' before 'enda'.\n",line_number,compilefile);
                    error++;
                }
                parsing_actor = 0;
            }

            return 0;
        case 12:    //break
        case 16:    //fall
        case 84:    //tip
        case 22:    //killit
        case 36:    //resetactioncount
        case 38:    //pstomp
        case 42:    //resetplayer
        case 47:    //resetcount
        case 61:    //wackplayer
        case 66:    //operate
        case 83:    //respawnhitag
        case 95:    //getlastpal
        case 96:    //pkick
        case 97:    //mikesnd
        case 104:   //tossweapon
        case 106:   //nullop
            return 0;
        case 60:    //gamestartup
            {
                int params[30];

                scriptptr--;
                for(j = 0; j < 30; j++)
                    {
                    transnum(LABEL_DEFINE);
                    scriptptr--;
                    params[j] = *scriptptr;

                    if (j != 25) continue;

                    if (keyword() != -1) {
                        buildprintf("Looks like Standard CON files.\n");
                        break;
                    } else {
                        conversion = 14;
                        buildprintf("Looks like Atomic Edition CON files.\n");
                    }
                }
                
                /*
                v1.3d               v1.5
                DEFAULTVISIBILITY   DEFAULTVISIBILITY
                GENERICIMPACTDAMAGE GENERICIMPACTDAMAGE
                MAXPLAYERHEALTH     MAXPLAYERHEALTH
                STARTARMORHEALTH    STARTARMORHEALTH
                RESPAWNACTORTIME    RESPAWNACTORTIME
                RESPAWNITEMTIME     RESPAWNITEMTIME
                RUNNINGSPEED        RUNNINGSPEED
                RPGBLASTRADIUS      GRAVITATIONALCONSTANT
                PIPEBOMBRADIUS      RPGBLASTRADIUS
                SHRINKERBLASTRADIUS PIPEBOMBRADIUS
                TRIPBOMBBLASTRADIUS SHRINKERBLASTRADIUS
                MORTERBLASTRADIUS   TRIPBOMBBLASTRADIUS
                BOUNCEMINEBLASTRADIUS   MORTERBLASTRADIUS
                SEENINEBLASTRADIUS  BOUNCEMINEBLASTRADIUS
                MAXPISTOLAMMO       SEENINEBLASTRADIUS
                MAXSHOTGUNAMMO      MAXPISTOLAMMO
                MAXCHAINGUNAMMO     MAXSHOTGUNAMMO
                MAXRPGAMMO          MAXCHAINGUNAMMO
                MAXHANDBOMBAMMO     MAXRPGAMMO
                MAXSHRINKERAMMO     MAXHANDBOMBAMMO
                MAXDEVISTATORAMMO   MAXSHRINKERAMMO
                MAXTRIPBOMBAMMO     MAXDEVISTATORAMMO
                MAXFREEZEAMMO       MAXTRIPBOMBAMMO
                CAMERASDESTRUCTABLE MAXFREEZEAMMO
                NUMFREEZEBOUNCES    MAXGROWAMMO
                FREEZERHURTOWNER    CAMERASDESTRUCTABLE
                                    NUMFREEZEBOUNCES
                                    FREEZERHURTOWNER
                                    QSIZE
                                    TRIPBOMBLASERMODE
                */

                j = 0;
                ud.const_visibility = params[j++];
                impact_damage = params[j++];
                max_player_health = params[j++];
                max_armour_amount = params[j++];
                respawnactortime = params[j++];
                respawnitemtime = params[j++];
                dukefriction = params[j++];
                if (conversion == 14) gc = params[j++];
                rpgblastradius = params[j++];
                pipebombblastradius = params[j++];
                shrinkerblastradius = params[j++];
                tripbombblastradius = params[j++];
                morterblastradius = params[j++];
                bouncemineblastradius = params[j++];
                seenineblastradius = params[j++];
                max_ammo_amount[PISTOL_WEAPON] = params[j++];
                max_ammo_amount[SHOTGUN_WEAPON] = params[j++];
                max_ammo_amount[CHAINGUN_WEAPON] = params[j++];
                max_ammo_amount[RPG_WEAPON] = params[j++];
                max_ammo_amount[HANDBOMB_WEAPON] = params[j++];
                max_ammo_amount[SHRINKER_WEAPON] = params[j++];
                max_ammo_amount[DEVISTATOR_WEAPON] = params[j++];
                max_ammo_amount[TRIPBOMB_WEAPON] = params[j++];
                max_ammo_amount[FREEZE_WEAPON] = params[j++];
                if (conversion == 14) max_ammo_amount[GROW_WEAPON] = params[j++];
                camerashitable = params[j++];
                numfreezebounces = params[j++];
                freezerhurtowner = params[j++];
                if (conversion == 14) {
                    spriteqamount = params[j++];
                    if(spriteqamount > 1024) spriteqamount = 1024;
                    else if(spriteqamount < 0) spriteqamount = 0;
                    lasermode = params[j++];
                }
            }
            return 0;
    }
    return 0;
}


void passone(void)
{

    while( parsecommand() == 0 );

    if( (error+warning) > 12)
        buildprintf(  "  * ERROR! Too many warnings or errors.\n");

}

static const char *defaultcons[3] =
{
     "GAME.CON",
     "USER.CON",
     "DEFS.CON"
};

void copydefaultcons(void)
{
    int i, fs, fpi;
    FILE *fpo;

    for(i=0;i<3;i++)
    {
        fpi = kopen4load( defaultcons[i] , 1 );
        if (fpi < 0) continue;
    
        fpo = fopen( defaultcons[i],"wb");
        if (fpo == NULL) {
            kclose(fpi);
            continue;
        }

        fs = kfilelength(fpi);

        kread(fpi,&hittype[0],fs);
        fwrite(&hittype[0],fs,1,fpo);

        kclose(fpi);
        fclose(fpo);
    }
}

void loadefs(const char *filenam)
{
    char *mptr;
    int i;
    int fs,fp;

    fp = kopen4load(filenam,loadfromgrouponly);
    if( fp == -1 )
    {
        if( loadfromgrouponly == 1 )
            gameexit("\nMissing con file(s).");
        else {
            char msg[255];
            const char *msgfmt = "CON file \"%s\" was not found.\n\n"
                "Check that the \"%s\" file is in the JFDuke3D directory "
                "and try running the game again.";
            snprintf(msg, sizeof(msg), msgfmt, filenam, duke3dgrp);
            gameexit(msg);
            return;
        }
        
        //loadfromgrouponly = 1;
        return; //Not there
    }
    
    fs = kfilelength(fp);

    buildprintf("Compiling: %s (%d bytes)\n",filenam,fs);

    mptr = Bmalloc(fs+1);
    if (!mptr) {
        buildprintf("Failed allocating %d byte CON text buffer.\n", fs+1);
        gameexit("Failed allocating memory for CON file.");
    }
    mptr[fs] = 0;
        
    textptr = mptr;
    
    kread(fp,textptr,fs);
    kclose(fp);

    //textptr[fs - 2] = 0;

    clearbuf(actorscrptr,MAXTILES,0L);  // JBF 20040531: MAXSPRITES? I think Todd meant MAXTILES...
    clearbufbyte(actortype,MAXTILES,0L);
    clearbufbyte(script,sizeof(script),0l); // JBF 20040531: yes? no?

    labelcnt = 0;
    scriptptr = script+1;
    warning = 0;
    error = 0;
    line_number = 1;
    total_lines = 0;

    strcpy(compilefile, filenam);   // JBF 20031130: Store currently compiling file name
    passone(); //Tokenize
    *script = encodescriptptr(scriptptr);
