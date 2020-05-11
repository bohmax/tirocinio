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

void send_packet(rtp* el){
    struct udphdr* udp = NULL;
    if (!from_loopback) {
        udp = (struct udphdr*) (el->packet + to_udphdr);
        udp->uh_dport = htons(stat_port);
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
        udp->uh_dport = htons(stat_port);
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

void stat_calc(rtp* el, int index, uint16_t rtp_id, uint32_t timestamp){
    stat_t* stat = &statistiche[index];
    if (delay_calibrator[index] == -1)
        delay_calibrator[index] = labs(timestamp - el->r_timestamp); // calibra il delay dei pacchetti, calcolato solo per il primo pacchetto
    if (rtp_id > stat->id_accepted) {
        stat->ids[stat->index]=rtp_id;
        stat->index++;
        long diff = labs(timestamp - el->r_timestamp);
        stat->delay += diff>delay_calibrator[index]? diff - delay_calibrator[index]: delay_calibrator[index]-diff; // memorizzo il delay, e poi lo dividerò quando vado a calcolare il delay
        if (stat->max < rtp_id)
            stat->max = rtp_id;
        if (stat->min > rtp_id)
            stat->min = rtp_id;
    }
}

/*void create_header_information(u_char* rtpdata, unsigned int fragment_type, int rtpdata_len, uint16_t seq_num){
 if(fragment_type == 7){
 starter(&metadata);
 add(&metadata, rtpdata, rtpdata_len);  // SPS information
 }
 else if (fragment_type == 8){
 starter(&metadata);
 add(&metadata, rtpdata, rtpdata_len);  // PPS information
 pthread_mutex_lock(&mtx_info);
 info->end_last_gop = seq_num; //setto il primo gop assoluto
 info->start_seq = seq_num+1;
 info->accepted28 = seq_num+1;
 printf("start new gop [%d]\n", seq_num+1);
 pthread_mutex_unlock(&mtx_info);
 inizialized = 1;
 }
 }*/

int addPacketToGOP(u_char* rtpdata, int rtpdata_len, uint16_t seq_num, rtp* el){
    int return_value = 0;
    //Nal header
    unsigned int start_bit = rtpdata[1] & 0x80;  // 128 se e' il primo pacchetto del frame 0 altrimenti
    unsigned int end_bit = rtpdata[1] & 0x40;  // 64 se e' l ultimo pacchetto 0 altrimenti
    unsigned int reserved = rtpdata[1] & 0x20 >> 5;
    unsigned int nal_type = rtpdata[1] & 0x1F;
    el->nal_type = nal_type;
    el->state = start_bit == 128 ? start_bit : end_bit;
    if (nal_type == 5){ //nuovo gop, devo essere sicuro che si riferisce al gop corrente
        pthread_mutex_lock(&mtx_info);
        if (info) {
            if (info->gop_over == -1) { //se questa condizione è falsa qualche altro thread ha già inserito tutte le informazioni utili per questo gop
                if (seq_num < info->start_seq && seq_num > info->end_last_gop) {
                    info->start_seq = seq_num;
                }
                else if (info->after_5 && seq_num > info->start28){
                    info->gop_over = info->end_seq;
                    info->end_seq = seq_num;
                    return_value = seq_num;
                }
            }
            else { //ultimi aggiornamenti dati prima di spedire al decodificatore
                if (info->start_seq > seq_num && info->end_last_gop < seq_num) {
                    info->start_seq = seq_num;
                }
                else if (seq_num > info->gop_over && seq_num < info->end_seq){
                    info->end_seq = seq_num;
                }
            }
        }
        pthread_mutex_unlock(&mtx_info);
    }
    else{ //nal type == 1
        pthread_mutex_lock(&mtx_info);
        if (info) {
            if (!info->after_5 && seq_num > info->accepted28) {
                info->after_5 = 1;
                info->start28 = seq_num;
                info->end_seq = seq_num;
            }
            if(info->gop_over == -1 && info->end_seq < seq_num) // se gop over != 0 è inutile aggiornare end seq, tanto il decoder controlla gop over per fermarsi
                info->end_seq = seq_num;
            pthread_mutex_unlock(&mtx_info);
        }
    }
    if (start_bit == 128){
        /* viene fatto dopo ogni marker */
        unsigned int idr_nal = rtpdata[0] & 0xE0;  // 3 NAL UNIT BITS
        unsigned int nal = idr_nal | nal_type;  // [ 3 NAL UNIT BITS | 5 FRAGMENT TYPE BITS] 8 bits
        el->decoder = nal;
    }
    //}
    //info->num_frame++;  // solo per dati statistici
    //if (return_value == 0){
    //    pthread_mutex_lock(&mtx_info);
    //    info->end_seq = info->end_seq > seq_num ? info->end_seq : seq_num;
    //    pthread_mutex_unlock(&mtx_info);
    //}
    return return_value;
}

int workOnPacket(rtp* el, int stat_index){
    int ris = 0;
    int fix = 0;
    if (from_loopback && datalink_loopback == DLT_NULL) {
        fix = (int) sizeof(struct ether_header) - 4;
    }
    int rtpdata_len = el->size - to_rtpdata + fix;
    rtphdr* rtpHeader = (rtphdr*)(el->packet + to_rtphdr - fix);
    uint16_t seq = ntohs(rtpHeader->seq_num);
    //modifica statistiche
    pthread_mutex_lock(&mtxstat[el->from_thr]);
    stat_calc(el, el->from_thr, seq, ntohl(rtpHeader->TS));
    pthread_mutex_unlock(&mtxstat[el->from_thr]);
    //printf("RTP number [%d], timestamp of this packet is: %d\n", ntohs(rtpHeader->seq_num), ntohl(rtpHeader->TS)); //function converts the unsigned short integer netshort from network byte order to host byte order.
    u_char* rtpdata = (u_char*) (el->packet + to_rtpdata - fix);
    //FU - A - -HEADER
    //unsigned int forbidden = rtpdata[0] & 0x80 >> 7;
    //unsigned int nri = rtpdata[0] & 0x60 >> 5;
    unsigned int fragment_type = rtpdata[0] & 0x1F;  // il valore è 28
    
    pthread_mutex_lock(&mtx_info);
    if (info && seq <= info->end_last_gop)
        ris = -1;
    pthread_mutex_unlock(&mtx_info);
    
    if (!ris && insert_hash(seq, el)){ //elemento già inserito ritorna
        if (fragment_type == 28) //dati per il GOP
            ris = addPacketToGOP(rtpdata, rtpdata_len, seq, el);
        else{
            el->nal_type = fragment_type; //non è esattamente ilnal type ma posso inserire il valore qui
            pthread_mutex_lock(&mtx_info);
            if (info->metadata_start > seq && seq < info->start_seq)
                info->metadata_start = seq;
            else if (seq > info->start_seq && seq < info->next_metadata)
                info->next_metadata = seq;
            pthread_mutex_unlock(&mtx_info);
        }
        send_packet(el);
    } else ris = -1;
    return ris;
}

rtp* set_header(rtp* el, uint16_t* from, int fix, int end_seq){
    int rtpdata_len = el->size - to_rtpdata + fix;
    u_char* rtpdata = (u_char*) (el->packet + to_rtpdata - fix);
    
    starter(&payload);
    add(&payload, rtpdata, rtpdata_len);
    
    while (!el->sent); //aspetto che il pacchetto è stato spedito
    return delete_and_get_next(el, from, end_seq);
}

rtp* save_packet(FILE* f, rtp* el, uint16_t* from, int fix, int end_seq){
    int rtpdata_len = el->size - to_rtpdata + fix;
    u_char* rtpdata = (u_char*) (el->packet + to_rtpdata - fix);
    if (el->state == 128) {
        starter(&payload);
        memcpy(payload.value+payload.size, &(el->decoder), 1); // header per il frame
        payload.size += 1;
    }
    add(&payload, rtpdata+2, rtpdata_len-2);
    fwrite(payload.value, 1, payload.size, f);
    payload.size = 0;
    return delete_and_get_next(el, from, end_seq);
}

void save_GOP(uint16_t *from, gop_info* info){
    FILE* f = NULL;
    char GOPName[64];
    int num_hdr = 0;
    int fix = 0;
    if (from_loopback && datalink_loopback == DLT_NULL)
        fix = (int) sizeof(struct ether_header) - 4;
    sprintf(GOPName, "%s-%06d", path_file, info->gop_num);
    if (!(f = fopen(GOPName, "w")))
        printf("Error: %s\n", strerror(errno));
    rtp* el = find_hash(from);
    //add(&payload, metadata.value, metadata.size); //se un giorno si decidesse di usare questa soluzione, solo un fwrite alla fine
    while (el && el->nal_type != 5){ // cicla finchè i pacchetti di header non finiscono
        el = set_header(el, from, fix, info->start_seq);
        num_hdr++;
    }
    if (num_hdr==2) { // sps e pps presenti
        add(&metadata, payload.value, payload.size);
        payload.size = 0;
    }
    fwrite(metadata.value, 1, metadata.size, f);
    *from = info->start_seq;
    while(el && el->nal_type == 5){ // copia tutto il pacchetto header
        el = save_packet(f, el, from, fix, info->end_seq);
    }
    while (el && el->nal_type < 5) { //copia finchè non trova un nuovo GOP
        el = save_packet(f, el, from, fix, info->end_seq);
    }
    //fwrite(payload.value, 1, payload.size, f);
    //payload.size = 0;
    fclose(f);
}
