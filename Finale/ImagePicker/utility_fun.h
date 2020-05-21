//
//  utility_fun.h
//  ImagePicker
//
//  Created by Massimo Puddu on 08/05/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef utility_fun_h
#define utility_fun_h

#include "utility.h"

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

rtp* delete_and_get_next(rtp* el, uint16_t* from, int end_seq);

/* IL CALCOLO DEL PSNR VIENE FATTO SU PYTHON NELLO SCRIPT DEL GRAFICO DEL PSNR NON QUA */
// algoritmi presi da https://it.mathworks.com/matlabcentral/fileexchange/37691-psnr-for-rgb-images
//float calculate_PSNR(u_char* src, u_char* dst, int w, int h);
//float calculate_MSE(u_char* src, u_char* dst, int w, int h);

//funzioni di utlità per la tabella hash
void* insert_hash(uint16_t primarykey, void* insert);

void* find_hash(uint16_t* primarykey);

int delete_hash(uint16_t* primarykey);

//per scrivere nel socket delle statistiche
int writen(int fd, void *buf, size_t size);

#endif /* utility_fun_h */
