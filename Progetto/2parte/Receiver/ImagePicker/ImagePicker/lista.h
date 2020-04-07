//
//  lista.h
//  ImagePicker
//
//  Created by Massimo Puddu on 04/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef lista_h
#define lista_h

#include "utility.h"

void pushList(list** head, list** coda, void* el);

void* popList(list** head, list** coda);

void freeList(list** head, list** coda, void(* del)(void** el));

/* funzioni per inizializzare e liberare elementi che verrano inseriti nella lista*/
rtp* setElRTP(u_char* buf, int size, int npkt);

void freeRTP(void** el); //list è un elemento occhio al nome

gop_info* setElGOP(int gop_num);

void freeGOP(void** el);

/* funzioni per la gestione della lista per decodificare */
/* per ora non è possibile farlo quindi è stato commentato dove usato con la vecchi nomenclature list */
/* era stato fatto per il multi thread ma è meglio usare quello previsto da ffmpeg */
/*void pushListDec(list_dec** head, list_dec** coda, AVFrame* frame, int nFrame);

list_dec* popListDec(list_dec** head, list_dec** coda);

void freeListDec(list_dec** el);*/

#endif /* lista_h */
