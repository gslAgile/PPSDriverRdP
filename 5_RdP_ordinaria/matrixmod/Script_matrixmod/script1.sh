#! /bin/bash

archivo="MA.txt"

while read line
do
	for ((i = 0; i < 11 ; i = i+2)); do
		echo -en "${line:$i:1}"
	done
	i=0
	echo -e
done < $archivo
