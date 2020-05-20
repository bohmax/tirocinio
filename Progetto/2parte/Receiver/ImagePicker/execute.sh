#!/bin/bash
helper="NAME
	Drones Streaming - Esecuzione del programma del tirocinio
SYNOPSIS
	helper CONFIGURATION_FILE
DESCRIPTION
	CONFIGURATION_FILE File di configurazione che gestisce sia sender e receiver
AUTHOR
	Written by Massimo Puddu
	"
if [ "$#" -ne 1 ]; then
    echo "$helper"
    exit 1
fi
input[0]=$(awk '/^pcap_file/{print $3}' "$1") #posizione dei file pcap
input[1]=$(awk '/^numero_di_canali/{print $3}' "$1") #numero di canali
input[2]=$(awk '/^dalla_porta/{print $3}' "$1") #numero della porta
input[3]=$(awk '/^interfaccia_di_rete/{print $3}' "$1") #interfaccia di rete in cui inserire e sniffare pacchetti
input[4]=$(awk '/^stat_port/{print $3}' "$1") #numero della porta delle statistiche
input[5]=$(awk '/^cartella_gop/{print $3}' "$1") #cartella dei gop lato sender
input[6]=$(awk '/^cartella_immagini/{print $3}' "$1") #cartella immagini sender
input[7]=$(awk '/^cartella_rec_gop/{print $3}' "$1") #cartella dei gop lato receiver
input[8]=$(awk '/^cartella_rec_immagini/{print $3}' "$1") #cartella delle immagini lato receiver
input[9]=$(awk '/^lunghezza_finestra_statistiche/{print $3}' "$1") #lunghezza della finestra delle statistiche
input[10]=$(awk '/^num_decoder_thread/{print $3}' "$1") #numero dei thread del decoder
input[11]=$(awk '/^ip/{print $3}' "$1") #ip su cui inviare i pacchetti dal sender
input[12]=$(awk '/^port_rtp_pcap/{print $3}' "$1") #porta che contiente in flusso rtp del file pcap
input[13]=$(awk '/^gamma_evento/{print $3}' "$1") #valore di gamma che mi indica in quale intervallo iniziare l'evento di perdita
input[14]=$(awk '/^beta_evento/{print $3}' "$1") #valore di beta che mi indica in quale intervallo iniziare l'evento di perdita
input[13]=$(awk '/^gamma_perdita/{print $3}' "$1") #valore di gamma che mi indica quanti pacchetti perdere
input[14]=$(awk '/^beta_perdita/{print $3}' "$1") #valore di beta che mi indica quanti pacchetti perdere
input[15]=$(awk '/^delay/{print $3}' "$1") #dalay massimo con cui ritardare un pacchetto

for i in "${input[@]}"
do
	if [[ -z "$i" ]]; then #controlla che non ci siano elementi vuoti
		echo "config file non corretto"
		exit -1
	fi
done

for i in {5..8}
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

#python3 Scheduler.py "${input[0]}" "${input[3]}" "${input[11]}" "${input[12]}" "${input[1]}" "${input[2]}" "${input[4]}" "${input[5]}" "${input[6]}" &
#sleep 2
ImagePicker/start.sh "${input[3]}" "${input[1]}" "${input[2]}" "${input[4]}" "${input[7]}" "${input[8]}" "${input[6]}" "${input[9]}" "${input[10]}"
