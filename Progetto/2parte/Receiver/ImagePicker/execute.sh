#!/bin/bash
helper="NAME
	Drones Streaming - Esecuzione del programma del tirocinio
SYNOPSIS
	helper WHO_EXECUTE SENDER_CONFIGURATION_FILE RECEIVER_CONFIGURATION_FILE
DESCRIPTION
	WHO_EXECUTE Intero che identifica che script eseguire, in questo modo,
	solo il file di configurazione corrispondente verrà utilizzato. 0 Per 
	utilizzare entrambi, 1 per utilizzare solo il sender e 2 per il receiver.
	SENDER_CONFIGURATION_FILE File di configurazione che gestisce il sender
	RECEIVER_CONFIGURATION_FILE di configurazione che gestisce il receiver
AUTHOR
	Written by Massimo Puddu
	"

if [[ $UID != 0 ]]; then
    echo "Please run this script with sudo"
    echo "sudo $0 $*"
    exit -1
fi
if [ "$#" -ne 3 ]; then
    echo "$helper"
    exit -1
fi

chmod +x Script/sender.sh
if [ ! $? -eq 0 ]; then
   echo "Failed to give scipt permission"
   exit -1
fi
chmod +x Script/receiver.sh
if [ ! $? -eq 0 ]; then
   echo "Failed to give scipt permission"
   exit -1
fi
chmod +x Script/all_in_one.sh
if [ ! $? -eq 0 ]; then
   echo "Failed to give scipt permission"
   exit -1
fi

# Attenzione qui ci si aspetta che entrambi i config file siano corretti 
# e che tutto vada a buon fine sennò si dovranno arrestare antrambi gli script(o uno dei due se l'altro fallisce) 
# manualmente inviando un SIGINT, è consigliabile usarli prima separatamente
if [[ "$1" -eq 0 ]]; then
	Script/all_in_one.sh "$2" "$3"
elif [[ "$1" -eq 1 ]]; then
	Script/sender.sh "$2"
elif [[ "$1" -eq 2 ]]; then
	Script/receiver.sh "$3"
else
	echo "Inserire il primo argomento con un valore tra 0 e 2 assicurandosi di selezionare il valore opportuno"
fi
