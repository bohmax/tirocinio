#!/bin/bash
helper="NAME
	Stream - Permette la visualizzazione di più stream rtp in un unico video
SYNOPSIS
	helper INTERFACE HOW_MANY_CHANNEL SRC_PORT DST_PORT GOP_FOLDER IMG_FOLDER
DESCRIPTION
	INTERFACE è l'interfaccia da cui sniffare i pacchetti
	HOW_MANY_CHANNEL il numero di canali
	SRC_PORT il primo numero di porta da cui sniffare i pacchetti
	DST_PORT porta di destinazione del video e porta per inviare le statistiche
	GOP_FOLDER è la cartella in cui verranno salvati i GOP
	IMG_FOLDER è la cartella in cui verranmno salvate le immagini dei GOP.
AUTHOR
	Written by Massimo Puddu
	"
if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo"
    echo "sudo $0 $*"
    exit 1
fi
if [ "$#" -ne 3 ]; then
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

GSTREAM_LOC='gst-launch-1.0 '
PIPELINE='-v udpsrc port='"$4"' caps = "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96" ! rtpjitterbuffer ! rtph264depay ! decodebin ! videoconvert ! '
SCREEN='xvimagesink'
if [[ "$OSTYPE" == "darwin"* ]]; then
	GSTREAM_LOC='/Library/Frameworks/GStreamer.framework/Commands/gst-launch-1.0 '
	SCREEN='osxvideosink'
fi

eval $GSTREAM_LOC $PIPELINE $SCREEN &
PID="$!"
./client "$1" "$2" "$3"
wait $PID
