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
void* segnali(void *arg);

void* statThread(void* arg);

void* DecoderThread(void* arg);

void* ReaderPacket(void* arg);

void* GOPThread(void* arg);

void* listenerThread(void* arg);

void sniff(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char* packet);

#endif /* threads_h */
