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
    int start_seq; //inizio del gop da analizzare
    int end_seq; //fine gop
    int num_frame;
}gop_info;

/* struttura per passare pacchetti rtp */
typedef struct el_rtp {
    u_char* packet;
    int size;
    long r_timestamp; //timestamp lato receiver
    int n_pkt;
    unsigned int nal_type; //viene calcolato in addPacketToGOP
    unsigned int state; //128 se è lo start, 64 se è la fine, 0 altrimenti
    unsigned int decoder; //se state == 128 allora contiene le informazioni per decodificare il frame
    int sent; /* permette di sapere se il pacchetto è stato spedito */
}rtp;

typedef struct{
    uint16_t perdita; //tasso di perdita
    uint16_t lunghezza; //lunghezza media perdita
    int delay; //delay medio della finestra
    uint16_t ordine; //numero di pacchetti fuori ordine
}send_stat;

typedef struct stats{
    int perdita; //tasso di perdita
    int lunghezza; //lunghezza media perdita
    int delay; //delay medio della finestra
    int ordine; //numero di pacchetti fuori ordine
    uint16_t min; //pacchetto con id più piccolo
    uint16_t max; //pacchetto con id più grande
    uint16_t id_accepted; //non si accettano id di un finestra passata, questo valore diventa il max value della finestra precedemte
    uint16_t ids[DIMARRSTAT];
    uint8_t index; //ultimo elemento inserito
}stat_t;

typedef struct lista{
    void* next;
    void* el;
}list;

#endif /* string_h */
