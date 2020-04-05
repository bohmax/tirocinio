//
//  utility.h
//  ImagePicker
//
//  Created by Massimo Puddu on 31/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef utility_h
#define utility_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pcap.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avio.h>
#include "define.h"
#include "rtp.h"
#include "struct.h"
#include "receiver.h"
#include "h264toPng.h"
#include "lista.h"

extern pcap_t* handle;    /* packet capture handle */
extern string payload;    /* contiene la codifica di un h264 */
extern gop gop_info;      /* contiene le informazioni di un gop */
extern char* path_file;   /* path del file su cui viene salvato lo streaming*/
extern char* path_image;   /* path su cui saranno salvate le immagini*/
extern int esci;            /* indica l'uscita dal programma*/
extern pthread_mutex_t mtx;  /* mutex per fare produttore consumatore con il decodificatore */
extern pthread_cond_t cond;  /* variabile di condizione per produttore consumatore*/
extern list* testa;
extern list* coda;  /* coda della lista sopra */

#endif /* utility_h */
