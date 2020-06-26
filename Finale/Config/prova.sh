#!/bin/bash
el=3
var=$(awk '/^canale/{$1=$2=""; print $0" \\n"}' "$1")
num=$(awk '/^numero_di_canali/{print $3}' "$1") #numero di canali
var=$(echo "$var" | sed 's/#.*\\n//g')
i=0
for el in $var; do
	if [[ "$el" =~ ^[+-]?[0-9]+\.?[0-9]*$ ]]; then #controlla che sia un nuumero
		input[i]=$el
		((i++))
	fi
done

if (( $i !=  $num*6 )); then
	echo "Numero di elementi per le distribuzioni errato"
fi