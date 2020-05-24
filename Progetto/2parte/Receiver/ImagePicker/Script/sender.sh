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
input[0]=$(awk '/^pcap_file/{print $3}' "$1") #posizione dei file pcap
input[1]=$(awk '/^interfaccia_di_rete/{print $3}' "$1") #interfaccia di rete in cui inserire e sniffare pacchetti
input[2]=$(awk '/^ip_receiver/{print $3}' "$1") #ip su cui inviare i pacchetti dal sender
input[3]=$(awk '/^numero_di_canali/{print $3}' "$1") #numero di canali
input[4]=$(awk '/^from_port/{print $3}' "$1") #numero della porta
input[5]=$(awk '/^stat_port/{print $3}' "$1") #numero della porta delle statistiche
input[6]=$(awk '/^cartella_gop/{print $3}' "$1") #cartella dei gop lato sender
input[7]=$(awk '/^cartella_immagini/{print $3}' "$1") #cartella immagini sender
input[8]=$(awk '/^gamma_evento/{print $3}' "$1") #valore di gamma che mi indica in quale intervallo iniziare l'evento di perdita
input[9]=$(awk '/^beta_evento/{print $3}' "$1") #valore di beta che mi indica in quale intervallo iniziare l'evento di perdita
input[10]=$(awk '/^gamma_perdita/{print $3}' "$1") #valore di gamma che mi indica quanti pacchetti perdere
input[11]=$(awk '/^beta_perdita/{print $3}' "$1") #valore di beta che mi indica quanti pacchetti perdere
input[12]=$(awk '/^gamma_delay/{print $3}' "$1") #valore di gamma che mi indica il delay
input[13]=$(awk '/^beta_delay/{print $3}' "$1") #valore di beta che mi indica il delay

for i in "${input[@]}"
do
	if [[ -z "$i" ]]; then #controlla che non ci siano elementi vuoti
		echo "config file non corretto"
		exit -1
	fi
done

for i in {6..7}
do
	if [ ! -d "${input[$i]}" ]; then # controlla che la directory non esista, se non esistono le crea
		if ! mkdir -p "${input[$i]}" ; then
			echo "Impossibile creare la cartella gop lato sender"
			exit 0
		fi
	fi
	length=${#input[i]}
	last_char=${input[i]:length-1:1}
	[[ $last_char != "/" ]] && input[i]="${input[i]}/";
done

python3 Sender/Scheduler.py "${input[0]}" "${input[1]}" "${input[2]}" "${input[3]}" "${input[4]}" "${input[5]}" "${input[6]}" "${input[7]}" "${input[8]}" "${input[9]}" "${input[10]}" "${input[11]}" "${input[12]}" "${input[13]}"&
clientpid+="$! "
wait $clientpid

