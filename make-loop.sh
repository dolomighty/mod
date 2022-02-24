#!/bin/bash
$FIRST && FIRST=false exec xterm -fa fixed -fs 12 -geometry 100x20-0-0 -iconic -e $0

date "+




=================================================================
%F %T `basename $0` release 2016-07-14 11:29:56"

# titolo della win. si aspetta /c/ oppure /html+js/ nel path
TIT=${PWD##*/c/}
TIT=${TIT##*/html+js/}
echo -ne "\033]0;$TIT\07"

# se c'è già un make in corso, aspettiamo
ASD='date +"%F %T make occupato. aspettiamo ..."'
while pgrep -x make &> /dev/null ; do
  eval $ASD
  ASD=
  sleep 1
done

ICO=dialog-error
MSG=fail
make &> .make.out+err
[[ $? == 0 ]] && ICO=dialog-apply && MSG=ok
cat .make.out+err


# notifiche varie
#echo -en "\033]0;`date '+%T'`\007"
#notify-send -t 3000 "`basename $0` - fatto" -- "`ls -l $CODE`"
#"$ERR"
notify-send -i $ICO -t 3000 "$TIT make" "$MSG"

# finito. ora aspettiamo che cambi qualunque cosa nei files
H="`grep -HRa "" * | md5sum`"
while sleep 0.5 ; do
  N="`grep -HRa "" * | md5sum`"
  [[ "$H" == "$N" ]] && continue
  exec $0
done


# qui non si deve mai arrivare


exit
exit
exit
exit



# speed test per check vari
time for I in {1..1000} ; do
#H="`ls --full-time -R | md5sum`"  # 0m7.659s
#H="`find -L . -type f -exec cat {} \; | md5sum`"  # 0m26.047s
H="`grep -R . * | md5sum`"     # 0m6.345s
#H="`ls --full-time | crc32 /dev/stdin`"   # troppo
#H="`fgrep -R "" * | sha1sum`"   # 0m5.952s
#H="`fgrep -R "" * | md5sum`"   # 0m5.966s
done





# speed test per check vari
time for (( A=10000 ; A ; A -- )) ; do sleep 0 ; done   # 0m26.242s
#time for A in `seq 10000` ; do sleep 0 ; done # 0m22.981s
#time for A in {1..10000} ; do sleep 0 ; done # 0m25.562s




# 2015-10-13 19:51:30 
# incredibilmente, la fn crc32 è più LENTA di md5sum
# ecco uno dei casi in cui misurare le performance 
# è meglio che fidarsi dell'istinto
# lascio il blocco commentato per riferimenti futuri
#
#H=`ls --full-time | crc32 /dev/stdin`
#while sleep 0.5 ; do
#  [[ $H == `ls --full-time | crc32 /dev/stdin` ]] && continue
#  exec $0
#done
#while sleep 0.5 ; do
#  H=`ls --full-time | crc32 /dev/stdin`
#  [[ -n $L ]] && [[ $L != $H ]] && exec $0
#  L=$H
#done


