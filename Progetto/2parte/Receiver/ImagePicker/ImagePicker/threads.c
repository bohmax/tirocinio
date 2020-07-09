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
    int volatile i = 0;
    pthread_t self = pthread_self();
    while(i<num_list){
        if (self == listener[i]) {
            pcap_breakloop(handle[i]);
            break;
        }
        i++;
    }
}

//manda la chiusura su una lista
void send_close(list** testa, list** coda, pthread_mutex_t* mtx, pthread_cond_t* cond, void(* del)(void** el), void* el){
    pthread_mutex_lock(mtx);
    if (del)
        freeList(testa, coda, del); //pulisci la liste così si è sicuri dell' uscita
    pushList(testa, coda, el);
    pthread_cond_signal(cond);
    pthread_mutex_unlock(mtx);
}

void* Segnali(void *arg){
    int ris;
    printf("Avviato\n");
    if(sigwait(&sigset_usr, &ris)!=0){
        perror("SIGWAIT");
        errno=0;
        exit(0);
    }
    printf("In uscita..\n");
    esci = 1;
    
    pthread_kill(stat_thr, SIGALRM); //sveglia thread statistiche per un uscita rapida
    /* inviare qui un pacchetto falzo per essere sicuri di uscire */
    /* invia chiusura gop thread */
    for (int i = 0; i < num_list; i++) {
        pthread_kill(listener[i], SIGALRM); //invia segnali a listener per sbloccarlo da pcap loop
        send_close(&testa_gop, &coda_gop, &mtx_gop, &cond_gop, NULL, setElRTP(NULL, -1, -1, -1, -1));
    }
    send_close(&testa_ord, &coda_ord, &mtx_ord, &cond_ord, &freeGOP, setElGOP(-1, -1));
    //svuota la lista di decodifica
    pthread_mutex_lock(&mtx_dec);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    for (int i = 0; i < num_decoder; i++) {
        pushList(&testa_dec, &coda_dec, setElGOP(-1, -1));
        pthread_cond_signal(&cond_dec);
    }
    pthread_mutex_unlock(&mtx_dec);
    printf("Thread segnali chiuso\n");
    return (void*) 0;
}

void* statThread(void* arg){
    int i = 0, j = 0, size, lenght, delay = 0;
    struct timespec ts = { 0, stat_interv*1000};
    stat_t temp; // usato per fare lo swap con statistiche e statistiche temp
    stat_t* statistiche_ref = malloc(sizeof(stat_t)*num_list); //contiene il riferimento alle vecchie statistiche
    uint16_t* ids_copy = malloc(sizeof(uint16_t)*DIMARRSTAT); //array temporaneo di lavoro che copia di nuovo gli ids
    send_stat* spedisci = malloc(sizeof(send_stat)*num_list);
    for(int i = 0; i<num_list;i++){
        memset(&statistiche_ref[i], 0, sizeof(stat_t));
        statistiche_ref[i].pkt_information = malloc(sizeof(pkt_info)*DIMARRSTAT);
        memset(statistiche_ref[i].pkt_information, 0, sizeof(pkt_info)*DIMARRSTAT);
        statistiche_ref[i].min = -1;
        memset(&spedisci[i], 0, sizeof(send_stat));
    }
    memset(ids_copy, 0, sizeof(uint16_t)*DIMARRSTAT);
    memset(spedisci, 0, sizeof(send_stat)*num_list);
    int sockfd = set_stat_sock(); //in utility.c
    while (!esci) {
        nanosleep(&ts, NULL);
        //cambia l'array
        for (i = 0; i < num_list; i++){
            pthread_mutex_lock(&mtxstat[i]);
            statistiche_ref[i].id_accepted = statistiche[i].max;
            temp = statistiche[i];
            statistiche[i] = statistiche_ref[i];
            statistiche_ref[i] = temp;
            pthread_mutex_unlock(&mtxstat[i]);
        }
        //inizio vero calcolo stastiche
        for (i = 0; i < num_list; i++) {
            if(statistiche_ref[i].index > 0){
                temp = statistiche_ref[i];
                for(j = 0; j < temp.index; j++){
                    ids_copy[j] = temp.pkt_information[j].ids; //copia array
                    delay += (float) temp.pkt_information[j].r_timestamp - temp.pkt_information[j].pkt_timestamp;
                }
                //calcolo delay
                spedisci[i].delay = (float) delay/statistiche_ref[i].index;
                qsort(statistiche_ref[i].pkt_information, statistiche_ref[i].index, sizeof(pkt_info), cmpfunc); // ordinalo
                //calcolo jitter
                spedisci[i].jitter = jitter_calculator(temp.pkt_information, temp.index);
                //calcolo out of order
                spedisci[i].ordine = stat_out_of_order(temp.pkt_information, ids_copy, statistiche_ref[i].index);
                //calcolo lunghezza
                spedisci[i].lunghezza = stat_lunghezza(temp.pkt_information, statistiche_ref[i].index);
                //calcolo numero di perdite
                size = statistiche_ref[i].index - 1; // numero di elementi nell'array - 1(l'estemo)
                lenght = temp.pkt_information[size].ids - temp.pkt_information[0].ids; // max - min
                spedisci[i].perdita = size > lenght ? size - lenght : lenght - size;
                spedisci[i].number_of_pkt = statistiche_ref[i].index;
            }
            else {
                spedisci[i].perdita = -1;
                spedisci[i].delay = -1;
                spedisci[i].jitter = -1;
                spedisci[i].lunghezza = -1;
                spedisci[i].ordine = -1;
                spedisci[i].number_of_pkt = 0;
            }
            statistiche_ref[i].index = 0;
            statistiche_ref[i].min = -1;
            delay = 0;
        }
        // spedisci al server
        send_to_server(sockfd, spedisci);
    }
    // close the socket
    if (sockfd != -1)
        close(sockfd);
    for (i=0; i<num_list; i++)
        free(statistiche_ref[i].pkt_information);
    free(statistiche_ref);
    free(ids_copy);
    free(spedisci);
    printf("Thread statistiche chiude\n");
    return (void*) 0;
}

