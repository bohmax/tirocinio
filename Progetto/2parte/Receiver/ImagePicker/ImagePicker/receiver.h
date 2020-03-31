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

//callback del thread che sniffa i pacchetti
void sniff(u_char *useless,const struct pcap_pkthdr* pkthdr,const u_char*
              packet);
#endif /* receiver_h */
