//
//  define.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef define_h
#define define_h

#define ERRORSYSHEANDLER(r,c,d,e) if((r=c)==d) { perror(e);exit(errno); }
#define SYSFREE(r,c,d,e) if((r=c)!=d) { perror(e);exit(errno); }
#define MAXBUF 1442 /* dimensione per ogni pacchetto di libpcap */
#define DPORT 9998 /* destination port del lettore video */
#define LFINESTRA 200000 /* indica la lunghezza della finestra, cioè il tempo che deve trascorre per inviare le statistiche */
//#define DIM 500 //dimensione media per 2 di sps e pps
#define DIMARRSTAT 1024 //dimensione array statistiche
#define GOPM 2000000 //dimensioni di un GOP medio moltiplicato per 2
#define HSIZE 4000 /* dimesione della tebella hash contentente i pacchetti. dimensione basata sul gop medio */
#define DIV 50 //grandezza partizione della tebella hash
#define NUMLISTTHR 2 /* numero di thread listener */
#define NUMDECODERTHR 1 //numero di thread per decodificare le immagini
#define MAC_LENGHT 6//lunghezza mac address
#define MAC_ADDR    0x00 //mac address da settare su un pacchetto ethernet da inviare a loopback
#define HOSTNAME "127.0.0.1" //host name su cui inoltrare i pacchetti rtp
#define DIM_IP 15 // dimensione di un ip in formato stringa
#define DIM_TIMESTAMP 8 // dimensione del timestamp aggiunto nei pacchetti
#define JITTER_DIV 16
typedef int make_iso_compilers_happy;

#endif /* define_h */
