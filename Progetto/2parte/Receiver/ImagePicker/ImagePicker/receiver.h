//
//  receiver.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef receiver_h
#define receiver_h

#include "utility.h"

//inizializza buff per contenere frame video
void starter(string* stringa);

//concatena dst con src a partire da dst_len e concatena la src fino a src_len
void add(string* dst, u_char src[], int src_len);

//Crea l'header per tutti i pacchetti dello streaming
void create_header_information(u_char* rtpdata, unsigned int fragment_type, int rtpdata_len);

//aggiungi i dati di un rtp al payload
//ritorna uno se il gop è terminato 0 altrimenti
int addPacketToGOP(u_char* rtpdata, int rtpdata_len, int num_pkt, gop_info* info);

//stub per chiamare addPacket e e create_header in autonomia
//ritorna uno se il gop è terminato 0 altrimenti
int workOnPacket(rtp* el, gop_info* info);

#endif /* receiver_h */