void* DecoderThread(void* arg){
    printf("Thread decodifica creato\n");
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
            pthread_mutex_lock(&mtx_info);
            if(info)
                info->accept_packet = info->start_seq; //aspetta e aggiorna il last gop, da questo momento non saranno più accettati numeri di sequenza del gop precedente
            pthread_mutex_unlock(&mtx_info);
            *from = el->metadata_start;
            save_GOP(from, el);
            pthread_mutex_lock(&mtx_dec);
            pushList(&testa_dec, &coda_dec, el);
            pthread_cond_signal(&cond_dec);
            pthread_mutex_unlock(&mtx_dec);
        }
        else{
            if (el->gop_over == -1) // esci
                esci = 1;
            else{ // pacchetti da scartare, inviati senza pps e sps
                *from = el->metadata_start;
                rtp* pkt = find_hash(from);
                while (pkt && pkt->slice_type != 5 )
                    pkt = delete_and_get_next(pkt, from, el->next_metadata-1);
                freeGOP((void**)&el);
            }
        }
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
    int num_thr = *(int*) arg, chiudi = 0, count = 0, ret;
    rtp* el = NULL;
    gop_info *copy = NULL;
    printf("Thread che crea GOP creato %d\n", num_thr);
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
                pthread_mutex_lock(&mtx_info);
                if (info) {
                    copy = info;
                    info = setNextElGOP(info->gop_num+1, ret, info->gop_over, info->end_seq, info->next_metadata, info->start_seq);
                    printf("start new gop [%d]\n", info->start_seq);
                }
                pthread_mutex_unlock(&mtx_info);
                
                pthread_mutex_lock(&mtx_ord);
                pushList(&testa_ord, &coda_ord, copy);
                pthread_cond_signal(&cond_ord);
                pthread_mutex_unlock(&mtx_ord);
                
            }
        }
        else {
            chiudi = 1;
            freeRTP((void**)&el);
        }
        count++;
    }
    /* manda l'ultimo GOP */ //probabilmente andrà tolto
    pthread_mutex_lock(&mtx_info);
    copy = info;
    info = NULL;
    pthread_mutex_unlock(&mtx_info);
    
    pthread_mutex_lock(&mtx_ord);
    if (copy)
        pushList(&testa_ord, &coda_ord, copy);
    pthread_cond_signal(&cond_ord);
    pthread_mutex_unlock(&mtx_ord);
    printf("Thread che crea GOP esce %d\n", num_thr);
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
    pcap_loop(handle[*num_thr], -1, sniff, (u_char*) num_thr);
    printf("Thread listener %d stopped sniffing\n", *num_thr);
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, setElRTP(NULL, -1,-1, -1, -1));
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
    SYSFREE(ret,pthread_join(gophandler,NULL),0,"gop 1")
    printf("Thread listener esce %d\n", *num_thr);
    free(num_thr);
    return (void*)0;
}

void sniff(u_char *user, const struct pcap_pkthdr* pkthdr, const u_char* packet){
    num_pkt += 1;
    //setto gli elementi per poter passare i dati a un altro thread
    u_char* buf = malloc(sizeof(u_char)*pkthdr->len);
    memcpy(buf, packet, pkthdr->len);
    //rtphdr* rtpHeader = (rtphdr*) (packet + 4 + sizeof(struct ip) + sizeof(struct udphdr));
    rtp* el= setElRTP(buf, pkthdr->len, pkthdr->ts.tv_sec, num_pkt, *(int*) user);
    pthread_mutex_lock(&mtx_gop);
    pushList(&testa_gop, &coda_gop, el);
    pthread_cond_signal(&cond_gop);
    pthread_mutex_unlock(&mtx_gop);
}
