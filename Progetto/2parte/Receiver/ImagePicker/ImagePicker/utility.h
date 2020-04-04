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
#include "define.h"
#include "rtp.h"
#include "string.h"
#include "receiver.h"
#include "h264toPng.h"

extern pcap_t* handle;    /* packet capture handle */
extern char* path_file;   /* path del file su cui viene salvato lo streaming*/

#endif /* utility_h */
