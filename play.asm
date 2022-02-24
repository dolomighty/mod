

; mix routine

proc voice_0_trigger
args
dw wavetable_ptr
dw loop_end
dw loop_len
dd incr
dw volume

          mov     ax , wavetable_ptr
          mov     cs:voice_0_ptr , ax
          add     ax , loop_end
          mov     cs:voice_0_loop_end , ax
          mov     ax , loop_len
          mov     cs:voice_0_loop_len , ax
          mov     ax , incr+0
          mov     cs:voice_0_incr_lo , ax
          mov     ax , incr+2
          mov     cs:voice_0_incr_hi , ax
          xor     ax , ax
          mov     cs:voice_0_ptr_frac , ax
          mov     al , byte ptr volume
          mov     cs:voice_0_volume , al
endp



macro voice_sample N

          mov     al , $12
voice_#N#_volume=$-1          
          imul    byte ptr [$1234]
voice_#N#_ptr=$-2
          add     dx , ax
          add     word ptr cs:voice_#N#_ptr_frac , $1234
voice_#N#_incr_lo=$-2
          adc     word ptr cs:voice_#N#_ptr , $1234
voice_#N#_incr_hi=$-2
          cmp     word ptr cs:voice_#N#_ptr , $1234
voice_#N#_loop_end=$-2
          jae     voice_#N#_rewind    ; unsigned test
voice_#N#_continue:

endm




macro voice_rewind N

voice_#N#_rewind:
          sub     word ptr cs:voice_#N#_ptr , $1234
voice_#N#_loop_len=$-2
          jmp     voice_#N#_continue

voice_#N#_ptr_frac dw 0

endm


proc mix
          xor     dx , dx

          voice_sample 0
          voice_sample 1
          voice_sample 2
          voice_sample 3

mix_done:
          ret

          voice_rewind 0
          voice_rewind 1
          voice_rewind 2
          voice_rewind 3
endp



