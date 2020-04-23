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

//inizializzazione socket, ritorna socket fd
int set_stat_sock(){
    int sockfd;
    struct sockaddr_in servaddr;
    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(HOSTNAME);
    servaddr.sin_port = htons(DPORT);
    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
          printf("connected to the server..\n");
    return sockfd;
}

//funzione di comparazione per il qsort
int cmpfunc (const void * a, const void * b){
   return ( *(uint16_t*)a - *(uint16_t*)b );
}

// Function designed for chat between client and server.
void func(int sockfd){
    char buff[80];
    int n;
    // infinite loop for chat
    for (;;) {
        bzero(buff, 80);
  
        // read the message from client and copy it in buffer
        read(sockfd, buff, sizeof(buff));
        // print buffer which contains the client contents
        printf("From client: %s\t To client : ", buff);
        bzero(buff, 80);
        n = 0;
        // copy server message in the buffer
        while ((buff[n++] = getchar()) != '\n')
            ;
  
        // and send that buffer to client
        write(sockfd, buff, sizeof(buff));
  
        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}
#endif /* threads_h */
