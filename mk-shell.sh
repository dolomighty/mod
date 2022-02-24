#!/bin/bash

rebuild(){
  date +"%F %T rebuild $TARGET"
  mkdir -p `dirname "$TARGET"`
  eval "$ONELINER" > "$TARGET"
}


awk '
/include.*SHELL/{
  TARGET=$2
  # "dyn/log2.h"
  gsub(/[<>""]/,"",TARGET)
  # dyn/log2.h
  ONELINER=$0
  sub(/.*SHELL /,"",ONELINER)
  sub(/\s+$/,"",ONELINER)
  print FILENAME " " TARGET " " ONELINER
}
' *.c* \
| while read SOURCE TARGET ONELINER ; do
#  echo $SOURCE " " $TARGET " " $ONELINER
  for ARG in $ONELINER; do
    # proviamo ad interpretare il comando come una lista di files
    # se abbiamo un file reale, facciamo i soliti check sull'età
    [[ -r $ARG ]] || continue
    [[ $ARG -nt "$TARGET" ]] || continue
    rebuild
    continue 2  # non serve fare altri test
  done
  [[ "$SOURCE" -nt "$TARGET" ]] && rebuild && continue
  [[ $0        -nt "$TARGET" ]] && rebuild && continue
done


true

exit
exit
exit


questo helper serve a produrre a compile time files da includere
seguire gli esempi qui sotto

#include "dyn/log2.h" // SHELL seq 1 255 | awk '{ printf("%d,",log($1)/log(2.0)+1) } NR%16==15{print""}'
#include "dyn/spruz.c" // SHELL ./pixmap2sprite.py sprite.png
          ↑↑↑↑↑↑↑↑↑↑↑           ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑     
            target                      oneliner

