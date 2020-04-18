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
int current_seq = 0; /* ultimo numero di sequenza letto più grande */
int start_gop = 0; /* inizio di un nuovo gop */
int end_new_gop = 0; /* aggiorna quando finisce il nuovo gop */
int to_iphdr = sizeof(struct ether_header);
int to_udphdr = sizeof(struct ether_header) + sizeof(sniff_ip_t);
int to_rtphdr = sizeof(struct ether_header) + sizeof(sniff_ip_t) + sizeof(struct udphdr);
int to_rtpdata = sizeof(struct ether_header) + sizeof(sniff_ip_t) + sizeof(struct udphdr) + sizeof(rtphdr);

void loopbackstarter(u_char* buff){
    int zero = 0, due = 2;
    memcpy(buff, &due, 1);
    memcpy(buff+1, &zero, 1);
    memcpy(buff+2, &zero, 1);
    memcpy(buff+3, &zero, 1);
}

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

int addPacketToGOP(u_char* rtpdata, int rtpdata_len, uint16_t seq_num, rtp* el, gop_info* info){
    int return_value = 0;
    //Nal header
    unsigned int start_bit = rtpdata[1] & 0x80;  // 128 se e' il primo pacchetto del frame 0 altrimenti
    unsigned int end_bit = rtpdata[1] & 0x40;  // 64 se e' l ultimo pacchetto 0 altrimenti
    unsigned int reserved = rtpdata[1] & 0x20 >> 5;
    unsigned int nal_type = rtpdata[1] & 0x1F;
    el->nal_type = nal_type;
    el->state = start_bit == 128 ? start_bit : end_bit;
    
    if (start_bit == 128){
        if (nal_type == 5){
            printf("start new gop [%d]\n", seq_num);
            if (info->start_seq == -1) //primo gop in assoluto, quindi non spedire direttamente il gop al thread ReaderPacket.
                info->start_seq = seq_num;
            else
                //info->num_frame = 0;
                //info->gop_num = num_gop;
                //info->start_seq = seq_num;
                //ora posso aggiornare i dati
                //start_gop = seq_num;
                //num_gop ++;
                return_value = seq_num;
        }
        /* viene fatto dopo ogni marker */
        unsigned int idr_nal = rtpdata[0] & 0xE0;  // 3 NAL UNIT BITS
        unsigned int nal = idr_nal | nal_type;  // [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
        el->decoder = nal;
    }
    //else if (end_bit == 64)
    //    if (nal_type == 5) { // potrebbe essere necessario controllare che end_gop sia minore del nuovo seq num
            //end_new_gop = seq_num;
    //        info->end_seq = seq_num;
    //    }
        info->num_frame++;  // solo per dati statistici
    //if (nal_type == 5) // mi assicuro di non aver già fatto la richiesta per il gop precedente al thread unificatore, potrei dover fare un else per aggiornare end gop nel caso non si sia ricevuto end_gop
    //    if (seq_num > start_gop && seq_num < end_new_gop) { //mi assicuro che sia nel range del gop da accettare, end_gop potrebbe non essere ancora aggiornato. verificare domani
           /*
            manda un pacchetto al thread per riassemblare i pacchetti del gop precedente
            */
            //printf("Hei");
        //}
    //if (current_seq < seq_num) //aggiorna current seq
    //    current_seq = seq_num;
    if(!insert_hash(seq_num, el)) //elemento già inserito ritorna
        return -1;
    if (return_value == 0)
        info->end_seq = info->end_seq > seq_num ? info->end_seq : seq_num;
    return return_value;
}

void* insert_hash(uint16_t primarykey, void* insert){
    uint16_t* key = setkeyHash(primarykey);
    int buck = hash_packet->hash_function(key) % hash_packet->nbuckets;
    int mtxi = buck / DIV;
    pthread_mutex_lock(&mtxhash[mtxi]);
    void* ris = icl_hash_insert(hash_packet, key, insert);
    pthread_mutex_unlock(&mtxhash[mtxi]);
    return ris;
}

void* find_hash(uint16_t* primarykey){
    int buck = hash_packet->hash_function(primarykey) % hash_packet->nbuckets;
    int mtxi = buck / DIV;
    pthread_mutex_lock(&mtxhash[mtxi]);
    void* ris = icl_hash_find(hash_packet, primarykey);
    pthread_mutex_unlock(&mtxhash[mtxi]);
    return ris;
}

int delete_hash(uint16_t* primarykey){
    int buck = hash_packet->hash_function(primarykey) % hash_packet->nbuckets;
    int mtxi = buck / DIV;
    pthread_mutex_lock(&mtxhash[mtxi]);
    int ris = icl_hash_delete(hash_packet, primarykey, &freeKeyHash, &freeElHash);
    pthread_mutex_unlock(&mtxhash[mtxi]);
    return ris;
}

