#!/bin/sh
for f in $(ls *.txt); do
    if [ -s $f ]; then
        dir=$(echo "$f" | cut -d. -f1)
        if [ -e $dir ]; then
            continue
        fi
        mkdir $dir
        cd $dir && wget -i ../$f && cd ..
        pwd
    fi
done
