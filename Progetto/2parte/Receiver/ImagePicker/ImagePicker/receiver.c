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

void create_header_information(u_char* rtpdata, unsigned int fragment_type, int rtpdata_len){
    if(fragment_type == 7){
        starter(&metadata);
        add(&metadata, rtpdata, rtpdata_len);  // SPS information
    }
    else if (fragment_type == 8){
        starter(&metadata);
        add(&metadata, rtpdata, rtpdata_len);  // PPS information
        inizialized = 1;
    }
}

int addPacketToGOP(u_char* rtpdata, int rtpdata_len, int num_pkt, gop_info* info){
    int return_value = 0;
    //Nal header
    unsigned int start_bit = rtpdata[1] & 0x80;  // 128 se e' il primo pacchetto del frame 0 altrimenti
    unsigned int end_bit = rtpdata[1] & 0x40;  // 64 se e' l ultimo pacchetto 0 altrimenti
    unsigned int reserved = rtpdata[1] & 0x20 >> 5;
    unsigned int nal_type = rtpdata[1] & 0x1F;
    
    if (start_bit == 128){
        if (nal_type == 5){
            printf("start new gop %d\n", num_pkt);
            if (payload.size != 0){
                FILE* f = NULL;
                char GOPName[64];
                sprintf(GOPName, "%s-%06d", path_file, info->gop_num);
                if ((f = fopen(GOPName, "w"))){
                    fwrite(payload.value, 1, payload.size, f);
                    fclose(f);
                } else printf("Error: %s\n", strerror(errno));
                
                //if (num_gop == 1){
                    //pcap_breakloop(handle);
                //    return 1;
                //}
                //else{
                //info->num_frame = 0;
                //info->gop_num = num_gop;
                payload.size = 0;
                //}
                num_gop ++;
                return_value = 1;
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
        info->num_frame++;  // solo per dati statistici
    
    add(&payload, rtpdata+2, rtpdata_len-2);  // rimuove gli header del payload e contiene solo i dati successivi
    return return_value;
}

void* insert_hash(int primarykey, void* insert){
    int* key = setkeyHash(primarykey);
    int buck = hash_pjw(key) % hash_packet->nbuckets;
    int mtxi = buck / DIV;
    pthread_mutex_lock(&mtxhash[mtxi]);
    void* ris = icl_hash_insert(hash_packet, key, insert);
    pthread_mutex_unlock(&mtxhash[mtxi]);
    return ris;
}

int workOnPacket(rtp* el, gop_info* info){
    
    int to_rtphdr = sizeof(struct udphdr);
    int to_rtpdata = to_rtphdr + sizeof(rtphdr);
    int rtpdata_len = el->size - to_rtpdata;
    rtphdr* rtpHeader = (rtphdr*)(el->packet + to_rtphdr);
    int seq = ntohs(rtpHeader->seq_num);
    if(!insert_hash(seq, el)){ //elemento già inserito ritorna
        freeRTP((void**)&el);
        return 0;
    }
    //printf("RTP number [%d], timestamp of this packet is: %d\n", ntohs(rtpHeader->seq_num), ntohl(rtpHeader->TS)); //function converts the unsigned short integer netshort from network byte order to host byte order.
    u_char* rtpdata =(u_char*) (el->packet + to_rtpdata);
    //FU - A - -HEADER
    unsigned int forbidden = rtpdata[0] & 0x80 >> 7;
    unsigned int nri = rtpdata[0] & 0x60 >> 5;
    unsigned int fragment_type = rtpdata[0] & 0x1F;  // il valore è 28
    
    if(!inizialized)
        create_header_information(rtpdata, fragment_type, rtpdata_len);
    //SPS e PPS ricevuti inizio a creare il GOP
    else if (fragment_type == 28) //dati per il GOP
        return addPacketToGOP(rtpdata, rtpdata_len, num_pkt, info);
    return 0;
}
