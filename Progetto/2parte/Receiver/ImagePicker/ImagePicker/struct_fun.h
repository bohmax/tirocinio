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

int* setkeyHash(int key);

/* funzioni per inizializzare e liberare elementi che verrano inseriti nella lista*/
rtp* setElRTP(u_char* buf, int size, int npkt);

void freeRTP(void** el); //list è un elemento occhio al nome

gop_info* setElGOP(int gop_num, int start);

void freeGOP(void** el);

void freeORD(void** el);

void freeKeyHash(void* el);

void freeElHash(void* el);

#endif /* struct_fun_h */
