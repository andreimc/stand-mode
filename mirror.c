/*
 *  mirror.m
 *  mirror
 *
 *  Created by Fabian Canas on 2/4/09.
 *  Copyright 2009-2015 Fabián Cañas. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#import <ApplicationServices/ApplicationServices.h>

#define MAX_DISPLAYS 10
#define SECONDARY_DISPLAY_COUNT 9

const CFStringRef kDisplayBrightness = CFSTR(kIODisplayBrightnessKey);

static CGDisplayCount numberOfTotalDspys = MAX_DISPLAYS;

static CGDirectDisplayID activeDspys[MAX_DISPLAYS];
static CGDirectDisplayID onlineDspys[MAX_DISPLAYS];
static CGDirectDisplayID secondaryDspys[SECONDARY_DISPLAY_COUNT];

int main (int argc, const char * argv[]) {
    
    enum { off, on, toggle, query } action = toggle;

    CGDisplayCount numberOfActiveDspys;
    CGDisplayCount numberOfOnlineDspys;
    
    CGDisplayErr activeError = CGGetActiveDisplayList (numberOfTotalDspys,activeDspys,&numberOfActiveDspys);
    
    if (activeError!=0) fprintf(stderr, "Error in obtaining active diplay list: %d\n",activeError);
    
    CGDisplayErr onlineError = CGGetOnlineDisplayList (numberOfTotalDspys,onlineDspys,&numberOfOnlineDspys);
    
    if (onlineError!=0) fprintf(stderr, "Error in obtaining online diplay list: %d\n",onlineError);
    
    if (numberOfOnlineDspys<2) {
        printf("No secondary display detected.\n");
        return 1;
    }
    
    bool displaysMirrored = CGDisplayIsInMirrorSet(CGMainDisplayID());
    int secondaryDisplayIndex = 0;
    for (int displayIndex = 0; displayIndex<numberOfOnlineDspys; displayIndex++) {
        fprintf(stderr, "All displays: %d \n", onlineDspys[displayIndex]);
        if (onlineDspys[displayIndex] != CGMainDisplayID()) {
            fprintf(stderr, "Index displays: %d \n", onlineDspys[displayIndex]);
            secondaryDspys[secondaryDisplayIndex] = onlineDspys[displayIndex];
            secondaryDisplayIndex++;
        }
    }
    
    CGDisplayConfigRef configRef;
    CGError err = CGBeginDisplayConfiguration (&configRef);
    if (err != 0) fprintf(stderr, "Error with CGBeginDisplayConfiguration: %d\n",err);
    
    if (action == toggle) {
        if (displaysMirrored) {
            action = off;
        } else {
            action = on;
        }
    }
    
    switch (action) {
        case on:
            printf("ON\n");
            system("./brightness -d 0 -v 0.0");
            CGConfigureDisplayMirrorOfDisplay(configRef, CGMainDisplayID(), secondaryDspys[0]);
            break;
        case off:
            printf("OFF\n");
            system("./brightness -d 1 -v 0.7");
            CGConfigureDisplayMirrorOfDisplay(configRef, secondaryDspys[0], kCGNullDirectDisplay);
            break;
        case query:
            if (displaysMirrored) { // Displays are unmirrored
                printf("on\n");
            } else { // Displays are mirrored
                printf("off\n");
            }
            break;
        default:
            break;
    }
    if (err != 0) fprintf(stderr, "Error configuring displays: %d\n",err);
    
    // Apply the changes
    err = CGCompleteDisplayConfiguration (configRef,kCGConfigurePermanently);
    if (err != 0) fprintf(stderr, "Error with applying configuration: %d\n",err);

    return err;
}