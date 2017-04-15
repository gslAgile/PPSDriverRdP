#!/bin/bash

function add_matriz() {
	for (( i=0 ; $i<5; i++ )); do
		echo crear matrices > /proc/matrixmod_test_1
		echo "crear matrices $i [ok]"
		sleep 1
	done
	echo "--------------------------------------"
}
echo "Creando matrices..."
sleep 5
add_matriz
echo "ejecucion de scrip finalizada.\n"
echo