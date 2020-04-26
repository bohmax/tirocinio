//
//  utility.c
//  ImagePicker
//
//  Created by Massimo Puddu on 23/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "utility.h"

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
    servaddr.sin_addr.s_addr = inet_addr(HOSTNAME);
    servaddr.sin_port = htons(DPORT);
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
        exit(-1);
    } else
        printf("connected to the server..\n");
    return sockfd;
}

//funzione di comparazione per il qsort
int cmpfunc (const void * a, const void * b){
   return ( *(uint16_t*)a - *(uint16_t*)b );
}

uint16_t stat_lunghezza(uint16_t arr[], int index){
    int num_lost, num_buchi = 0, tot = 0, j;
    qsort(arr, index, sizeof(uint16_t), cmpfunc);
    for (j = 1; j < index; j++) {
        num_lost = arr[j] - arr[j-1] - 1;
        if (num_lost > 0)
            num_buchi++;
        tot += num_lost;
    }
    if (num_buchi > 0)
        return tot/num_buchi;
    return 0;
}

uint16_t stat_out_of_order(uint16_t arr[], uint16_t current, int index){
    int j = 0;
    uint16_t out = 0;
    uint16_t max_actual = current;
    for (j = 0; j < index; j++) {
        if(current == arr[j]){
            out++;
            current = max_actual;
        } else if (max_actual < arr[j])
            max_actual = arr[j];
    }
    return out;
}

void send_to_server(int sockfd, send_stat spedisci[]){
    writen(sockfd, spedisci, sizeof(send_stat)*NUMLISTTHR);
}

// algoritmo preso da https://it.mathworks.com/matlabcentral/fileexchange/37691-psnr-for-rgb-images
//src e dst sono le immagini da confrontare, w e h sono dimensioni delle due immagini che devono coincidere(non viene controllato)
float calculate_PSNR(u_char* src, u_char* dst, int w, int h){
    return (float)(10.0*log10( pow(255,2) / calculate_MSE(src, dst, w, h))); //255 è per le immagini 8 bit
}

/*float calculate_MSE(u_char* src, u_char* dst, int w, int h){
    float mseR = sum(sum(mseRImage)) / (rows * columns);
    float mseG = sum(sum(mseGImage)) / (rows * columns);
    float mseB = sum(sum(mseBImage)) / (rows * columns);
    
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
