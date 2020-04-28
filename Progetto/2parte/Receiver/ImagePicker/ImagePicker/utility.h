//
//  utility.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pcap.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avio.h>
#include "packet_structs.h"
#include "define.h"
#include "struct.h"
#include "struct_fun.h"
#include "receiver.h"
#include "threads.h"
#include "h264toPng.h"
#include "lista.h"
#include "icl_hash.h"

extern FILE* pipe_plot; //pipe per communicare con il grafico delle statistiche
extern struct sockaddr_in servaddr; /* indirizzi per inoltrare i pacchetti udp*/
extern int fd; /* file descriptor del socket */
extern pthread_t *listener, stat_thr,segnal; //thread listener, statistiche e segnali
extern icl_hash_t* hash_packet;       /* hash table contentente i pacchetti da ordinare */
extern pcap_t** handle;    /* packet capture handles */
extern pcap_t* loopback;    /* loopback interface */
extern int from_loopback;   /* booleano, indica se loopback e handle coincidono */
extern int datalink_loopback;   /* intero che indica il tipo del livello 2 del pacchetto */
extern string payload;    /* contiene la codifica di un h264 */
extern string metadata; //dovrà contenere SPS e PPS
extern gop_info* info;  /* utilizzato per creare le immagini */
extern char* path_file;   /* path del file su cui viene salvato lo streaming*/
extern char* path_image;   /* path su cui saranno salvate le immagini*/
extern int esci;            /* indica l'uscita dal programma*/
extern int num_pkt; //numero dei pacchetti arrivati
extern pthread_mutex_t mtx_gop;  /* mutex per fare produttore consumatore con il decodificatore */
extern pthread_cond_t cond_gop;  /* variabile di condizione per produttore consumatore*/
extern pthread_mutex_t mtx_ord;  /* mutex per ordinare i pacchetti */
extern pthread_cond_t cond_ord;  /* variabile di condizione per ordinare i pacchetti*/
extern pthread_mutex_t mtx_dec;  /* mutex per fare produttore consumatore con il decodificatore */
extern pthread_cond_t cond_dec;  /* variabile di condizione per produttore consumatore*/
extern pthread_mutex_t mtx_info;  /* mutex per poter utilizzare la variabile info */
extern pthread_mutex_t mtxhash[HSIZE/DIV]; /* mutex della tabella hash*/
extern pthread_mutex_t mtxstat[NUMLISTTHR];
extern list* testa_gop;
extern list* coda_gop;  /* coda della lista sopra */
extern list* testa_ord; /* lista dei pacchetti */
extern list* coda_ord;
extern list* testa_dec;
extern list* coda_dec;  /* coda della lista sopra */
extern sigset_t sigset_usr;
extern stat_t* statistiche;
extern long delay_calibrator[NUMLISTTHR]; // calcola il delay del primo pacchetto

//inizializzazione socket, ritorna socket fd
int set_stat_sock(void);

//funzione di comparazione per il qsort
int cmpfunc (const void * a, const void * b);

//per calolare la statistica sulla lunghezza della perdita
uint16_t stat_lunghezza(uint16_t arr[], int index);

//per calolare la statistica sul out of order
uint16_t stat_out_of_order(uint16_t arr[], uint16_t current, int index);

// Function designed for chat between client and server.
void send_to_server(int sockfd, send_stat spedisci[]);

// algoritmi presi da https://it.mathworks.com/matlabcentral/fileexchange/37691-psnr-for-rgb-images
//float calculate_PSNR(u_char* src, u_char* dst, int w, int h);
//float calculate_MSE(u_char* src, u_char* dst, int w, int h);

//funzioni di utlità per la tabella hash
void* insert_hash(uint16_t primarykey, void* insert);

void* find_hash(uint16_t* primarykey);

int delete_hash(uint16_t* primarykey);

//per scrivere nel socket delle statistiche
static inline int writen(int fd, void *buf, size_t size) {
    size_t left = size;
    int r=0;
    char *bufptr = (char*)buf;
    while(left>0) {
        r=(int)write(fd ,bufptr,left);
        if (r == -1) {
            if (errno == EINTR || errno==EPIPE) continue;
            return -1;
        }
        if (r == 0) return 0;
        left    -= r;
        bufptr  += r;
    }
    return 1;
}

#endif /* utility_h */
