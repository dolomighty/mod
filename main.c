
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include "mod.h"





void audio_cb( void *userdata , Uint8 *stream , int len_bytes ){
    int16_t *frame = (int16_t*)stream;
    int len_frames = len_bytes / sizeof(*frame);
    for(; len_frames > 0; len_frames-=2 ){
        int l,r;
        mod_sample(&l,&r);
        *frame++ = l;
        *frame++ = r;
    }
}


int main( int argc , char *argv[] ){

    assert(SDL_Init( SDL_INIT_AUDIO )==0);
    atexit(SDL_Quit);
        
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;

    SDL_zero(want);
    want.freq     = 48000;
    want.format   = AUDIO_S16;
    want.channels = 2;
    want.samples  = 256;
    want.callback = audio_cb;

    assert( (dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0 )) != 0 );

    SDL_PauseAudioDevice(dev, 0);  // start audio playing.

    mod_play(argv[1],have.freq);
    getchar();
    
    SDL_CloseAudioDevice(dev);

    return 0;
}


