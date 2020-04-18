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

/* IP header  https://www.tcpdump.org/pcap.html */
typedef struct sniff_ip {
    u_char ip_vhl;        /* version << 4 | header length >> 2 */
    u_char ip_tos;        /* type of service */
    u_short ip_len;        /* total length */
    u_short ip_id;        /* identification */
    u_short ip_off;        /* fragment offset field */
#define IP_RF 0x8000        /* reserved fragment flag */
#define IP_DF 0x4000        /* dont fragment flag */
#define IP_MF 0x2000        /* more fragments flag */
#define IP_OFFMASK 0x1fff    /* mask for fragmenting bits */
    u_char ip_ttl;        /* time to live */
    u_char ip_p;        /* protocol */
    u_short ip_sum;        /* checksum */
    struct in_addr ip_src,ip_dst; /* source and dest address */
}sniff_ip_t;
#define IP_HL(ip)        (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)        (((ip)->ip_vhl) >> 4)

/* RTP Header as defined in H.225 */

typedef struct _RTPHeader{
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
