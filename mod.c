
#include <stdio.h>

// IMPL

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "macros.h"


#define TRACKS 4




typedef struct FILE_INSTR {
  char     name[22];
  uint16_t len;
  int8_t   finetune;
  int8_t   volume;
  uint16_t loop_start;
  uint16_t loop_len;
} FILE_INSTR; // 22+2+1+1+2+2=30



typedef union FILE_NOTE {
//  ordine nel file:
//      0        1        2        3
//  IIIIPPPP:pppppppp:iiiiFFFF:aaaaaaaa
//      IIIIiiii ( 8 bits) : instrument
//  PPPPpppppppp (12 bits) : period
//          FFFF ( 4 bits) : effect 
//      aaaaaaaa ( 8 bits) : argument
  uint8_t u8[4];
  struct { uint8_t v; } IIIIPPPP;
  struct { uint8_t pad[1]; uint8_t v; } pppppppp;
  struct { uint8_t pad[2]; uint8_t v; } iiiiFFFF;
  struct { uint8_t pad[3]; uint8_t v; } aaaaaaaa;
} FILE_NOTE;  // 4

#define INSTRUMENT(N) ((0x0FF&((int)N.IIIIPPPP.v&0xF0)|(N.iiiiFFFF.v>>4)))
#define PERIOD(N)     ((0xFFF&((int)N.IIIIPPPP.v<<8)|N.pppppppp.v))
#define EFFECT(N)     (0xF&N.iiiiFFFF.v)
#define FXPARAM(N)    (N.aaaaaaaa.v)


typedef struct FILE_LINE {
  FILE_NOTE track[TRACKS];
} FILE_LINE;  // 4*4= 16



typedef struct FILE_PATTERN {
  FILE_LINE line[64];
} FILE_PATTERN; // 16*64= 1024




typedef struct INSTR {
  // rappresentazione interna
  char   name[22+1];
  int    len;
  int8_t finetune;
  int8_t volume;
  int    loop_end;
  int    loop_len;
  int8_t *wavetable;  // pcm 8bit signed
} INSTR;


typedef struct MOD {
  // rappresentazione interna
  char songname[20+1];
  INSTR instr[31];
  int instr_count;
  int song_len;
  int song_end;
  uint8_t sequence[128];
  int patt_count;
  FILE_PATTERN *pattern;
} MOD;

static MOD mod;









typedef struct CURRENT {
  int tick;
  int speed;
  int line;
  int seqpos;
  int pattern;
  int sample_rate;
  int samples_per_tick;
  int samples;
  int bpm;
  long frame;
  char running:1;
} CURRENT;

volatile CURRENT current;



typedef struct VOICE {
  int8_t *wavetable;  // 8bit pcm signed
  int8_t volume;  // 0..64
  int32_t index;
  int32_t incr;
  int32_t loop_end;
  int32_t loop_len;
  int effect;
  int fxparam;
} VOICE;

volatile VOICE voice[4];







void endian( void *arr , int size ){
  // inplace byte array reverse
  // abcde → edcba
  uint8_t *a = (uint8_t *)arr;
  uint8_t *b;
  assert(arr);
  assert(size>0);
  b = a+size-1;
  for(;a<b;a++,b--){
    uint8_t t = *a;
    *a = *b;
    *b = t;
  }
}

int read_u8( FILE *f ){
  uint8_t v;
  assert(1==fread(&v,sizeof(v),1,f));
  return v;
}


//void print_note_from_period( int p ){
//  // 0x1ac = 428 = C3
//  static char *notes[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
//  int note   = pow(p,1.0/12);
//  int octave = note / 12;
//  printf("%3d%3d",note,octave);
//}



//#include "mod_dump.c"
#include "mod_import.c"



void advance(){
  // regole di avanzamento di default
  current.tick++;
  if( current.tick < current.speed ) return;
  current.tick = 0;

  current.line++;
  if( current.line < 64 ) return;
  current.line = 0;

  current.seqpos++;
  if( current.seqpos < mod.song_len ) return;
  current.seqpos = 0;
}



