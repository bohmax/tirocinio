//
//  main.c
//  ImagePicker
//
//  Created by Massimo Puddu on 30/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "utility.h"

char* path_file = NULL; /* definita in utility.h */
char* path_image = NULL;
list* testa_gop = NULL;
list* coda_gop = NULL;
list* testa_dec = NULL;
list* coda_dec = NULL;
pthread_mutex_t mtx_gop = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_gop = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx_dec = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_dec = PTHREAD_COND_INITIALIZER;
pthread_t segnal; //thread listener e segnali
//gop gop_info = { 0 };
int esci = 0;

//thread per la gestione dei segnali per far terminare il programma
void* segnali(void *arg);

//maschera globale dei segnali
static sigset_t sigset_usr;
pcap_t* handle;    /* packet capture handle */

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

pcap_if_t* find_device(pcap_if_t* alldevs, char* name){
    pcap_if_t *d=alldevs;
    while(d!=NULL) {
        if(strncmp(name, d->name, strlen(name))==0)
            return d;
        d=d->next;
    }
    return NULL;
}

void set_handler(char device_name[], struct bpf_program* fp, char filter[], char errbuf[]){
    bpf_u_int32 pMask;            /* subnet mask */
    bpf_u_int32 pNet;             /* ip address*/
    
    handle = pcap_open_live(device_name, BUFSIZ, 0, 5000, errbuf); //ottengo uno sniffer
    if(handle == NULL){
        handle = pcap_open_offline(device_name, errbuf);
        if(handle == NULL){
            printf("pcap_open() failed due to [%s]\n", errbuf);
            exit(1);
        }
    }
    pcap_lookupnet(device_name, &pNet, &pMask, errbuf);
    if(pcap_compile(handle, fp, filter, 0, pNet) == -1){ //usato per compilare la stringa str per il bpf filter
        printf("\npcap_compile() failed\n");
        exit(1);
    }
    // Set the filter compiled above
    if(pcap_setfilter(handle, fp) == -1){
        printf("\npcap_setfilter() failed\n");
        exit(1);
    }
}

void set_signal(){
    int notused;
    ERRORSYSHEANDLER(notused,sigemptyset(&sigset_usr),-1,"NO SIGEMPY 1")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGUSR1),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGINT),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGTERM),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,sigaddset( &sigset_usr, SIGQUIT),-1,"NO ADDSET")
    ERRORSYSHEANDLER(notused,pthread_sigmask(SIG_SETMASK, &sigset_usr, NULL),-1,"NO SIGMAS")
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
            if(workOnPacket(el->packet, el->size, el->n_pkt, info)){ //attualmente è copiato attravarso una alloc, potrei togliere la memcpy
                pthread_mutex_lock(&mtx_dec);
                pushList(&testa_dec, &coda_dec, info);
                pthread_cond_signal(&cond_dec);
                pthread_mutex_unlock(&mtx_dec);
                gop_count++;
                info = setElGOP(gop_count);
            }
        } else
            chiudi = 1;
        freeRTP((void**)&el);
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

int main(int argc, const char * argv[]) {
    clock_t begin = clock();
    int notused; //variabile usata per memorizzare valori di ritorno di alcune chiamate di sistema
    char *dev_name = NULL;   /* capture device name */
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
    char stringfilter[] = "not icmp and udp and dst port 5000";
    pcap_if_t *alldevs = NULL;
    pcap_if_t *device = NULL;
    struct bpf_program fp;        /* to hold compiled program */
    if (argc != 4) exit(EXIT_FAILURE);
    dev_name = (char*) argv[1];
    path_file = (char*) argv[2];
    path_image = (char*) argv[3];
    if(pcap_findalldevs(&alldevs, errbuf)==-1) exit(EXIT_FAILURE);
    set_signal();
    //avvio thread che gestisce i segnali
    SYSFREE(notused,pthread_create(&segnal,NULL,&segnali,NULL),0,"thread")
    pthread_t listener;
    device = find_device(alldevs, dev_name); //se è null provo a leggere offline
    // fetch the network address and network mask
    set_handler(dev_name, &fp, stringfilter, errbuf);
    printf("Sniffing on device: %s\n", dev_name);
    // For every packet received, call the callback function
    // ctrl-c to stop sniffing
    //
    SYSFREE(notused,pthread_create(&listener,NULL,&listenerThread,NULL),0,"thread")
    printf("Aspetta i thread\n");
    SYSFREE(notused,pthread_join(listener,NULL),0,"listener 1")
    pthread_kill(segnal, SIGINT); //manda un segnale al thread
    SYSFREE(notused,pthread_join(segnal,NULL),0,"join 1")
    printf("Uscita dal programma\n");
    pcap_freecode(&fp);
    pcap_close(handle);
    pcap_freealldevs(alldevs);
    freeList(&testa_gop, &coda_gop, &freeRTP);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    //create_image();
    clock_t end = clock();
    printf("Tempo di esecuzione %f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
