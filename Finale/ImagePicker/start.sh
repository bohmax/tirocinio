#!/bin/bash
helper="NAME
	Stream - Permette la visualizzazione di più stream rtp in un unico video
SYNOPSIS
	helper INTERFACE HOW_MANY_CHANNEL SRC_PORT DST_PORT GOP_FOLDER IMG_FOLDER IMG_FOLDER_SEND STAT_LEN NUM_DECODER
DESCRIPTION
	INTERFACE è l'interfaccia da cui sniffare i pacchetti
	IP_SENDER ip del sender per inviare le statistiche
	HOW_MANY_CHANNEL il numero di canali
	SRC_PORT il primo numero di porta da cui sniffare i pacchetti
	DST_PORT porta di destinazione per inviare le statistiche
	VIDEO_PORT porta in cui inviare i pacchetti al player
	GOP_FOLDER è la cartella in cui verranno salvati i GOP
	IMG_FOLDER è la cartella in cui verranmno salvate le immagini dei GOP
	IMG_FOLDER_SEND è la cartella in cui il sender salva le immagini dei GOP
	STAT_LEN Tempo che intercorre prima di inviare le statistiche
	NUM_DECODER Numero di thread che decodificano il GOP.
AUTHOR
	Written by Massimo Puddu
	"
if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo"
    echo "sudo $0 $*"
    exit 1
fi
if [ "$#" -ne 11 ]; then
    echo "$helper"
    exit 1
fi
for nvar in "$@"
do
    if [ "$nvar" = "--help" ];then #Controllo se è presente un --help tra gli argomenti 
		echo "$helper"
		exit 1
	fi
done

if [ "${BASH_SOURCE[0]}" = "./start.sh" ]; then
	cd ..
fi

GSTREAM_LOC='gst-launch-1.0 '
PIPELINE='-v udpsrc port='"$6"' caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtpjitterbuffer ! rtph264depay ! decodebin ! videoconvert ! '
SCREEN='xvimagesink'
if [[ "$OSTYPE" == "darwin"* ]]; then
	GSTREAM_LOC='/Library/Frameworks/GStreamer.framework/Commands/gst-launch-1.0 '
	SCREEN='osxvideosink'
fi

eval $GSTREAM_LOC $PIPELINE $SCREEN &
PID+="$!"
ImagePicker/client "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$11"
PID+="$!"
wait $PID
