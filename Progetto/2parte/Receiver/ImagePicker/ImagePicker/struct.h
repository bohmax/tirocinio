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

typedef struct packet_info {
    uint16_t ids; // id di un pacchetto
    double pkt_timestamp; // timestamp lato sender
    long r_timestamp; // timestamp lato receiver
} pkt_info;

/* struttura per passare informazioni su un gop */
typedef struct el_gop {
    //char path[64]; //path su cui sarà salvato il gop
    int gop_num;
    int start_seq; //inizio del gop da analizzare
    int end_seq; //fine gop
    int num_frame;
    int accept_packet; // contiene il numero di sequenza con cui si decide se analizzare un pacchetto o meno
    int gop_over; //1 se non ci sono ulteriori modifiche da fare al gop, 0 altrimenti
    int end_last_gop; // quando finisce il gop precedente, utili per avere più informazioni quando se ne inizia un altro
    int accepted28; // mi indica da che numero di sequenza accettare i nal type 28
    int after_5; // indica se ho ricevuto un pacchetto con nal type diverso da 5
    int startP; // inizio del nal type di tipo 28, utile per sapere quando il nal_type 5 finisce
    int metadata_start; // indica da dove inizia il pacchetto sps e pps
    int next_metadata; // mi indica il punto di inizio del prossimo gop
    uint16_t losted_packet[DIMARRSTAT]; //array dove ogni entry indica che pacchetti sono stati persi
    int len_losted[DIMARRSTAT/2]; // ogni entry contiene il numero la lunghezza dei pacchetti persi dell i-esimo frame
    int index_losted; // indica l'ultimo elemento usato da losted_packet
    int index_len, incremented; // indica l'ultimo elemento usato da len_losted, la seconda variabile mi dice se l'indice è stato incrementato da un pacchetto che era la fine del frame
}gop_info;

/* struttura per passare pacchetti rtp */
typedef struct el_rtp {
    u_char* packet;
    int size;
    long r_timestamp; //timestamp lato receiver
    int n_pkt;
    int from_thr;
    unsigned int slice_type; //viene calcolato in addPacketToGOP
    unsigned int state; //128 se è lo start, 64 se è la fine, 0 altrimenti
    unsigned int decoder; //se state == 128 allora contiene le informazioni per decodificare il frame
    int sent; /* permette di sapere se il pacchetto è stato spedito */
}rtp;

typedef struct{
    uint16_t perdita; //tasso di perdita
    uint16_t lunghezza; //lunghezza media perdita
    float delay; //delay medio della finestra
    float jitter;
    uint16_t ordine; //numero di pacchetti fuori ordine
    int number_of_pkt; //numero di pacchetti ricevuti
}send_stat;

typedef struct stats{
    uint16_t min; //pacchetto con id più piccolo
    uint16_t max; //pacchetto con id più grande
    uint16_t id_accepted; //non si accettano id di un finestra passata, questo valore diventa il max value della finestra precedemte
    pkt_info* pkt_information; // contiene i dati utili per calcolare le statistiche di un pacchetto
    uint8_t index; //ultimo elemento inserito
}stat_t;

typedef struct lista{
    void* next;
    void* el;
}list;

#endif /* string_h */
