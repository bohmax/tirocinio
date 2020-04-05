//
//  lista.c
//  ImagePicker
//
//  Created by Massimo Puddu on 04/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "lista.h"

void pushList(list** head, list** coda, AVFrame* frame, int nFrame){
    list* new = malloc(sizeof(list));
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

list* popList(list** head, list** coda){
    list* ret = *head;
    *head = (*head)->next;
    if (*head == NULL)
        *coda = NULL;
    return ret;
}

void freeList(list** el) {
    free(*el);
    *el = NULL;
}
