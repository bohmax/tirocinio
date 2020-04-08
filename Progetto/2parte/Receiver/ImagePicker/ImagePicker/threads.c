//
//  threads.c
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "threads.h"

void* segnali(void *arg){
    int ris;
    printf("Avviato\n");
    if(sigwait(&sigset_usr, &ris)!=0){
        perror("SIGWAIT");
        errno=0;
        exit(0);
    }
    printf("In uscita..\n");
    //per qualsiasi segnare registrato termina pcap_loop
    pcap_breakloop(handle);
    /* invia chiusura gop thread */
    //pulisci la liste così si è sicuri dell' uscita
    pthread_mutex_lock(&mtx_gop);
    freeList(&testa_gop, &coda_gop, &freeRTP);
    pushList(&testa_gop, &coda_gop, setElRTP(NULL, -1, -1));
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
    //svuota la lista di decodifica
    pthread_mutex_lock(&mtx_dec);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    pthread_mutex_unlock(&mtx_dec);
    for (int i = 0; i < NUMDECODERTHR; i++) {
        pthread_mutex_lock(&mtx_dec);
        pushList(&testa_dec, &coda_dec, setElGOP(-1));
        pthread_cond_signal(&cond_dec);
        pthread_mutex_unlock(&mtx_dec);
    }
    esci = 1;
    return (void*) 0;
}

void* DecoderThread(void* arg){
    printf("Thread per la decodifica di un GOP creato\n");
    int esci = 0;
    gop_info* el = NULL;
    while (!esci) {
        pthread_mutex_lock(&mtx_dec);
        while (testa_dec == NULL)
            pthread_cond_wait(&cond_dec, &mtx_dec);
        el = popList(&testa_dec, &coda_dec);
        pthread_mutex_unlock(&mtx_dec);
        if (el->gop_num >= 0)
            create_image(el);
        else esci = 1;
        freeGOP((void**)&el);
    }
    printf("Thread decodifica chiude\n");
    return (void*) 0;
}

void* GOPThread(void* arg){
    printf("Thread che crea GOP creato\n");
    int chiudi = 0, count = 0, gop_count = 0, ret;
    pthread_t decodehandler[NUMDECODERTHR];
    for (int i = 0; i < NUMDECODERTHR; i++)
        SYSFREE(ret,pthread_create(&decodehandler[i],NULL,&DecoderThread,NULL),0,"thread")
    gop_info* info = setElGOP(gop_count);
    rtp* el = NULL;
    while (!chiudi) {
        pthread_mutex_lock(&mtx_gop);
        while (testa_gop == NULL)
            pthread_cond_wait(&cond_gop, &mtx_gop);
        el = popList(&testa_gop, &coda_gop);
        pthread_mutex_unlock(&mtx_gop);
        if (el->packet) {
            if(workOnPacket(el, info)){ //attualmente è copiato attravarso una alloc, potrei togliere la memcpy
                pthread_mutex_lock(&mtx_dec);
                pushList(&testa_dec, &coda_dec, info);
                pthread_cond_signal(&cond_dec);
                pthread_mutex_unlock(&mtx_dec);
                gop_count++;
                info = setElGOP(gop_count);
            }
        }
        else {
            chiudi = 1;
            freeRTP((void**)&el);
        }
        count++;
    }
    freeGOP((void**)(&info));
    for (int i = 0; i < NUMDECODERTHR; i++) {
        pthread_mutex_lock(&mtx_dec);
        pushList(&testa_dec, &coda_dec, setElGOP(-1));
        pthread_cond_signal(&cond_dec);
        pthread_mutex_unlock(&mtx_dec);
    }
    for (int i = 0; i < NUMDECODERTHR; i++)
        SYSFREE(ret,pthread_join(decodehandler[i],NULL),0,"decode 1")
    
    printf("Thread che crea GOP esce\n");
    return (void*) 0;
}

void* listenerThread(void* arg){
    //creazione thread che gestisce
    int ret;
    pthread_t gophandler;
    rtp* end = setElRTP(NULL, -1,-1);
    SYSFREE(ret,pthread_create(&gophandler,NULL,&GOPThread,NULL),0,"thread")
    pcap_loop(handle, -1, sniff, NULL);
    printf("Thread in uscita\n");
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, end);
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
    SYSFREE(ret,pthread_join(gophandler,NULL),0,"gop 1")
    return (void*)0;
}

void sniff(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char*
              packet){
    num_pkt += 1;
    int to_udp = (sizeof(struct ether_header) + sizeof(struct ip));
    int udplen = pkthdr->len - to_udp;
    //setto gli elementi per poter passare i dati a un altro thread
    u_char* buf = malloc(sizeof(u_char)*udplen);
    memcpy(buf, packet+to_udp, udplen);
    rtp* el= setElRTP(buf, udplen, num_pkt);
    //possibili miglioramenti eliminare le lock -> UN Produttore e un consumatore
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, el);
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
}
