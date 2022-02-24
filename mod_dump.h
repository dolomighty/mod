

void mod_dump(){

    printf("songname %s\n",mod.songname);

    printf("instruments:\n");

    {
        int i;
        for( i=0 ; i<mod.instr_count ; i++ ){
            printf("%2d: len %6ld loop_start %6ld loop_len %6ld name %s\n"
                ,i+1
                ,mod.instr[i].len
                ,mod.instr[i].loop_start
                ,mod.instr[i].loop_len
                ,mod.instr[i].name
            );
        }
    }

    printf("song_len %d\n",mod.song_len);
    printf("start_bpm %d\n",mod.start_bpm);
    printf("patt_count %d\n",mod.patt_count);
    
    printf("sequence ->\n");
    {
        int i;
        for( i=0 ; i<128 ; i++ ){
            printf("%3d ",mod.sequence[i]);
            if(i%16==15)printf("\n");
        }
    }

    {
        int patt;
        for( patt=0 ; patt<mod.patt_count ; patt++ ){
            int line;
            printf("pattern %d:\n",patt);
            for( line=0 ; line<64 ; line++ ){
                int track;
                printf("%2d: ",line);
                for( track=0 ; track<TRACKS ; track++ ){
                    union FILE_NOTE n = mod.pattern[patt].line[line].track[track];
                    int instrument = INSTRUMENT(n);
                    int period     = PERIOD(n);
                    int effect     = EFFECT(n);
                    int fxparam    = FXPARAM(n);

                    if(period)     printf("%4x ",period);     else printf("     ");
                    if(instrument) printf("%2d ",instrument); else printf("   ");
                    if(effect)     printf("%1x",effect);      else printf(" ");
                    if(fxparam)    printf("%2x : ",fxparam);  else printf("   : ");
                }
                printf("\n");
            }
        }
    }
}


