#!/bin/bash

let columna=0
let fila=0
let uno=1

while read line
do
	for ((i=0; i<11 ; i=i+2)); do
		echo add I "$fila"_"$columna"_"${line:$i:1}" > /proc/matrixmod
		columna=$[$columna + $uno]
	done
	i=0
	columna=0
	fila=$[$fila + $uno]
	#echo -e
done < MA.txt

exit 0