static void tick(){
  // eseguita ad ogni tick

  current.pattern = mod.sequence[current.seqpos];

  if(current.tick==0){
    const char *sep = "\n";
    int track;
    for( track=0 ; track<TRACKS ; track++ ){
      // al tick 0 si parsano le note

      FILE_NOTE n = mod.pattern[current.pattern].line[current.line].track[track];
      int instrument = INSTRUMENT(n);
      int period     = PERIOD(n);
      voice[track].effect  = EFFECT(n);
      voice[track].fxparam = FXPARAM(n);

      printf("%s%4d %02d %x%02x"
        ,sep
        ,period
        ,instrument
        ,voice[track].effect
        ,voice[track].fxparam
      );
      sep = "   ";

      if(period){
        // nmero magico dai docs e dal confronto con FT2
        voice[track].incr = 3579545.25*65536/current.sample_rate/period;

//          printf("tr %d period %d incr %d\n"
//            ,track
//            ,period
//            ,voice[track].incr
//          );
      }

      if(instrument){
        INSTR *I = &mod.instr[instrument-1];
        voice[track].index     = 0; // retrig
        voice[track].wavetable = I->wavetable;
        voice[track].volume    = I->volume;
        if(!voice[track].volume) voice[track].volume = 64; // default
        voice[track].loop_len  = I->loop_len*65536;
        voice[track].loop_end  = I->loop_end*65536;
//        printf("tr %d in %d vo %d ll %d le %d name %s\n"
//          ,track
//          ,instrument
//          ,I->volume
//          ,voice[track].loop_len
//          ,voice[track].loop_end
//          ,I->name
//        );
      }

      // effetti gestiti solo al tick 0
      switch(voice[track].effect){
        case 0xC : // volume
          voice[track].volume = voice[track].fxparam;
        break;
        case 0xF : // set speed
          if(voice[track].fxparam<32) current.speed = voice[track].fxparam;
          else                        current.bpm   = voice[track].fxparam;
        break;
      }
    } // for track
  } // tick 0
  else
  {
    const char *sep = "\n";
    int track;
    for( track=0 ; track<TRACKS ; track++ ){

      // effetti gestiti nel tick !=0
      switch(voice[track].effect){
        case 0x0 : // arpeggio
        break;
        case 0x1 : // portamento up
        break;
        case 0x2 : // portamento down
        break;
        case 0x3 : // portamento to note
        break;
        case 0x4 : // vibrato
        break;
        case 0x5 : // portamento to note + volume slide
        break;
        case 0x6 : // vibrato + volume slide
        break;
        case 0x7 : // tremolo
        break;
        case 0x8 : // pan
        break;
        case 0x9 : // sample offset
        break;
        case 0xA : // volume slide
        break;
        case 0xB : // position jump
        break;
        case 0xD : // pattern break
        break;
        case 0xE : // extended fx
        break;
      }
    } // for track
  }

//  printf("frame %9ld seq %2d patt %2d line %2d tick %d/%d\n"
//    ,current.frame
//    ,current.seqpos
//    ,current.pattern
//    ,current.line
//    ,current.tick
//    ,current.speed
//  );

}

// ENDIMPL





void mod_play( int sample_rate )
// IMPL
{

  current.running=0;

  memset((void*)&current,0,sizeof(current));
  memset((void*)&voice,0,sizeof(voice));

  current.speed = 6;    // default
  current.bpm   = 125;  // default
  current.frame = 0;
  current.sample_rate = sample_rate;
  // nmero magico dai docs e dal confronto con FT2
  current.samples_per_tick = 2.5*current.sample_rate/current.bpm;
  tick();

//  {
//    INSTR *I = &mod.instr[7];
//    int tr;
//    for( tr=0 ; tr<TRACKS ; tr++ ){
//      voice[tr].index     = 0;
//      voice[tr].incr      = 64;
//      voice[tr].wavetable = I->wavetable;
//      voice[tr].volume    = 64;
//      voice[tr].loop_len  = I->loop_len*65536;
//      voice[tr].loop_end  = I->loop_end*65536;
//    }
//  }

  current.running=1;
}
// ENDIMPL



void mod_main( int sample_rate )
// IMPL
{
//  FILE *f = fopen("echo15.mod","rb");
  FILE *f = fopen("echo31.mod","rb");
//  FILE *f = fopen("test.mod","rb");
  assert(f);
  mod_import(f);
  fclose(f);
  mod_play(sample_rate);
  getchar();
}



//void dump_voice()
//{
//  int track;
//  for( track=0 ; track<TRACKS ; track++ ){
//    printf("tr %d i %d di %d le %d ll %d\n"
//      ,track
//      ,voice[track].index
//      ,voice[track].incr
//      ,voice[track].loop_end
//      ,voice[track].loop_len
//    );
//  }
//  printf("\n");
//}






int voice_sample( int trk ){
  volatile VOICE *V = &voice[trk];
  if(!V->wavetable)return 0;
  int i = V->index/65536;
  int s = (int)V->wavetable[i] * (int)V->volume;   // -128*64= -8192   +127*64= 8128
//  int s = (rand()&255) * (int)V->volume;   // -128*64= -8192   +127*64= 8128
  if(!V->incr)return s;
  V->index += V->incr;
  if( V->index < V->loop_end ) return s;
  // qui V->index >= V->loop_end, index potenzialmente non valido
  if( V->loop_len ){
    // loop - index è di nuovo valido
    V->index -= V->loop_len;
  } else {
    // no loop - bisogna fermarsi e tornare ad un passo prima
    V->incr = 0;
    V->index = i*65536;
  }
  return s;
}




// ENDIMPL




int mod_sample()
// IMPL
{
  if(!current.running)return 0;

  int32_t mix = 0;
  mix = (int32_t)voice_sample(0)+  
        (int32_t)voice_sample(1)+
        (int32_t)voice_sample(2)+
        (int32_t)voice_sample(3);  // ±128*64*4 = ±32768

  // clamp
  if(mix<-32767) mix=-32767; else
  if(mix>+32767) mix=+32767;

  current.frame ++;

  // vediamo se è il caso di passare al prossimo tick
  current.samples++;            
  if( current.samples < current.samples_per_tick ) return mix;
  current.samples = 0;

  // prossimo tick
  advance();
  tick();

  return mix;
}
// ENDIMPL








