//
//  threads.h
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef threads_h
#define threads_h

#include "utility.h"

/* funzione handler dei segnali che vengono inviati al thread listener per chiudere*/
void termination_loopback(int signum);

//thread per la gestione dei segnali per far terminare il programma
void* Segnali(void *arg);

//thread delle statistiche
void* statThread(void* arg);

//thread che si occupa della decodifica delle immagini
void* DecoderThread(void* arg);

//thread che si occupa di creare i GOP e salvarli
void* ReaderPacket(void* arg);

//thread che definisce i GOP
void* GOPThread(void* arg);

//thread che sniffa i pacchetti
void* listenerThread(void* arg);

//callback per sniffare i pacchetti
void sniff(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char* packet);

#endif /* threads_h */
