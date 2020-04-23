//
//  struct.c
//  ImagePicker
//
//  Created by Massimo Puddu on 08/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "struct_fun.h"

uint16_t* setkeyHash(uint16_t key){
    uint16_t* num = malloc(sizeof(uint16_t));
    *num = key;
    return num;
}

stat_t* setStat(){
    stat_t* new = malloc(sizeof(stat_t));
    memset(new, 0, sizeof(stat_t));
    return new;
}

gop_info* setElGOP(int gop_num, int start){
    gop_info* new = malloc(sizeof(gop_info));
    memset(new, '\0', sizeof(gop_info));
    new->gop_num = gop_num;
    new->start_seq = start;
    new->num_frame = 0;
    new->end_seq = 0;
    return new;
}

rtp* setElRTP(u_char* buf, int size, long timestamp, int npkt){
    rtp* new = malloc(sizeof(rtp));
    memset(new, '\0', sizeof(rtp));
    new->packet = buf;
    new->size = size;
    new->r_timestamp = timestamp;
    new->n_pkt = npkt;
    new->nal_type = 0;
    new->state = 0;
    new->decoder = 0;
    new->sent = 0;
    return new;
}

void freeGOP(void** el){
    gop_info* info = (gop_info*) (*el);
    free(info);
    info = NULL;
}

void freeORD(void** el){
    //rtp* todestroy = (rtp*) (*el);
    //free(todestroy->packet);
    //free(todestroy);
    //todestroy = NULL;
}

void freeRTP(void** el){
    rtp* todestroy = (rtp*) (*el);
    free(todestroy->packet);
    free(todestroy);
    todestroy = NULL;
}

void freeStat(void** el){
    stat_t* stat = (stat_t*) (*el);
    free(stat);
    stat = NULL;
}

void freeKeyHash(void* el){
    int* todestroy = (int*) el;
    free(todestroy);
    todestroy = NULL;
}

void freeElHash(void* el){
    freeRTP(&el);
}
