/*
 * main.cpp
 *
 *  Created on: Feb 12, 2013
 *      Author: Francisco Tufro <info@franciscotufro.com>
 */

#include <aku/AKU.h>
#include <aku/AKU-untz.h>
#include <aku/AKU-luaext.h>

#include <lua-headers/moai_lua.h>

#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <math.h>

#include "bbutil.h"

static screen_context_t screen_cxt;

// AKU Variables and Constants
AKUContextID aku_context_id;

namespace BB10InputDeviceID {
	enum {
		DEVICE,
		TOTAL,
	};
}

namespace BB10InputDeviceSensorID {
	enum {
		TOUCH,
		TOTAL,
	};
}

void enqueueTouch ( screen_event_t event, bool down) {
	int position[2], touch;

	screen_get_event_property_iv(event, SCREEN_PROPERTY_SOURCE_POSITION, position);
	screen_get_event_property_iv(event, SCREEN_PROPERTY_TOUCH_ID, &touch);

	AKUEnqueueTouchEvent (
		BB10InputDeviceID::DEVICE,
		BB10InputDeviceSensorID::TOUCH,
		touch, // use the address of the touch as a unique id
		down,
		position[0],
		position[1]
	);

}

void handleScreenEvent(bps_event_t *event) {
    screen_event_t screen_event = screen_event_get_event(event);

    int screen_val;
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
    	enqueueTouch ( screen_event, true);
    	break;
    case SCREEN_EVENT_MTOUCH_MOVE:
    	enqueueTouch ( screen_event, true);
    	break;
    case SCREEN_EVENT_MTOUCH_RELEASE:
    	enqueueTouch ( screen_event, false);
        break;
    }
}

int initialize() {
    //Query width and height of the window surface created by utility code
    EGLint surface_width, surface_height;

    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);

    // GL Output Console


    //Initialize AKU
    aku_context_id = AKUCreateContext ();

  // AKUUntzInit ();
  // AKUExtLoadLuacrypto ();
  // AKUExtLoadLuacurl ();
  // AKUExtLoadLuafilesystem ();
  // AKUExtLoadLuasocket ();
  // AKUExtLoadLuasql ();

	AKUSetInputConfigurationName ( "BlackBerry 10" );

	// Initialize input devices
	AKUReserveInputDevices			( BB10InputDeviceID::TOTAL );
	AKUSetInputDevice				( BB10InputDeviceID::DEVICE, "device" );

	AKUReserveInputDeviceSensors	( BB10InputDeviceID::DEVICE, BB10InputDeviceSensorID::TOTAL );
	AKUSetInputDeviceTouch			( BB10InputDeviceID::DEVICE, BB10InputDeviceSensorID::TOUCH, "touch" );

	// Configure Screen
	AKUDetectGfxContext ();
	AKUSetScreenSize ( surface_width, surface_height );
	AKUSetViewSize ( surface_width, surface_height );

	// Initialize Moai LUA bytecode
	AKURunBytecode ( moai_lua, moai_lua_SIZE );
	AKUPause(false);

    return EXIT_SUCCESS;
}

void render() {
	AKURender ();
    bbutil_swap();
}

int main(int argc, char *argv[]) {
    int exit_application = 0;

    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_cxt, 0);

    //Initialize BPS library
    bps_initialize();

    //Use utility code to initialize EGL for rendering with GL ES 2.0
    if (EXIT_SUCCESS != bbutil_init_egl(screen_cxt)) {
        fprintf(stderr, "bbutil_init_egl failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        return 0;
    }

    //Initialize application logic
    if (EXIT_SUCCESS != initialize()) {
        fprintf(stderr, "initialize failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        bps_shutdown();
        return 0;
    }

    //Signal BPS library that navigator and screen events will be requested
    if (BPS_SUCCESS != screen_request_events(screen_cxt)) {
        fprintf(stderr, "screen_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        bps_shutdown();
        return 0;
    }

    if (BPS_SUCCESS != navigator_request_events(0)) {
        fprintf(stderr, "navigator_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        bps_shutdown();
        return 0;
    }

    //Signal BPS library that navigator orientation is not to be locked
    if (BPS_SUCCESS != navigator_rotation_lock(false)) {
        fprintf(stderr, "navigator_rotation_lock failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_cxt);
        bps_shutdown();
        return 0;
    }

    // Setup working directory
    #define MAX_LENGTH 256
    char fullpath[MAX_LENGTH];
    char appPath[MAX_LENGTH];
    getcwd(appPath, MAX_LENGTH);
    snprintf(fullpath, MAX_LENGTH, "%s/app/native/assets/", appPath);
    AKUSetWorkingDirectory(fullpath);

    // Run main script!
	AKURunScript ( "main.lua" );

	while (!exit_application) {
        //Request and process all available BPS events
        bps_event_t *event = NULL;

        for(;;) {
            if (BPS_SUCCESS != bps_get_event(&event, 0)) {
                fprintf(stderr, "bps_get_event failed\n");
                break;
            }

            if (event) {
                int domain = bps_event_get_domain(event);

                if (domain == screen_get_domain()) {
                    handleScreenEvent(event);
                } else if ((domain == navigator_get_domain())
                        && (NAVIGATOR_EXIT == bps_event_get_code(event))) {
                    exit_application = 1;
                }
            } else {
                break;
            }
        }

        // This is needed to update moai's output
        fflush( stdout );
        //fprintf(stdout, " UPDATE!\n");
        AKUUpdate ();

        render();
    }

    // Kill AKU
	AKUFinalize ();

    //Stop requesting events from libscreen
    screen_stop_events(screen_cxt);

    //Shut down BPS library for this process
    bps_shutdown();

    //Use utility code to terminate EGL setup
    bbutil_terminate();

    //Destroy libscreen context
    screen_destroy_context(screen_cxt);
    return 0;
}
