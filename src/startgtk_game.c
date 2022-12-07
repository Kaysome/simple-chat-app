
//-------------------------------------------------------------------------
/*
 Copyright (C) 2007-2021 Jonathon Fowler <jf@jonof.id.au>

 This file is part of JFDuke3D

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
 */
//-------------------------------------------------------------------------

#include "compat.h"

#include <gtk/gtk.h>

#include "baselayer.h"
#include "build.h"
#include "startwin.h"
#include "grpscan.h"

#define TAB_CONFIG 0
#define TAB_GAME 1
#define TAB_MESSAGES 2

static struct soundQuality_t {
    int frequency;
    int samplesize;
    int channels;
} soundQualities[] = {
    { 44100, 16, 2 },
    { 22050, 16, 2 },
    { 11025, 16, 2 },
    { 0, 0, 0 },    // May be overwritten by custom sound settings.
    { 0, 0, 0 },
};

static GtkWindow *startwin;
static struct {
    GtkWidget *startbutton;
    GtkWidget *cancelbutton;

    GtkWidget *tabs;
    GtkWidget *configbox;
    GtkWidget *alwaysshowcheck;

    GtkWidget *messagestext;

    GtkWidget *vmode3dcombo;
    GtkListStore *vmode3dlist;
    GtkWidget *fullscreencheck;

    GtkWidget *usemousecheck;
    GtkWidget *usejoystickcheck;
    GtkWidget *soundqualitycombo;
    GtkListStore *soundqualitylist;

    GtkWidget *singleplayerbutton;
    GtkWidget *joinmultibutton;
    GtkWidget *hostmultibutton;
    GtkWidget *hostfield;
    GtkWidget *numplayersspin;
    GtkAdjustment *numplayersadjustment;

    GtkWidget *gametable;
    GtkListStore *gamelist;

    GtkWidget *chooseimportbutton;
    GtkWidget *importinfobutton;

    GtkWindow *importstatuswindow;
    GtkWidget *importstatustext;
    GtkWidget *importstatuscancelbutton;
} controls;

static gboolean startwinloop = FALSE;
static struct startwin_settings *settings;
static gboolean quiteventonclose = FALSE;
static int retval = -1;


extern int gtkenabled;

// -- SUPPORT FUNCTIONS -------------------------------------------------------

static GObject * get_and_connect_signal(GtkBuilder *builder, const char *name, const char *signal_name, GCallback handler)
{
    GObject *object;

    object = gtk_builder_get_object(builder, name);
    if (!object) {
        buildprintf("gtk_builder_get_object: %s not found\n", name);
        return 0;
    }
    g_signal_connect(object, signal_name, handler, NULL);
    return object;
}

static void foreach_gtk_widget_set_sensitive(GtkWidget *widget, gpointer data)
{
    gtk_widget_set_sensitive(widget, (gboolean)(intptr_t)data);
}

static void populate_video_modes(gboolean firsttime)
{
    int i, mode3d = -1;
    int xdim = 0, ydim = 0, bpp = 0, fullscreen = 0;
    char modestr[64];
    int cd[] = { 32, 24, 16, 15, 8, 0 };
    GtkTreeIter iter;

    if (firsttime) {
        getvalidmodes();
        xdim = settings->xdim3d;
        ydim = settings->ydim3d;
        bpp  = settings->bpp3d;
        fullscreen = settings->fullscreen;
    } else {
        // Read back the current resolution information selected in the combobox.
        fullscreen = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(controls.fullscreencheck));
        if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(controls.vmode3dcombo), &iter)) {
            gtk_tree_model_get(GTK_TREE_MODEL(controls.vmode3dlist), &iter, 1 /*index*/, &mode3d, -1);
        }
        if (mode3d >= 0) {
            xdim = validmode[mode3d].xdim;
            ydim = validmode[mode3d].ydim;
            bpp = validmode[mode3d].bpp;
        }
    }

    // Find an ideal match.
    mode3d = checkvideomode(&xdim, &ydim, bpp, fullscreen, 1);
    if (mode3d < 0) {
        for (i=0; cd[i]; ) { if (cd[i] >= bpp) i++; else break; }
        for ( ; cd[i]; i++) {
            mode3d = checkvideomode(&xdim, &ydim, cd[i], fullscreen, 1);
            if (mode3d < 0) continue;
            break;
        }
    }

    // Repopulate the list.
    gtk_list_store_clear(controls.vmode3dlist);
    for (i = 0; i < validmodecnt; i++) {
        if (validmode[i].fs != fullscreen) continue;

        sprintf(modestr, "%d \xc3\x97 %d %d-bpp",
            validmode[i].xdim, validmode[i].ydim, validmode[i].bpp);
        gtk_list_store_insert_with_values(controls.vmode3dlist,
            &iter, -1,
            0, modestr, 1, i, -1);
        if (i == mode3d) {
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(controls.vmode3dcombo), &iter);
        }
    }
}

