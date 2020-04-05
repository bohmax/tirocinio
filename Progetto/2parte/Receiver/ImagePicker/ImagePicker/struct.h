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

typedef struct lista {
    struct lista* next;
    AVPacket* packet; //se NULL chiudi il thread
    AVFrame* frame;
    AVCodecContext* context;
    int Nframe;
}list;
#endif /* string_h */
