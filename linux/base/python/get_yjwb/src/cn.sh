#!/bin/sh
for f in $(ls *.txt); do 
    id=$(echo "$f" | cut -d. -f1)
    echo $id
    for i in $(cat name_file); do 
        p_name=$(echo "$i" | cut -d : -f1)
        p_id=$(echo "$i" | cut -d : -f2)
        echo $p_name
        if [ "$id" == "$p_id" ]; then
            mv $f $p_name.txt
        fi
    done 
done
