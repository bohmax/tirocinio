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
#include <pcap.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "rtp.h"
#include "string.h"
#include "receiver.h"
#define ERRORSYSHEANDLER(r,c,d,e) if((r=c)==d) { perror(e);exit(errno); }
#define SYSFREE(r,c,d,e) if((r=c)!=d) { perror(e);exit(errno); }
#define GOPM 400000 //dimensioni di un GOP medio moltiplicato per 2

extern string metadata; //dovrà contenere SPS e PPS
extern string payload; //dovrà contenere un intero GOP
extern pcap_t* handle;    /* packet capture handle */

#endif /* utility_h */
