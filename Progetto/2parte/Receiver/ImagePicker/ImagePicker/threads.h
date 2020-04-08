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

//thread per la gestione dei segnali per far terminare il programma
void* segnali(void *arg);

void* DecoderThread(void* arg);

void* GOPThread(void* arg);

void* listenerThread(void* arg);

void sniff(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char*
           packet);

#endif /* threads_h */
