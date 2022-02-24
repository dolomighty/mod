


// http://lclevy.free.fr/mo3/mod.txt



static int array_is_ascii( uint8_t *a , int bytes )
{
    for(;bytes;bytes--,a++)
    {
//    printf("%02x ",*a);
        if(*a< 32)return 0;
        if(*a>126)return 0;
    }
    return 1;
}


static int guess_instr_count( FILE *f )
{
    // euristica 15/31 strumenti
    FILE_INSTR instr;
    long pos = ftell(f);
    uint8_t M_K_[4];
    fseek(f,0x438,SEEK_SET);  // puntiamo alla targa M.K. o simile
    assert(1==fread(M_K_,sizeof(M_K_),1,f));
    fseek(f,pos,SEEK_SET);  // undo fread'ing
    return array_is_ascii(M_K_,sizeof(M_K_)) ? 31 : 15;
}


static void hexdump( void *_a , int bytes )
{
    uint8_t *a = (uint8_t *)_a; // alias
    int o=0;
    for(;bytes;bytes--,a++,o++)
    {
        if(o%16==0) printf("%04x ",o);
        if(o%8==0) printf(" ");
        printf("%02x ",*a);
        if(o%16==15) printf("\n");
    }
    printf("\n");
}


void mod_import( FILE *f )
{
    int i;

    // importante che sian allineate bit per bit
    assert(sizeof(FILE_INSTR)==30);
    assert(sizeof(FILE_NOTE)==4);
    assert(sizeof(FILE_LINE)==16);
    assert(sizeof(FILE_PATTERN)==1024);

    memset(&mod,0,sizeof(mod));

    assert(1==fread(mod.songname,20,1,f));
//  printf("songname %s\n",mod.songname);

    mod.instr_count = guess_instr_count(f);
//  printf("mod.instr_count %d\n",mod.instr_count);
//  exit(1);

//  printf("instruments:\n");
    for( i=0 ; i<mod.instr_count ; i++ ){
        FILE_INSTR instr={};

        assert( 1 == fread(&instr,sizeof(instr),1,f));

        strncpy(mod.instr[i].name,instr.name,sizeof(mod.instr[i].name));

#define ENDIAN(ARR) endian(&ARR,sizeof(ARR))
        ENDIAN(instr.len);
        ENDIAN(instr.loop_start);
        ENDIAN(instr.loop_len);

//    printf("%2d: len %6d loop_start %6d loop_len %6d name %s (file)\n"
//      ,i+1
//      ,instr.len
//      ,instr.loop_start
//      ,instr.loop_len
//      ,instr.name
//    );

        mod.instr[i].volume = instr.volume;
        if(!mod.instr[i].volume) mod.instr[i].volume = 64;  // default

        mod.instr[i].len = instr.len*2;
        mod.instr[i].wavetable  = 0;

        if(!instr.loop_start){
            // no loop
            mod.instr[i].loop_end = mod.instr[i].len;
            mod.instr[i].loop_len = 0;
        } else {
            mod.instr[i].loop_end = (instr.loop_start+instr.loop_len)*2;
            mod.instr[i].loop_len = instr.loop_len*2;
        }


//    printf("%2d: len %6d loop_end %6d loop_len %6d name %s (internal)\n"
//      ,i+1
//      ,mod.instr[i].len
//      ,mod.instr[i].loop_end
//      ,mod.instr[i].loop_len
//      ,mod.instr[i].name
//    );
    }
//  exit(1);

    mod.song_len = read_u8(f);
//  printf("song_len %d\n",mod.song_len);
//  exit(1);

    mod.song_end = read_u8(f);
//  printf("mod.song_end %d\n",mod.song_end);
//  exit(1);

    assert(1==fread(&mod.sequence,sizeof(mod.sequence),1,f));

    {
        int i;
        mod.patt_count = 0;
        for( i=0 ; i<128 ; i++ ){
            if(mod.patt_count>=mod.sequence[i]) continue;
            mod.patt_count = mod.sequence[i];
        }
        mod.patt_count ++;
    }

//  printf("sequence ->\n");
//  {
//    int i;
//    for( i=0 ; i<128 ; i++ ){
//      printf("%3d ",mod.sequence[i]);
//      if(i%16==15)printf("\n");
//    }
//  }
//  printf("patt_count %d\n",mod.patt_count);

    // File format tag, solo nei mod con 31 strumenti
    if(mod.instr_count==31){
        char tag[4+1]={0};
        assert(1==fread(tag,4,1,f));
//    printf("tag %s\n",tag);
    }

    // patterns!
    // ogni pattern è 4 "note" (4 bytes per nota ) x 4 canali x 64 linee = 1024 bytes
    mod.pattern = (FILE_PATTERN*)malloc( sizeof(mod.pattern[0]) * mod.patt_count );
    assert(mod.pattern);
    {
        int i;
        for( i=0 ; i<mod.patt_count ; i++ ){
            assert( 1==fread(&mod.pattern[i],sizeof(mod.pattern[i]),1,f));
        }
    }

    // pcm
    {
        int i;
        for( i=0 ; i<mod.instr_count ; i++ ){
            if(!mod.instr[i].len)continue;
//      printf("pcm %d\n",i);
            mod.instr[i].wavetable = (int8_t*)malloc(mod.instr[i].len);
            assert(mod.instr[i].wavetable);
            assert(1==fread(mod.instr[i].wavetable,mod.instr[i].len,1,f));
        }
    }

//  // dump
//  {
//    int patt;
//    for( patt=0 ; patt<mod.patt_count ; patt++ ){
//      int line;
//      printf("pattern %d:\n",patt);
//      for( line=0 ; line<64 ; line++ ){
//        int track;
//        printf("%2d: ",line);
//        for( track=0 ; track<TRACKS ; track++ ){
//          union FILE_NOTE n = mod.pattern[patt].line[line].track[track];
//          int instrument = INSTRUMENT(n);
//          int period     = PERIOD(n);
//          int effect     = EFFECT(n);
//          int fxparam    = FXPARAM(n);
//
//          if(period)     printf("%4x ",period);     else printf("     ");
////          if(period)      print_note_from_period(period);      else printf("---- ");
//          if(instrument) printf("%2d ",instrument); else printf("   ");
//          if(effect)     printf("%1x",effect);      else printf(" ");
//          if(fxparam)    printf("%2x : ",fxparam);  else printf("   : ");
//        }
//        printf("\n");
//      }
//    }
//  }
}


