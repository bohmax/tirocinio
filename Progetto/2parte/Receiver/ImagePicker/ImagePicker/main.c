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
pcap_t** handle;    /* packet capture handle */
pcap_t* loopback;    /* loopback interface */
pthread_t *listener, stat_thr, segnal; //thread listener, statistiche e segnali
int esci = 0;
int from_loopback = 0;
int datalink_loopback = 0;
struct sockaddr_in servaddr;
int fd;
stat_t* statistiche; //statistiche dei thread listener

void get_loopback(pcap_if_t* alldevs,char name[] ,char errbuf[]){
    pcap_if_t *d=alldevs;
    while(d!=NULL) {
        if(d->flags == PCAP_IF_LOOPBACK || d->flags == 55){
            if (!strcmp(d->name, name)){
                from_loopback = 1;
                loopback = handle[0];
            }
            else{
                loopback = pcap_open_live(d->name, BUFSIZ, 0, 5000, errbuf); //ottengo uno sniffer
                if(loopback == NULL){
                    printf("pcap_open() loopback failed due to [%s]\n", errbuf);
                    exit(1);
                }
            }
            datalink_loopback = pcap_datalink(loopback);
            return;
        }
        d=d->next;
    }
    printf("Cannot find loopback interface, other thank loopback intrface you need a loopback with DLT_NULL link type header\n");
    exit(1);
}

void set_handler(char device_name[], int index, struct bpf_program* fp, char filter[], char errbuf[]){
    bpf_u_int32 pMask;            /* subnet mask */
    bpf_u_int32 pNet;             /* ip address*/
    handle[index] = pcap_open_live(device_name, MAXBUF, 0, 100, errbuf); //ottengo uno sniffer
    if(handle == NULL){
        handle[index] = pcap_open_offline(device_name, errbuf);
        if(handle == NULL){
            printf("pcap_open() failed due to [%s]\n", errbuf);
            exit(1);
        }
    }
    pcap_lookupnet(device_name, &pNet, &pMask, errbuf);
    if(pcap_compile(handle[index], fp, filter, 0, pNet) == -1){ //usato per compilare la stringa str per il bpf filter
        printf("\npcap_compile() failed\n");
        exit(1);
    }
    // Set the filter compiled above
    if(pcap_setfilter(handle[index], fp) == -1){
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

void set_socket(){
    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd<0){
        perror("cannot open socket");
        exit(-1);
    }
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(HOSTNAME);
    servaddr.sin_port = htons(DPORT);
}

int main(int argc, const char * argv[]) {
    clock_t begin = clock();
    int notused; //variabile usata per memorizzare valori di ritorno di alcune chiamate di sistema
    char *dev_name = NULL;   /* capture device name */
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
    char* stringfilter = malloc(64);
    memset(stringfilter, '0', 64);
    pcap_if_t *alldevs = NULL;
    statistiche = malloc(sizeof(stat_t)*NUMLISTTHR);
    listener = malloc(sizeof(pthread_t)*NUMLISTTHR);
    handle = malloc(sizeof(pcap_t*)*NUMLISTTHR);
    memset(statistiche, 0, sizeof(stat_t)*NUMLISTTHR);
    pthread_t order;
    
    struct bpf_program fp;        /* to hold compiled program */
    if (argc != 4) exit(EXIT_FAILURE);
    dev_name = (char*) argv[1];
    path_file = (char*) argv[2];
    path_image = (char*) argv[3];
    if(pcap_findalldevs(&alldevs, errbuf)==-1) exit(EXIT_FAILURE);
    set_socket();
    set_signal();
    inizialize_thread();
    hash_packet = icl_hash_create(HSIZE, uint16_hash_function, uint_16t_key_compare);
    //avvio thread che gestisce i segnali
    SYSFREE(notused,pthread_create(&segnal,NULL,&segnali,NULL),0,"thread")
    SYSFREE(notused,pthread_create(&stat_thr,NULL,&statThread,NULL),0,"thread")
    SYSFREE(notused,pthread_create(&order,NULL,&ReaderPacket,NULL),0,"thread") // ctrl-c to stop sniffing
    //device = find_device(alldevs, dev_name); //se è null provo a leggere offline
    printf("Sniffing on device: %s\n", dev_name);
    for (int i=0; i<NUMLISTTHR; i++){
        // fetch the network address and network mask
        int* id = malloc(sizeof(int));
        *id = i;
        sprintf(stringfilter, "not icmp and udp and dst port %d", (5000 + i));
        set_handler(dev_name, i, &fp, stringfilter, errbuf);
        SYSFREE(notused,pthread_create(&listener[i],NULL,&listenerThread,id),0,"thread")
    }
    get_loopback(alldevs, dev_name, errbuf);
    printf("Aspetta i thread\n");
    for (int i=0; i<NUMLISTTHR; i++)
        SYSFREE(notused,pthread_join(listener[i],NULL),0,"listener 1")
    /* libera lista e invia segnale di chiusura a order */
    pthread_mutex_lock(&mtx_ord);
    pushList(&testa_ord, &coda_ord, setElGOP(-1, -1));
    pthread_cond_signal(&cond_ord);
    pthread_mutex_unlock(&mtx_ord);
    SYSFREE(notused,pthread_join(order,NULL),0,"join order")
    SYSFREE(notused,pthread_join(stat_thr,NULL),0,"join statistiche")
    pthread_kill(segnal, SIGINT); //manda un segnale al thread
    SYSFREE(notused,pthread_join(segnal,NULL),0,"join 1")
    printf("Uscita dal programma\n");
    pcap_freecode(&fp);
    for (int i = 0; i < NUMLISTTHR; i++) {
        pcap_close(handle[i]);
    }
    if (!from_loopback)
        pcap_close(loopback);
    pcap_freealldevs(alldevs);
    freeList(&testa_gop, &coda_gop, &freeRTP);
    freeList(&testa_dec, &coda_dec, &freeGOP);
    freeList(&testa_ord, &coda_ord, &freeGOP);
    icl_hash_destroy(hash_packet, &freeKeyHash, &freeElHash);
    free(handle);
    free(statistiche);
    free(listener);
    free(stringfilter);
    clock_t end = clock();
    close(fd);
    printf("Tempo di esecuzione %f\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
