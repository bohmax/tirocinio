#!/bin/bash
el=3
removed_comment=$(grep -o '^[^#]*' "$1")

var=$(echo "$removed_comment" | awk '/^channels_stats/{$1=$2=""; print $0}')
num=$(echo "$removed_comment" | awk '/^number_of_links/{print $3}') #numero di canali
simulator=$(echo "$removed_comment" | awk '/^simulator/{print $3}')
var=$(echo $var | tr -d "[]") # rimuove le parentesi quadre di channels stats
var=$(echo "$var" | sed 's.; .;.g') # Si elimina lo spazio successivo a ;
IFS=';' read -a arr <<< "$var" # crea un array per ogni ;
i=0
j=0
if [[ simulator -eq 1 ]]; then
	for el in "${arr[@]}"
	do
		i=0
		for val in $el
		do
			if [[ "$val" =~ ^[+-]?[0-9]+\.?[0-9]*$ ]]; then #controlla se è un numero
				input[i]=$val
				((i++))
			fi
		done
		if [[ !($i -eq 6) ]]; then
			echo "Numero di argomenti delle statistiche delle porte sbagliato"
			exit 1
		fi
		((j++))
	done
	if [[ !($j -eq 3) ]]; then
			echo "Il numero di canali non coincide con il numero di parametri delle statistiche"
			exit 1
		fi
else
	for i in $(seq 1 $num)
	do
		input[$i-1]=0
	done
fi