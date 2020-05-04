//
//  threads.c
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "threads.h"

/* funzione handler dei segnali che vengono inviati al thread listener per chiudere*/
void termination_loopback(int signum){
    int volatile i = 0, trovato = 0;
    pthread_t self = pthread_self();
    while(i<num_list && !trovato){
        if (self == listener[i]) {
            pcap_breakloop(handle[i]);
            trovato = 1;
        }
        i++;
    }
}

//manda la chiusura su una lista
void send_close(list** testa, list** coda, pthread_mutex_t* mtx, pthread_cond_t* cond, void(* del)(void** el), void* el){
    pthread_mutex_lock(mtx);
    freeList(testa, coda, del); //pulisci la liste così si è sicuri dell' uscita
    pushList(testa, coda, el);
    pthread_cond_signal(cond);
    pthread_mutex_unlock(mtx);
}

void* segnali(void *arg){
    int ris;
    printf("Avviato\n");
    if(sigwait(&sigset_usr, &ris)!=0){
        perror("SIGWAIT");
        errno=0;
        exit(0);
    }
    printf("In uscita..\n");
    esci = 1;
    //invia segnali a listener per sbloccarlo da pcap loop
    for (int i=0; i<num_list; i++)
        pthread_kill(listener[i], SIGALRM);
    pthread_kill(stat_thr, SIGALRM); //sveglia thread statistiche per un uscita rapida
    /* inviare qui un pacchetto falzo per essere sicuri di uscire */
    /* invia chiusura gop thread */
    send_close(&testa_gop, &coda_gop, &mtx_gop, &cond_gop, &freeRTP, setElRTP(NULL, -1, -1, -1));
    send_close(&testa_ord, &coda_ord, &mtx_ord, &cond_ord, &freeGOP, setElGOP(-1, -1));
    //svuota la lista di decodifica
    pthread_mutex_lock(&mtx_dec);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    for (int i = 0; i < num_decoder; i++) {
        pushList(&testa_dec, &coda_dec, setElGOP(-1, -1));
        pthread_cond_signal(&cond_dec);
    }
    pthread_mutex_unlock(&mtx_dec);
    return (void*) 0;
}