static void populate_sound_quality(gboolean firsttime)
{
    int i, curidx = -1;
    char modestr[64];
    GtkTreeIter iter;

    if (firsttime) {
        for (i = 0; soundQualities[i].frequency > 0; i++) {
            if (soundQualities[i].frequency == settings->samplerate &&
                soundQualities[i].samplesize == settings->bitspersample &&
                soundQualities[i].channels == settings->channels) {
                curidx = i;
                break;
            }
        }
        if (curidx < 0) {
            soundQualities[i].frequency = settings->samplerate;
            soundQualities[i].samplesize = settings->bitspersample;
            soundQualities[i].channels = settings->channels;
        }
    }

    gtk_list_store_clear(controls.soundqualitylist);
    for (i = 0; soundQualities[i].frequency > 0; i++) {
        sprintf(modestr, "%d kHz, %d-bit, %s",
            soundQualities[i].frequency / 1000,
            soundQualities[i].samplesize,
            soundQualities[i].channels == 1 ? "Mono" : "Stereo");
        gtk_list_store_insert_with_values(controls.soundqualitylist,
            &iter, -1,
            0, modestr, 1, i, -1);
        if (i == curidx) {
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(controls.soundqualitycombo), &iter);
        }
    }
}

static void populate_game_list(gboolean firsttime)
{
    struct grpfile const *fg;
    GtkTreeIter iter;
    GtkTreeSelection *sel;

    (void)firsttime;

    gtk_list_store_clear(controls.gamelist);

    for (fg = GroupsFound(); fg; fg = fg->next) {
        if (!fg->ref) continue;
        gtk_list_store_insert_with_values(controls.gamelist,
            &iter, -1,
            0, fg->ref->name, 1, fg->name, 2, fg, -1);
        if (fg == settings->selectedgrp) {
            sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(controls.gametable));
            gtk_tree_selection_select_iter(sel, &iter);
        }
    }
}

static void set_settings(struct startwin_settings *thesettings)
{
    settings = thesettings;
}

static void setup_config_mode(void)
{
    if (!settings->selectedgrp) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(controls.tabs), TAB_GAME);
    } else {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(controls.tabs), TAB_CONFIG);
    }

    // Enable all the controls on the Configuration page.
    gtk_container_foreach(GTK_CONTAINER(controls.configbox),
            foreach_gtk_widget_set_sensitive, (gpointer)TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.alwaysshowcheck), settings->forcesetup);
    gtk_widget_set_sensitive(controls.alwaysshowcheck, TRUE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.fullscreencheck), settings->fullscreen);
    gtk_widget_set_sensitive(controls.fullscreencheck, TRUE);

    populate_video_modes(TRUE);
    gtk_widget_set_sensitive(controls.vmode3dcombo, TRUE);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.usemousecheck), settings->usemouse);
    gtk_widget_set_sensitive(controls.usemousecheck, TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.usejoystickcheck), settings->usejoy);
    gtk_widget_set_sensitive(controls.usejoystickcheck, TRUE);

    populate_sound_quality(TRUE);
    gtk_widget_set_sensitive(controls.soundqualitycombo, TRUE);

    if (!settings->netoverride) {
        gtk_widget_set_sensitive(controls.singleplayerbutton, TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controls.singleplayerbutton), TRUE);

        gtk_widget_set_sensitive(controls.joinmultibutton, TRUE);
        gtk_widget_set_sensitive(controls.hostfield, FALSE);

        gtk_widget_set_sensitive(controls.hostmultibutton, TRUE);
        gtk_widget_set_sensitive(controls.numplayersspin, FALSE);
        gtk_spin_button_set_range(GTK_SPIN_BUTTON(controls.numplayersspin), 2, MAXPLAYERS);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(controls.numplayersspin), 2.0);
    } else {
        gtk_widget_set_sensitive(controls.singleplayerbutton, FALSE);
        gtk_widget_set_sensitive(controls.joinmultibutton, FALSE);
        gtk_widget_set_sensitive(controls.hostfield, FALSE);
        gtk_widget_set_sensitive(controls.hostmultibutton, FALSE);
        gtk_widget_set_sensitive(controls.numplayersspin, FALSE);
    }

    populate_game_list(TRUE);
    gtk_widget_set_sensitive(controls.gametable, TRUE);
    gtk_widget_set_sensitive(controls.chooseimportbutton, TRUE);
    gtk_widget_set_sensitive(controls.importinfobutton, TRUE);

    gtk_widget_set_sensitive(controls.cancelbutton, TRUE);
    gtk_widget_set_sensitive(controls.startbutton, TRUE);
}

static void setup_messages_mode(gboolean allowcancel)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(controls.tabs), TAB_MESSAGES);

    // Disable all the controls on the Configuration page.
    gtk_container_foreach(GTK_CONTAINER(controls.configbox),
            foreach_gtk_widget_set_sensitive, (gpointer)FALSE);
    gtk_widget_set_sensitive(controls.alwaysshowcheck, FALSE);

    gtk_widget_set_sensitive(controls.gametable, FALSE);
    gtk_widget_set_sensitive(controls.chooseimportbutton, FALSE);
    gtk_widget_set_sensitive(controls.importinfobutton, FALSE);