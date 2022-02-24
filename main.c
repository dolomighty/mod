
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "mod.h"
#include "macros.h"





void audio_cb ( void *userdata , Uint8 *stream , int len_bytes ){
  int16_t *frame = (int16_t*)stream ;
  int len_frames = len_bytes / sizeof(*frame);
  for ( ; len_frames > 0 ; len_frames -- , frame ++ ){
    *frame = mod_sample();
  }
}


int main( int argc , char *argv[] ){
  
  if( 0 != SDL_Init( SDL_INIT_AUDIO )) FATAL("SDL_Init: %s\n",SDL_GetError());
  atexit(SDL_Quit);
    
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;

  SDL_zero(want);
  want.freq     = 48000;
  want.format   = AUDIO_S16;
  want.channels = 1;
  want.samples  = 256;
  want.callback = audio_cb;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0 );
  if( ! dev ){
    printf("Failed to open audio: %s\n", SDL_GetError());
    return 1 ;
  }

  SDL_PauseAudioDevice(dev, 0);  // start audio playing.

//  mod_play("echo15.mod",have.freq);
//  mod_play("echo31.mod",have.freq);
//  mod_play("test.mod",have.freq);
  mod_play("../AEIOU.mod",have.freq);
  getchar();
  
  SDL_CloseAudioDevice(dev);

  return 0 ;
}