void* statThread(void* arg){
    int i = 0;
    int* index = malloc(sizeof(int)*num_list); //indice dei vari array delle stat
    uint16_t** copy_array = malloc(sizeof(uint16_t*)*num_list);
    uint16_t* current = malloc(sizeof(uint16_t)*num_list); //viene usato per calcolare out of ord
    send_stat* spedisci = malloc(sizeof(send_stat)*num_list);
    for(int i = 0; i<num_list;i++){
        index[i] = 0;
        current[i] = 0;
        copy_array[i] = malloc(sizeof(uint16_t)*DIMARRSTAT);
        for (int j = 0; j < DIMARRSTAT; j++)
            copy_array[i][j] = 0;
        memset(&spedisci[i], 0, sizeof(send_stat));
    }
    memset(spedisci, 0, sizeof(send_stat)*num_list);
    int sockfd = set_stat_sock(); //in utility.c
    while (!esci) {
        usleep(stat_interv);
        for (i = 0; i<num_list; i++) {
            pthread_mutex_lock(&mtxstat[i]);
            stat_t* stat = &statistiche[i];
            index[i] = stat->index;
            for(int x=0; x<stat->index;x++)
                copy_array[i][x] = stat->ids[x];
            spedisci[i].perdita = stat->index - (stat->max - stat->min) - 1;
            spedisci[i].delay = stat->delay;
            current[i] = stat->min;
            stat->id_accepted = stat->max;
            stat->index = 0;
            stat->delay = 0;
            stat->min = -1;
            stat->max = 0;
            stat->delay = 0;
            delay_calibrator[i] = -1;
            pthread_mutex_unlock(&mtxstat[i]);
        }
        //inizio vero calcolo stastiche
        for (i = 0; i < num_list; i++) {
            if(index[i] > 0){
                //solo la perdita viene calcolata prima
                spedisci[i].delay = spedisci[i].delay/index[i];
                //calcolo out of order
                spedisci[i].ordine = stat_out_of_order(copy_array[i], current[i], index[i]);
                //calcolo lunghezza
                spedisci[i].lunghezza = stat_lunghezza(copy_array[i],index[i]);
            }
            else {
                spedisci[i].perdita = -1;
                spedisci[i].delay = -1;
                spedisci[i].lunghezza = -1;
                spedisci[i].ordine = -1;
            }
        }
        // spedisci al server
        send_to_server(sockfd, spedisci);
    }
    // close the socket
    if (sockfd != -1)
        close(sockfd);
    free(index);
    for(int i = 0; i<num_list;i++)
        free(copy_array[i]);
    free(copy_array);
    free(current);
    free(spedisci);
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

void* ReaderPacket(void* arg){
    printf("Thread per salvare un GOP\n");
    int esci = 0, ret;
    gop_info* el = NULL;
    uint16_t* from = malloc(sizeof(uint16_t));
    pthread_t decodehandler[num_decoder];
    for (int i = 0; i < num_decoder; i++)
        SYSFREE(ret,pthread_create(&decodehandler[i],NULL,&DecoderThread,NULL),0,"thread")
    while (!esci) {
        pthread_mutex_lock(&mtx_ord);
        while (testa_ord == NULL)
            pthread_cond_wait(&cond_ord, &mtx_ord);
        el = popList(&testa_ord, &coda_ord);
        pthread_mutex_unlock(&mtx_ord);
        if (el->start_seq != -1) {
            usleep(LFINESTRA); //aspetta altri pacchetti per 200ms
            *from = el->start_seq;
            save_GOP(from, el);
            pthread_mutex_lock(&mtx_dec);
            pushList(&testa_dec, &coda_dec, el);
            pthread_cond_signal(&cond_dec);
            pthread_mutex_unlock(&mtx_dec);
        }
        else esci = 1;
    }
    free(from);
    freeGOP((void**)&el);
    for (int i = 0; i < num_decoder; i++) {
        pthread_mutex_lock(&mtx_dec);
        pushList(&testa_dec, &coda_dec, setElGOP(-1, -1));
        pthread_cond_signal(&cond_dec);
        pthread_mutex_unlock(&mtx_dec);
    }
    for (int i = 0; i < num_decoder; i++)
        SYSFREE(ret,pthread_join(decodehandler[i],NULL),0,"decode 1")
    printf("Thread per salvare il GOP chiude\n");
    return (void*) 0;
}

void* GOPThread(void* arg){
    printf("Thread che crea GOP creato\n");
    int num_thr = *(int*) arg, chiudi = 0, count = 0, gop_count = 0, ret;
    rtp* el = NULL;
    while (!chiudi) {
        pthread_mutex_lock(&mtx_gop);
        while (testa_gop == NULL)
            pthread_cond_wait(&cond_gop, &mtx_gop);
        el = popList(&testa_gop, &coda_gop);
        pthread_mutex_unlock(&mtx_gop);
        if (el->packet) {
            if((ret = workOnPacket(el, num_thr)) == -1) //attualmente è copiato attravarso una alloc, potrei togliere la memcpy
                freeRTP((void**)&el);
            else if (ret > 0){ //ret ha il numero di sequenza del nuovo pacchetto
                pthread_mutex_lock(&mtx_ord);
                pushList(&testa_ord, &coda_ord, info);
                pthread_cond_signal(&cond_ord);
                pthread_mutex_unlock(&mtx_ord);
                gop_count++;
                info = setElGOP(gop_count, ret);
            }
        }
        else {
            chiudi = 1;
            freeRTP((void**)&el);
        }
        count++;
    }
    /* manda l'ultimo GOP */ //probabilmente andrà tolto
    pthread_mutex_lock(&mtx_ord);
    pushList(&testa_ord, &coda_ord, info);
    pthread_cond_signal(&cond_ord);
    pthread_mutex_unlock(&mtx_ord);
    printf("Thread che crea GOP esce\n");
    return (void*) 0;
}

void* listenerThread(void* arg){
    //creazione thread che gestisce
    int ret, *num_thr = (int*) arg;
    /* registro l'handler per sigusr1*/
    struct sigaction new_action;
    new_action.sa_handler = termination_loopback;
    new_action.sa_flags = 0;
    ERRORSYSHEANDLER(ret,sigemptyset(&new_action.sa_mask),-1,"NO SIGEMPY LIST")
    ERRORSYSHEANDLER(ret,sigaction(SIGALRM, &new_action, NULL),-1,"NO ALARM")
    /* */
    pthread_t gophandler;
    SYSFREE(ret,pthread_create(&gophandler,NULL,&GOPThread,num_thr),0,"thread")
    pcap_loop(handle[*num_thr], -1, sniff, NULL);
    printf("Thread in uscita\n");
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, setElRTP(NULL, -1,-1, -1));
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
    SYSFREE(ret,pthread_join(gophandler,NULL),0,"gop 1")
    free(num_thr);
    return (void*)0;
}

void sniff(u_char *useless, const struct pcap_pkthdr* pkthdr, const u_char* packet){
    num_pkt += 1;
    //setto gli elementi per poter passare i dati a un altro thread
    u_char* buf = malloc(sizeof(u_char)*pkthdr->len);
    memcpy(buf, packet, pkthdr->len);
    //rtphdr* rtpHeader = (rtphdr*) (packet + 4 + sizeof(struct ip) + sizeof(struct udphdr));
    rtp* el= setElRTP(buf, pkthdr->len, pkthdr->ts.tv_sec, num_pkt);
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, el);
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
}
