//
//  string.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef string_h
#define string_h

#include "utility.h"

typedef struct stringa {
    int size;
    u_char value[GOPM];
}string;

typedef struct gop_data {
    int num_frame;
    string payload;
}gop;

/* struttura per passare informazioni su un gop */
typedef struct el_gop {
    //char path[64]; //path su cui sarà salvato il gop
    int gop_num;
    int num_frame;
}gop_info;

/* struttura per passare pacchetti rtp */
typedef struct el_rtp {
    u_char* packet;
    int size;
    int n_pkt;
}rtp;

typedef struct lista{
    void* next;
    void* el;
}list;

/* lista usata per passare pacchetti per decodificare con ffmpeg*/
typedef struct lista_dec {
    struct lista_dec* next;
    AVPacket* packet; //se NULL chiudi il thread
    AVFrame* frame;
    AVCodecContext* context;
    int Nframe;
}list_dec;
#endif /* string_h */
