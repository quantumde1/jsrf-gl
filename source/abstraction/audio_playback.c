#include "../../include/abstraction.h"

#ifdef _arch_dreamcast


#include <stdio.h>

#include <kos/dbgio.h>

#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <dc/sound/stream.h>

#include <kos/init.h>
#include <kos.h>

#include <adx/adx.h> /* ADX Decoder Library */
#include <adx/snddrv.h> /* Direct Access to Sound Driver */

static void* audio_thread(void *filename) {
    while(1) {
        play_again:
        if( adx_dec( concat_strings(ASSETS_PATH, (char*)filename), 1 ) < 1 ) {
            writeln("Error: invalid ADX");
            return NULL;
        }
        while( snddrv.drv_status == SNDDRV_STATUS_NULL ) {
            thd_pass();
        }
        while (snddrv.drv_status != SNDDRV_STATUS_NULL ) {
            thd_sleep(50);
        }
        if (snddrv.drv_status == SNDDRV_STATUS_NULL) {
            writeln("audio loop");
            goto play_again;
        }
    }
    return NULL;
}

void audioPlayback(char* filename) {
    writeln("Init audio playback");
    thd_create(0, audio_thread, filename);
}

#endif