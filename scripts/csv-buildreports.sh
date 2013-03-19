#!/bin/bash

export IFS='
'
for i in `./csv-list.pl -col`
do
    echo Trying: .`echo $i | tr -s " " ":" | cut -d":" -f2`.
    ./csv-check.pl `echo $i | tr -s " " ":" | cut -d":" -f2`
done

