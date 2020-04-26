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
input[11]=$(awk '/^dim_gop_start/{print $3}' "$1") #dimensione del gop header
input[12]=$(awk '/^dim_gop/{print $3}' "$1") #dimensione di un gop totale
input[13]=$(awk '/^ip/{print $3}' "$1") #ip su cui inviare i pacchetti dal sender

for i in "${input[@]}"
do
	if [[ -z "$i" ]]; then #controlla che non ci siano elementi vuoti
		echo "config file non corretto"
		exit -1
	fi
done

python3 Scheduler.py "{input[0]}" "{input[3]}" "{input[13]}" "{input[1]}" "{input[2]}" "{input[5]}" "{input[6]}"