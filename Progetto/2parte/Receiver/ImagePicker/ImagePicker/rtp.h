//
//  rtp.h
//  ImagePicker
//
//  Created by Massimo Puddu on 30/03/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef _netinet_rtp_h
#define _netinet_rtp_h

/* RTP Upper Layer Format Numbers H.225 */
//struct presa da https://fossies.org/linux/argus/include/netbsd-include/netinet/rtp.h

#define IPPROTO_RTP 257

#define RTP_PCMU    0
#define RTP_PCMA    8
#define RTP_G722    9
#define RTP_G723    4
#define RTP_G728    15
#define RTP_G729    18
#define RTP_H261    31
#define RTP_H263    34

/* RTP Header as defined in H.225 */

typedef struct _RTPHeader
{
  //first byte
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  unsigned int         CC:4;        /* CC field */
  unsigned int         X:1;         /* X field */
  unsigned int         P:1;         /* padding flag */
  unsigned int         version:2;
#elif G_BYTE_ORDER == G_BIG_ENDIAN
  unsigned int         version:2;
  unsigned int         P:1;         /* padding flag */
  unsigned int         X:1;         /* X field */
  unsigned int         CC:4;        /* CC field*/
#else
#error "G_BYTE_ORDER should be big or little endian."
#endif
  //second byte
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  unsigned int         PT:7;     /* PT field */
  unsigned int         M:1;       /* M field */
#elif G_BYTE_ORDER == G_BIG_ENDIAN
  unsigned int         M:1;         /* M field */
  unsigned int         PT:7;       /* PT field */
#else
#error "G_BYTE_ORDER should be big or little endian."
#endif
    unsigned int seq_num:16;      /* length of the recovery */
    unsigned int TS;                   /* Timestamp */
    unsigned int ssrc;
} rtphdr; //12 bytes

#endif /* rtp_h */
