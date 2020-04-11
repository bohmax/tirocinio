//
//  main.c
//  ImagePicker
//
//  Created by Massimo Puddu on 30/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "utility.h"

icl_hash_t* hash_packet = NULL; /* definita tutto in utility.h */
char* path_file = NULL;
char* path_image = NULL;
list* testa_gop = NULL;
list* coda_gop = NULL;
list* testa_ord = NULL;
list* coda_ord = NULL;
list* testa_dec = NULL;
list* coda_dec = NULL;
string metadata; //dovrà contenere SPS e PPS
string payload; //dovrà contenere un intero GOP
pthread_mutex_t mtx_gop = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_gop = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx_dec = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_dec = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx_ord = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ord = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtxhash[HSIZE/DIV];
sigset_t sigset_usr; /* maschera globale dei segnali */
pcap_t* handle;    /* packet capture handle */
pthread_t segnal; //thread listener e segnali
int esci = 0;
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

void inizialize_thread(){
    int notused, size = HSIZE/DIV;
    for(int i=0; i < size; i++){
        if((notused=pthread_mutex_init(&mtxhash[i], NULL)<0)){
            perror("impossibile inizializzare mutex");
            exit(errno);
        }
    }
}

int main(int argc, const char * argv[]) {
    clock_t begin = clock();
    int notused; //variabile usata per memorizzare valori di ritorno di alcune chiamate di sistema
    char *dev_name = NULL;   /* capture device name */
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
    char stringfilter[] = "not icmp and udp and dst port 5000";
    pcap_if_t *alldevs = NULL;
    pcap_if_t *device = NULL;
    pthread_t listener, order;
    struct bpf_program fp;        /* to hold compiled program */
    if (argc != 4) exit(EXIT_FAILURE);
    dev_name = (char*) argv[1];
    path_file = (char*) argv[2];
    path_image = (char*) argv[3];
    if(pcap_findalldevs(&alldevs, errbuf)==-1) exit(EXIT_FAILURE);
    set_signal();
    inizialize_thread();
    hash_packet = icl_hash_create(HSIZE, NULL, NULL);
    //avvio thread che gestisce i segnali
    SYSFREE(notused,pthread_create(&segnal,NULL,&segnali,NULL),0,"thread")
    device = find_device(alldevs, dev_name); //se è null provo a leggere offline
    // fetch the network address and network mask
    set_handler(dev_name, &fp, stringfilter, errbuf);
    printf("Sniffing on device: %s\n", dev_name);
    // For every packet received, call the callback function
    // ctrl-c to stop sniffing
    //
    SYSFREE(notused,pthread_create(&order,NULL,&ReaderPacket,NULL),0,"thread")
    SYSFREE(notused,pthread_create(&listener,NULL,&listenerThread,NULL),0,"thread")
    printf("Aspetta i thread\n");
    SYSFREE(notused,pthread_join(listener,NULL),0,"listener 1")
    SYSFREE(notused,pthread_join(order,NULL),0,"listener 1")
    pthread_kill(segnal, SIGINT); //manda un segnale al thread
    SYSFREE(notused,pthread_join(segnal,NULL),0,"join 1")
    printf("Uscita dal programma\n");
    pcap_freecode(&fp);
    pcap_close(handle);
    pcap_freealldevs(alldevs);
    freeList(&testa_gop, &coda_gop, &freeRTP);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    freeList(&testa_ord, &coda_ord, &freeORD);
    icl_hash_destroy(hash_packet, &freeKeyHash, &freeElHash);
    //create_image();
    clock_t end = clock();
    printf("Tempo di esecuzione %f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
