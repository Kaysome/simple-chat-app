//-------------------------------------------------------------------------
/*
 Copyright (C) 2013-2021 Jonathon Fowler <jf@jonof.id.au>

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

#import <Cocoa/Cocoa.h>

#include "compat.h"
#include "baselayer.h"
#include "build.h"
#include "grpscan.h"
#include "startwin.h"

#import "GameListSource.h"

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

#include <stdlib.h>

// Callbacks to glue the C in grpscan.c with the Objective-C here.
static void importmeta_progress(void *data, const char *path);
static int importmeta_cancelled(void *data);

@interface StartupWinController : NSWindowController <NSWindowDelegate>
{
    BOOL quiteventonclose;
    GameListSource *gamelistsrc;
    NSThread *importthread;
    BOOL inmodal;
    struct startwin_settings *settings;

    IBOutlet NSButton *alwaysShowButton;
    IBOutlet NSButton *fullscreenButton;
    IBOutlet NSTextView *messagesView;
    IBOutlet NSTabView *tabView;
    IBOutlet NSTabViewItem *tabConfig;
    IBOutlet NSTabViewItem *tabMessages;
    IBOutlet NSPopUpButton *videoMode3DPUButton;

    IBOutlet NSPopUpButton *soundQualityPUButton;
    IBOutlet NSButton *useMouseButton;
    IBOutlet NSButton *useJoystickButton;

    IBOutlet NSButton *singlePlayerButton;
    IBOutlet NSButton *joinMultiButton;
    IBOutlet NSTextField *hostField;
    IBOutlet NSButton *hostMultiButton;
    IBOutlet NSTextField *numPlayersField;
    IBOutlet NSStepper *numPlayersStepper;

    IBOutlet NSTabViewItem *tabGame;
    IBOutlet NSScrollView *gameList;

    IBOutlet NSButton *chooseImportButton;
    IBOutlet NSButton *importInfoButton;

    IBOutlet NSButton *cancelButton;
    IBOutlet NSButton *startButton;

    IBOutlet NSWindow *importStatusWindow;
    IBOutlet NSTextField *importStatusText;
    IBOutlet NSButton *importStatusCancel;
}

- (int)modalRun;
- (void)closeQuietly;
- (void)populateVideoModes:(BOOL)firstTime;
- (void)populateSoundQuality:(BOOL)firstTime;

- (IBAction)fullscreenClicked:(id)sender;

- (IBAction)multiPlayerModeClicked:(id)sender;

- (IBAction)chooseImportClicked:(id)sender;
- (IBAction)importInfoClicked:(id)sender;
- (IBAction)importStatusCancelClicked:(id)sender;
- (void)updateImportStatusText:(NSString *)text;
- (void)doImport:(NSString *)path;
- (void)doneImport:(NSNumber *)result;
- (BOOL)isImportCancelled;

- (IBAction)cancel:(id)sender;
- (IBAction)start:(id)se