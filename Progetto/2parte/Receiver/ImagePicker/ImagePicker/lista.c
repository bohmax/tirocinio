//
//  lista.c
//  ImagePicker
//
//  Created by Massimo Puddu on 04/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "lista.h"

gop_info* setElGOP(int gop_num){
    gop_info* new = malloc(sizeof(gop_info));
    memset(new, '\0', sizeof(gop_info));
    new->gop_num = gop_num;
    new->num_frame = 0;
    return new;
}

void freeGOP(void** el){
    gop_info* info = (gop_info*) (*el);
    free(info);
    *el = NULL;
}

rtp* setElRTP(u_char* buf, int size, int npkt){
    rtp* new = malloc(sizeof(rtp));
    memset(new, '\0', sizeof(rtp));
    new->packet = buf;
    new->size = size;
    new->n_pkt = npkt;
    return new;
}

void freeList(list** head, list** coda, void(* del)(void** el)){
    while (*head) {
        void* el = popList(head, coda);
        del(&el);
    }
}

void pushList(list** head, list** coda, void* el){
    list* new = malloc(sizeof(list));
    new->next = NULL;
    new->el = el;
    if (*head == NULL){
        *head = new;
        *coda = *head;
    }
    else{
        (*coda)->next = new;
        *coda = new;
    }
}

void* popList(list** head, list** coda){
    list* ret = *head;
    void* el = ret->el;
    *head = (*head)->next; // se GOPM è troppo piccolo qua ci sarà un segfault
    free(ret);
    if (*head == NULL)
        *coda = NULL;
    return el;
}

void freeRTP(void** el){
    rtp* todestroy = (rtp*) (*el);
    free(todestroy->packet);
    free(*el);
    *el = NULL;
}

void pushListDec(list_dec** head, list_dec** coda, AVFrame* frame, int nFrame){
    list_dec* new = malloc(sizeof(list_dec));
    new->Nframe = nFrame;
    new->frame = frame;
    new->next = NULL;
    if (*head == NULL){
        *head = new;
        *coda = new;
    }
    else{
        (*coda)->next = new;
        *coda = new;
    }
}

list_dec* popListDec(list_dec** head, list_dec** coda){
    list_dec* ret = *head;
    *head = (*head)->next;
    if (*head == NULL)
        *coda = NULL;
    return ret;
}

void freeListDec(list_dec** el) {
    free(*el);
    *el = NULL;
}
