#!/bin/bash
helper="NAME
	Drones Streaming - Esecuzione del programma del tirocinio
SYNOPSIS
	helper CONFIGURATION_FILE
DESCRIPTION
	CONFIGURATION_FILE File di configurazione che contiene i dati in input del receiver
AUTHOR
	Written by Massimo Puddu
	"
if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo"
    echo "sudo $0 $*"
    exit -1
fi
if [ "$#" -ne 1 ]; then
    echo "$helper"
    exit -1
fi
removed_comment=$(grep -o '^[^#]*' "$1")
input[0]=$(echo "$removed_comment" | awk '/^nic/{print $3}') #interfaccia di rete in cui inserire e sniffare pacchetti
input[1]=$(echo "$removed_comment" | awk '/^sender_IP/{print $3}') #ip per spedire le statistiche al sender
input[2]=$(echo "$removed_comment" | awk '/^number_of_links/{print $3}') #numero di canali
input[3]=$(echo "$removed_comment" | awk '/^port/{print $3}') #sniffa pacchetti dalla porta x fino alla porta x+numero canali
input[4]=$(echo "$removed_comment" | awk '/^feedback_port_at_sender/{print $3}') #numero della porta delle statistiche
input[5]=$(echo "$removed_comment" | awk '/^video_port/{print $3}') #numero della porta per inoltrare i pacchetti al player
input[6]=$(echo "$removed_comment" | awk '/^GOP_folder/{print $3}') #cartella dei gop lato receiver
input[7]=$(echo "$removed_comment" | awk '/^VF_folder/{print $3}') #cartella delle immagini lato receiver
input[8]=$(echo "$removed_comment" | awk '/^original_VF_folder/{print $3}') #cartella immagini sender
input[9]=$(echo "$removed_comment" | awk '/^window_length/{print $3}') #lunghezza della finestra delle statistiche
input[10]=$(echo "$removed_comment" | awk '/^decoding_threads/{print $3}') #numero dei thread del decoder

for i in "${input[@]}"
do
	if [[ -z "$i" ]]; then #controlla che non ci siano elementi vuoti
		echo "config file non corretto"
		exit -1
	fi
done

for i in {6..8}
do
	if [ ! -d "${input[$i]}" ]; then # controlla che la directory non esista, se non esistono le crea
		if ! mkdir -p "${input[$i]}" ; then
			echo "Impossibile creare la cartella gop lato sender"
			exit -1
		fi
	fi
	length=${#input[i]}
	last_char=${input[i]:length-1:1}
	[[ $last_char != "/" ]] && input[i]="${input[i]}/";
done

if [ ! -f ImagePicker/client ]; then
    cd ImagePicker
    make
    if [ ! $? -eq 0 ]; then
    	cd ..
    	echo "Makefail failed try to run it from it s folder and check if ffmpeg is linked with the path in makefile"
    	exit -1
    fi
    cd .. 
fi

sleep 2
ImagePicker/start.sh "${input[0]}" "${input[1]}" "${input[2]}" "${input[3]}" "${input[4]}" "${input[5]}" "${input[6]}" "${input[7]}" "${input[8]}" "${input[9]}" "${input[10]}"&
clientpid+="$! " 
wait $clientpid

