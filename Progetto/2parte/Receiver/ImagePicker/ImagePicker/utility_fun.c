//
//  utility.c
//  ImagePicker
//
//  Created by Massimo Puddu on 23/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "utility_fun.h"

//inizializzazione socket, ritorna socket fd
int set_stat_sock(){
    int sockfd, tentativi=0;
    struct sockaddr_in servaddr;
    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_sender);
    servaddr.sin_port = htons(stat_port);
    // connect the client socket to server socket
    while (tentativi < 5 && !esci) {
        if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            sleep(2);
            printf("Altri %d tentativi, prima della chiusura\n", 4 - tentativi);
            tentativi++;
        }
        else
            break;
    }
    if (tentativi == 5 && !esci){
        printf("Impossibile connettersi al server, chiudo\n");
        fprintf(pipe_plot, "Esci\n");
        fclose(pipe_plot);
        exit(-1);
    } else
        printf("connected to the server..\n");
    return sockfd;
}

//funzione di comparazione per il qsort
int cmpfunc (const void * a, const void * b){
    return ( (*(pkt_info*)a).ids - (*(pkt_info*)b).ids );
}

uint16_t stat_lunghezza(pkt_info arr[], int index){
    int num_lost, num_buchi = 0, tot = 0, j;
    for (j = 1; j < index; j++) {
        num_lost = arr[j].ids - arr[j-1].ids - 1;
        if (num_lost > 0)
            num_buchi++;
        tot += num_lost;
    }
    if (num_buchi > 0)
        return tot/num_buchi;
    return 0;
}

uint16_t stat_out_of_order(pkt_info ord[], uint16_t not_ord[], int dim){
    int i = 0, j = 0;
    uint16_t out = 0, prec, curr; //prec contiene l'ids corrente
    for (i = 0; i < dim; i++) {
        if (ord[i].ids != not_ord[i]) {
            out++;
            prec = not_ord[i];
            j = i + 1;
            while (not_ord[j] != ord[i].ids) { // shift a sinistra finchè non si trova l'elemento corrispondente
                curr = not_ord[j];
                not_ord[j] = prec;
                j++;
                prec = curr;
            }
            not_ord[i] = ord[i].ids;
            not_ord[j] = prec;
        }
    }
    return out;
}

float jitter_calculator(pkt_info el[], int dim){
    pkt_info infj, infi; // variabili di lavoro per rendere più leggibile il calcolo del jitter
    float d_value = 0, jitter = 0;
    for(int j = 1; j < dim; j++){
        infj = el[j];
        infi = el[j-1];
        d_value = fabs((infj.r_timestamp-infj.pkt_timestamp)-(infi.r_timestamp-infi.pkt_timestamp));
        jitter = jitter + (d_value - jitter)/JITTER_DIV;
    }
    return jitter;
}

void send_to_server(int sockfd, send_stat spedisci[]){
    writen(sockfd, spedisci, sizeof(send_stat)*num_list);
}

rtp* delete_and_get_next(rtp* el, uint16_t* from, int end_seq){
    while (!el->sent); //aspetto che il pacchetto è stato spedito
    delete_hash(from);
    el = NULL;
    while (end_seq > *from && !el) {
        (*from)++;
        el = find_hash(from);
    }
    return el;
}

void fill_losted_packet(uint16_t* prec, uint16_t* from, rtp* el, gop_info* info){
    while ((*prec)+1 != *from) {
        info->losted_packet[info->index_losted] = *prec;
        info->index_losted++;
        info->len_losted[info->index_len]++;
        (*prec)++;
    }
    *prec = *from;
    //Attenzione, assumo che  mi arrivi almeno una fine o un inizio di ogni frame
    if(el){
    if (el->state == 128) { // state vale 128 se è un nuovo frame 64 se il frame finisce, 0 altrimenti
        if(info->incremented) // è già stato incrementato precedentemente
            info->incremented = 0;
        else
            info->index_len++;
    }
    else if (el->state == 64){
        info->incremented = 1;
        info->index_len++;
    }
    else
        info->incremented = 0;
    }
}

int writen(int fd, void *buf, size_t size) {
    size_t left = size;
    int r=0;
    char *bufptr = (char*)buf;
    while(left>0) {
        r=(int)write(fd ,bufptr,left);
        if (r == -1) {
            if (errno == EINTR) continue;
            if (errno == EPIPE) pthread_kill(segnal, SIGINT);
            return -1;
        }
        if (r == 0) return 0;
        left    -= r;
        bufptr  += r;
    }
    return 1;
}

// algoritmo preso da https://it.mathworks.com/matlabcentral/fileexchange/37691-psnr-for-rgb-images
//src e dst sono le immagini da confrontare, w e h sono dimensioni delle due immagini che devono coincidere(non viene controllato)
/*float calculate_PSNR(u_char* src, u_char* dst, int w, int h){
    return (float)(10.0*log10( pow(255,2) / calculate_MSE(src, dst, w, h))); //255 è per le immagini 8 bit
}

float calculate_MSE(u_char* src, u_char* dst, int w, int h){
    float mseR = sum(sum(mseRImage)) / (w * h);
    float mseG = sum(sum(mseGImage)) / (w * h);
    float mseB = sum(sum(mseBImage)) / (w * h);
    
    return (mseR + mseG + mseB)/3;
}*/

/* funzioni di utilità per interagire con la tabella hash */
void* insert_hash(uint16_t primarykey, void* insert){
    uint16_t* key = setkeyHash(primarykey);
    int buck = hash_packet->hash_function(key) % hash_packet->nbuckets;
    int mtxi = buck / DIV;
    pthread_mutex_lock(&mtxhash[mtxi]);
    void* ris = icl_hash_insert(hash_packet, key, insert);
    pthread_mutex_unlock(&mtxhash[mtxi]);
    if (!ris)
        freeKeyHash(key);
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
