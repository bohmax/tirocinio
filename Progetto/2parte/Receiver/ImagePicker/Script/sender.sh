#!/bin/bash
helper="NAME
	Drones Streaming - Esecuzione del programma del tirocinio
SYNOPSIS
	helper CONFIGURATION_FILE
DESCRIPTION
	CONFIGURATION_FILE File di configurazione che contiene l'input per il sender
AUTHOR
	Written by Massimo Puddu
	"
if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo"
    echo "sudo $0 $*"
    exit 1
fi
if [ "$#" -ne 1 ]; then
    echo "$helper"
    exit 1
fi
removed_comment=$(grep -o '^[^#]*' "$1")
input[0]=$(echo "$removed_comment" | awk '/^pcap_src_file/{print $3}') #posizione dei file pcap
input[1]=$(echo "$removed_comment" | awk '/^receiver_IP/{print $3}') #ip su cui inviare i pacchetti dal sender
input[2]=$(echo "$removed_comment" | awk '/^number_of_links/{print $3}') #numero di canali
input[3]=$(echo "$removed_comment" | awk '/^port/{print $3}') #numero della porta
input[4]=$(echo "$removed_comment" | awk '/^incoming_feedback_port/{print $3}') #numero della porta delle statistiche
input[5]=$(echo "$removed_comment" | awk '/^GOP_folder/{print $3}') #cartella dei gop lato sender per i GOP
input[6]=$(echo "$removed_comment" | awk '/^VF_folder/{print $3}') #cartella immagini sender per le immagini
input[7]=$(echo "$removed_comment" | awk '/^simulator/{print $3}') #cartella immagini sender
canali=$(echo "$removed_comment" | awk '/^channels_stats/{$1=$2=""; print $0" \\n"}') #stampa tutta la riga per le statistiche
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
	if [[ !($j -eq ${input[2]}) ]]; then
			echo "Il numero di canali non coincide con il numero di parametri delle statistiche"
			exit 1
		fi
else
	for i in $(seq 1 $num)
	do
		input[$i-1]=0
	done
fi

for i in "${input[@]}"
do
	if [[ -z "$i" ]]; then #controlla che non ci siano elementi vuoti
		echo "config file non corretto"
		exit -1
	fi
done

for i in {5..6}
do
	if [ ! -d "${input[$i]}" ]; then # controlla che la directory non esista, se non esistono le crea
		if ! mkdir -p "${input[$i]}" ; then
			echo "Impossibile creare la cartella gop lato sender"
			exit 0
		fi
	fi
	length=${#input[i]}
	last_char=${input[i]:length-1:1}
	[[ $last_char != "/" ]] && input[i]="${input[i]}/";  #aggiunge uno slash finale nei path
done

python3 Sender/Scheduler.py ${input[@]}&
clientpid+="$! "
wait $clientpid

