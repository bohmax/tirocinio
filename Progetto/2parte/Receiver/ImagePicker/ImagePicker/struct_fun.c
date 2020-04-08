//
//  struct.c
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "struct_fun.h"

int* setkeyHash(int key){
    int* num = malloc(sizeof(int));
    *num = key;
    return num;
}

gop_info* setElGOP(int gop_num){
    gop_info* new = malloc(sizeof(gop_info));
    memset(new, '\0', sizeof(gop_info));
    new->gop_num = gop_num;
    new->num_frame = 0;
    return new;
}

rtp* setElRTP(u_char* buf, int size, int npkt){
    rtp* new = malloc(sizeof(rtp));
    memset(new, '\0', sizeof(rtp));
    new->packet = buf;
    new->size = size;
    new->n_pkt = npkt;
    return new;
}

void freeGOP(void** el){
    gop_info* info = (gop_info*) (*el);
    free(info);
    info = NULL;
}

void freeRTP(void** el){
    rtp* todestroy = (rtp*) (*el);
    free(todestroy->packet);
    free(todestroy);
    todestroy = NULL;
}

void freeKeyHash(void* el){
    int* todestroy = (int*) el;
    free(todestroy);
    todestroy = NULL;
}

void freeElHash(void* el){
    u_char* todestroy = (u_char*) el;
    free(todestroy);
    todestroy = NULL;
}
