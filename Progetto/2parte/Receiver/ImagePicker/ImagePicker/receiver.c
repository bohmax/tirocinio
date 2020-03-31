//
//  receiver.c
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "receiver.h"

int inizialized = 0; //ci dice se è stato trovato SPS e PPS
int num_pkt = 0; //numero dei pacchetti arrivati
int num_frame = 0; //numero di frame analizzati
int num_gop = 0; //numero di GOP trovati

void starter(string* stringa){
    int zero = 0, uno = 1;
    memcpy(stringa->value+stringa->size, &zero, 1);
    memcpy(stringa->value+stringa->size+1, &zero, 1);
    memcpy(stringa->value+stringa->size+2, &uno, 1);
    stringa->size += 3;
}

void add(string* dst, u_char src[], int src_len){
    memcpy(dst->value+dst->size, src, src_len);
    dst->size += src_len;
}

void sniff(u_char *useless,const struct pcap_pkthdr* pkthdr,const u_char*
              packet){
    num_pkt += 1;
    int to_rtphdr = (sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr));
    int to_rtpdata = to_rtphdr + sizeof(rtphdr);
    rtphdr* rtpHeader = (rtphdr*)(packet + to_rtphdr);
    //printf("RTP number [%d], timestamp of this packet is: %d\n", ntohs(rtpHeader->seq_num), ntohl(rtpHeader->TS)); //function converts the unsigned short integer netshort from network byte order to host byte order.
    u_char* rtpdata =(u_char*) (packet + to_rtpdata);
    //FU - A - -HEADER
    unsigned int forbidden = rtpdata[0] & 0x80 >> 7;
    unsigned int nri = rtpdata[0] & 0x60 >> 5;
    unsigned int fragment_type = rtpdata[0] & 0x1F;  // il valore è 28
    int rtpdata_len = pkthdr->len - to_rtpdata;
    
    if(!inizialized){
        if(fragment_type == 7){
            starter(&metadata);
            add(&metadata, rtpdata, rtpdata_len);  // SPS information
        }
        else if (fragment_type == 8){
            starter(&metadata);
            add(&metadata, rtpdata, rtpdata_len);  // PPS information
            inizialized = 1;
        }
        return;
    }
    
    //SPS e PPS ricevuti inizio a creare il frame
    if (fragment_type == 28){
        //Nal header
        unsigned int start_bit = rtpdata[1] & 0x80;  // 128 se e' il primo pacchetto del frame 0 altrimenti
        unsigned int end_bit = rtpdata[1] & 0x40;  // 64 se e' l ultimo pacchetto 0 altrimenti
        unsigned int reserved = rtpdata[1] & 0x20 >> 5;
        unsigned int nal_type = rtpdata[1] & 0x1F;
        
        if (start_bit == 128){
            if (nal_type == 5){
                printf("start new gop %d\n", num_pkt);
                if (payload.size != 0){
                    FILE* f = fopen("/Users/maxuel/Desktop/hope", "wb");
                    fwrite(payload.value, 1, payload.size, f);
                    fclose(f);
                    num_gop += 1;
                    memset(payload.value, '\0', payload.size); //inizializzo gop
                    payload.size = 0;
                    if (num_gop == 2){
                        pcap_breakloop(handle);
                    }
                }
            }
            add(&payload, metadata.value, metadata.size);
            unsigned int idr_nal = rtpdata[0] & 0xE0;  // 3 NAL UNIT BITS
            unsigned int nal = idr_nal | nal_type;  // [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
            starter(&payload);
            memcpy(payload.value+payload.size, &nal, 1); // header per il frame
            payload.size += 1;
        }
        else if (end_bit == 64)
            num_frame += 1;  // solo per dati statistici
        
        add(&payload, rtpdata+2, rtpdata_len-2);  // rimuove gli header del payload e contiene solo i dati successivi
    }
}