void send_packet(rtp* el){
    struct udphdr* udp = NULL;
    if (!from_loopback) {
        udp = (struct udphdr*) (el->packet + to_udphdr);
        udp->uh_dport = htons(DPORT);
        udp->uh_sum = 0;
        if (datalink_loopback == DLT_NULL) {
            int new_len = to_iphdr - 4;
            u_char* buff = el->packet + new_len;
            loopbackstarter(buff);
            if (pcap_sendpacket(loopback, buff, el->size - new_len) != 0)
                fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(loopback));
        }
        else{ //se devo spedire un altro pacchetto ethernet devo azzerarlo
            struct ether_header* eth = (struct ether_header *) el->packet;
            for (int i=0; i < MAC_LENGHT; i++) {
                eth->ether_dhost[i] = MAC_ADDR;
                eth->ether_shost[i] = MAC_ADDR;
            }
            if (pcap_sendpacket(loopback, el->packet, el->size) != 0)
                fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(loopback));
        }
    }
    else{
        u_char* send = NULL;
        int fix = 0, new_size = 0;
        if (datalink_loopback == DLT_NULL)
            fix = sizeof(struct ether_header) - 4;
        udp = (struct udphdr*) (el->packet + to_udphdr - fix);
        udp->uh_dport = htons(DPORT);
        udp->uh_sum = 0;
        send = (el->packet + to_rtphdr - fix);
        new_size = el->size - to_rtphdr + fix;
        if (sendto(fd, send, new_size, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            perror("cannot send message");
        }
        //if (pcap_sendpacket(loopback, el->packet, el->size) != 0) //si potrebbe usare se non fosse per linux
        //    fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(loopback));
    }
    el->sent = 1;
}

int workOnPacket(rtp* el, gop_info* info){
    int ris = 0;
    int fix = 0;
    if (from_loopback && datalink_loopback == DLT_NULL) {
        fix = (int) sizeof(struct ether_header) - 4;
    }
    int rtpdata_len = el->size - to_rtpdata + fix;
    rtphdr* rtpHeader = (rtphdr*)(el->packet + to_rtphdr - fix);
    uint16_t seq = ntohs(rtpHeader->seq_num);
    //printf("RTP number [%d], timestamp of this packet is: %d\n", ntohs(rtpHeader->seq_num), ntohl(rtpHeader->TS)); //function converts the unsigned short integer netshort from network byte order to host byte order.
    u_char* rtpdata = (u_char*) (el->packet + to_rtpdata - fix);
    //FU - A - -HEADER
    //unsigned int forbidden = rtpdata[0] & 0x80 >> 7;
    //unsigned int nri = rtpdata[0] & 0x60 >> 5;
    unsigned int fragment_type = rtpdata[0] & 0x1F;  // il valore è 28
    
    if(!inizialized)
        create_header_information(rtpdata, fragment_type, rtpdata_len);
    //SPS e PPS ricevuti inizio a creare il GOP
    else if (fragment_type == 28){ //dati per il GOP
        if ((ris = addPacketToGOP(rtpdata, rtpdata_len, seq, el, info)) != -1) {
            send_packet(el);
        }
        return ris;
    } else send_packet(el);
    return -1;
}

rtp* save_packet(FILE* f, rtp* el, uint16_t* from, int fix, int end_seq){
    int rtpdata_len = el->size - to_rtpdata + fix;
    u_char* rtpdata =(u_char*) (el->packet + to_rtpdata - fix);
    if (el->state == 128) {
        starter(&payload);
        memcpy(payload.value+payload.size, &(el->decoder), 1); // header per il frame
        payload.size += 1;
    }
    add(&payload, rtpdata+2, rtpdata_len-2);
    while (!el->sent); //aspetto che il pacchetto è stato spedito
    delete_hash(from);
    el = NULL;
    while (end_seq > *from && !el) {
        (*from)++;
        el = find_hash(from);
    }
    return el;
}

void save_GOP(uint16_t *from, gop_info* info){
    FILE* f = NULL;
    char GOPName[64];
    int fix = 0;
    if (from_loopback && datalink_loopback == DLT_NULL)
        fix = (int) sizeof(struct ether_header) - 4;
    sprintf(GOPName, "%s-%06d", path_file, info->gop_num);
    if (!(f = fopen(GOPName, "w")))
        printf("Error: %s\n", strerror(errno));
    rtp* el = find_hash(from);
    add(&payload, metadata.value, metadata.size);
    while(el && el->nal_type == 5){ // copia tutto il pacchetto header
        el = save_packet(f,el, from, fix, info->end_seq);
    }
    while (el && el->nal_type != 5) { //copia finchè non trova un nuovo GOP
        el = save_packet(f,el, from, fix, info->end_seq);
    }
    fwrite(payload.value, 1, payload.size, f);
    payload.size = 0;
    fclose(f);
}
