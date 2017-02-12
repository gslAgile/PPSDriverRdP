#!/bin/bash

# Variables
let dimf=0 # dimension de filas
let dimc=0 # dimension de columnas
let pf=0 # puntero filas
let pc=0 # puntero columnas
let longl=0 # longitud de lineas
linea=""
archivo="MI.txt"
archivo2="MA.txt"

# MATRIZ DE INCIDENCIA
# Calculo de dimension de filas de matriz en archivo
while read line
do
	echo -en "I $pf "
	dimf=$[$dimf + 1]
	linea=$line
done < $archivo

# Calculo de dimension de columnas de matriz
longl=${#linea}

for((i=0 ; i<$longl ; i++)); do
	if [ "${linea:$i:1}" -ge "0" 2>/dev/null ]||[ "${linea:$i:1}" -le "9" 2>/dev/null] ]; then
		dimc=$[$dimc+1]
	fi
done

# 2>/dev/null descarta algunos errores no importantes

echo -e
echo -e "dimf: $dimf Linea: $linea long: $longl dimc: $dimc"
echo -e

# Creacion de matriz en modulo
echo crear I _"$dimf"_"$dimc" > /proc/matrixmod
dmesg|tail


# Insercion de elementos de matriz en modulo
while read line
do
	longl=${#line}
	for ((i=0; i<$longl ; i=i+2)); do # i con avance doble para evitar espacios
		if [ "${line:$i:1}" = "-" 2>/dev/null ]; then
			echo add I "$pf"_"$pc"_"${line:$i:2}"  > /proc/matrixmod
			i=$i+1

		else
			echo add I "$pf"_"$pc"_"${line:$i:1}"  > /proc/matrixmod
		fi
		pc=$[$pc + 1]
	done
	i=0
	pc=0
	pf=$[$pf + 1]
	#echo -e
done < $archivo



# VECTOR DE MARCADO INICIAL
dimf=0 # dimension de filas
dimc=0 # dimension de columnas
pf=0 # puntero filas
pc=0 # puntero columnas
longl=0 # longitud de lineas
# Calculo de dimension de filas de matriz en archivo
while read line
do
	echo -en "I $pf "
	dimf=$[$dimf + 1]
	linea=$line
done < $archivo2

# Calculo de dimension de columnas de matriz
longl=${#linea}

for((i=0 ; i<$longl ; i++)); do
	if [ "${linea:$i:1}" -ge "0" 2>/dev/null ]||[ "${linea:$i:1}" -le "9" 2>/dev/null] ]; then
		dimc=$[$dimc+1]
	fi
done

# 2>/dev/null descarta algunos errores no importantes

echo -e
echo -e "dimf: $dimf Linea: $linea long: $longl dimc: $dimc"
echo -e

# Creacion de matriz en modulo
echo crear MI _"$dimf"_"$dimc" > /proc/matrixmod
dmesg|tail


# Insercion de elementos de matriz en modulo
while read line
do
	longl=${#line}
	for ((i=0; i<$longl ; i=i+2)); do # i con avance doble para evitar espacios
		if [ "${line:$i:1}" = "-" 2>/dev/null ]; then
			echo add MI "$pf"_"$pc"_"${line:$i:2}"  > /proc/matrixmod
			i=$i+1

		else
			echo add MI "$pf"_"$pc"_"${line:$i:1}"  > /proc/matrixmod
		fi
		pc=$[$pc + 1]
	done
	i=0
	pc=0
	pf=$[$pf + 1]
	#echo -e
done < $archivo2

exit 0