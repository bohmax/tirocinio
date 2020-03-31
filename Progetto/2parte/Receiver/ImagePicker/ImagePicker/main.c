//
//  main.c
//  ImagePicker
//
//  Created by Massimo Puddu on 30/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "utility.h"

char* path_file = NULL; /* definita in utility.h */

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

void* listenerThread(void* arg){
    pcap_loop(handle, -1, sniff, NULL);
    printf("Thread in uscita\n");
    return (void*)0;
}

int main(int argc, const char * argv[]) {
    int notused; //variabile usata per memorizzare valori di ritorno di alcune chiamate di sistema
    char *dev_name = NULL;   /* capture device name */
    char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
    char stringfilter[] = "not icmp and udp and dst port 5000";
    pcap_if_t *alldevs = NULL;
    pcap_if_t *device = NULL;
    struct bpf_program fp;        /* to hold compiled program */
    pthread_t listener, segnal; //thread listener e segnali
    if (argc != 3) exit(EXIT_FAILURE);
    dev_name = (char*) argv[1];
    path_file = (char*) argv[2];
    if(pcap_findalldevs(&alldevs, errbuf)==-1) exit(EXIT_FAILURE);
    set_signal();
    //avvio thread che gestisce i segnali
    SYSFREE(notused,pthread_create(&segnal,NULL,&segnali,NULL),0,"thread")
    
    device = find_device(alldevs, dev_name); //se è null provo a leggere offline
    // fetch the network address and network mask
    set_handler(dev_name, &fp, stringfilter, errbuf);
    printf("Sniffing on device: %s\n", dev_name);
    // For every packet received, call the callback function
    // ctrl-z to stop sniffing
    SYSFREE(notused,pthread_create(&listener,NULL,&listenerThread,NULL),0,"thread")
    printf("Aspetta i thread\n");
    SYSFREE(notused,pthread_join(listener,NULL),0,"listener 1")
    pthread_kill(segnal, SIGINT); //manda un segnale al thread
    SYSFREE(notused,pthread_join(segnal,NULL),0,"join 1")
    printf("Uscita dal programma\n");
    pcap_freecode(&fp);
    pcap_close(handle);
    pcap_freealldevs(alldevs);
    return 0;
}
