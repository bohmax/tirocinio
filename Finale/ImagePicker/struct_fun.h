//
//  struct_fun.h
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef struct_fun_h
#define struct_fun_h

#include "utility.h"

uint16_t* setkeyHash(uint16_t key);

//inizializza le statistiche
stat_t* setStat(void);

/* funzioni per inizializzare e liberare elementi che verrano inseriti nella lista*/
rtp* setElRTP(u_char* buf, int size, double timestamp, int npkt, int from_thr);

void freeRTP(void** el); //list è un elemento occhio al nome

//crea un gop fittizzio da inviare per la chiusura dei thread appositi o come primo thread
gop_info* setElGOP(int gop_num, int start);

//per inizializzare il prossimo gop
gop_info* setNextElGOP(int gop_num, int start, int last_frame_gop, int start_new_gop, int new_metadata, int accept);

void freeGOP(void** el);

void freeORD(void** el);

void freeKeyHash(void* el);

void freeElHash(void* el);

void freeStat(void** el);

#endif /* struct_fun_h */
