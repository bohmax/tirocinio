#!/bin/bash
helper="NAME
	Drones Streaming - Esecuzione del programma del tirocinio
SYNOPSIS
	helper SENDER_CONFIGURATION_FILE RECEIVER_CONFIGURATION_FILE
DESCRIPTION
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
if [ "$#" -ne 2 ]; then
    echo "$helper"
    exit -1
fi

chmod +x sender.sh
if [ ! $? -eq 0 ]; then
   echo "Failed to give scipt permission"
   exit -1
fi
chmod +x receiver.sh
if [ ! $? -eq 0 ]; then
   echo "Failed to give scipt permission"
   exit -1
fi

# Attenzione qui ci si aspetta che entrambi i config file siano corretti 
# e che tutto vada a buon fine sennò si dovranno arrestare antrambi gli script(o uno dei due se l'altro fallisce) 
# manualmente inviando un SIGINT, è consigliabile usarli prima separatamente
./sender.sh "$1" &
clientpid+="$! " 
sleep 2
./receiver.sh "$2" &
clientpid+="$! " 
wait $clientpid