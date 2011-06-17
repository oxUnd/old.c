#!/bin/sh

for d in $(ls .); do
    echo $d
    if [ -d $d ]; then
        for l in $(ls ./$d); do
            file=$(basename $l | cut -d. -f1)
            ext=$(basename $l | cut -d. -f2)
            id=$(echo "$file" | cut -d- -f2)
            id_3=$(echo "$file" | cut -d- -f3)
            if [ "$id_3" != "" ]; then
                #echo $id_3
                mv ./$d/$l ./$d/${d}_${id_3}.$ext
                continue
            fi
            if [ "$id" != "" ]; then
                mv ./$d/$l ./$d/${d}_${id}.$ext
            fi
        done
    fi
done
