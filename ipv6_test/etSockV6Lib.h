/************************************************************************
Copyright (C), 2022-2023,
File name:      etSockV6Lib.h
Author:           
Date:           
Description:    对不同操作系统的socket IPv6接口进行封装，对上层提供统一socket接口
Others:         无
  *************************************************************************/
#ifndef __ETSOCKV6LIB_H__
#define __ETSOCKV6LIB_H__
#pragma once

/************************************************************************
                             引用的头文件
*************************************************************************/
#include "sys.h"
#include <errno.h>

#ifdef WINDOWS
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif // WINDOWS

#ifdef LINUX
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <linux/route.h>
//<net/route.h> & <linux/route.h> redefinition
//<linux/in6.h> & <netinet/in.h> redefinition
#endif

#ifdef AIX
#include "../../os/ospub.h" 
#endif

#ifdef VXWORKS
#include <in.h>
#include <in6.h>
#endif

/************************************************************************
                             宏/类型定义
*************************************************************************/
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef  SERVERV6_MAX_CONNECTIONS
#define  SERVERV6_MAX_CONNECTIONS    10    //listen允许的最大连接数
#endif

#define  IPPROTO_IPV4_HDRVAL         0x04
#define  IPPROTO_IPV6_HDRVAL         0x06

#ifndef IPV6_FLOWINFO_FLOWLABEL
#define IPV6_FLOWINFO_FLOWLABEL		 0x000fffff
#endif
#ifndef IPV6_FLOWINFO_PRIORITY
#define IPV6_FLOWINFO_PRIORITY		 0x0ff00000
#endif
#define IPV6_FLOWINFO_VERSION 		 0xf0000000

#if defined(LINUX) && !defined(IPV6_UNICAST_IF)
#define IPV6_UNICAST_IF         76
#endif

#ifndef s6_addr
typedef struct in6_addr {
    union {
        UCHAR       Byte[16];
        USHORT      Word[8];
    } u;
} IN6_ADDR;
#endif

typedef struct inv6_pktinfo {  //in6_pktinfo
    struct in6_addr ipi6_addr;    // Source/destination IPv6 address.
    EULONG ipi6_ifindex;    // Send/receive interface index.
} INV6_PKTINFO, * PINV6_PKTINFO;

typedef struct ipv6_mtuinfo {  //ip6_mtuinfo
    struct sockaddr_in6 ip6m_addr;
    EULONG ip6m_mtu;
} IPV6_MTUINFO, * PIPV6_MTUINFO;

#if defined(WINDOWS)
struct ip6_hdr
{
    union
    {
        struct ip6_hdrctl
        {
            EUINT32 ip6_un1_flow;   /* 4 bits version, 8 bits TC, 20 bits flow-ID */
            EUINT16 ip6_un1_plen;   /* payload length */
            EUINT8  ip6_un1_nxt;    /* next header */
            EUINT8  ip6_un1_hlim;   /* hop limit */
        } ip6_un1;
        EUINT8 ip6_un2_vfc;       /* 4 bits version, top 4 bits tclass */
    } ip6_ctlun;
    struct in6_addr ip6_src;      /* source address */
    struct in6_addr ip6_dst;      /* destination address */
};
#define ip6_vfc   ip6_ctlun.ip6_un2_vfc
#define ip6_flow  ip6_ctlun.ip6_un1.ip6_un1_flow
#define ip6_plen  ip6_ctlun.ip6_un1.ip6_un1_plen
#define ip6_nxt   ip6_ctlun.ip6_un1.ip6_un1_nxt
#define ip6_hlim  ip6_ctlun.ip6_un1.ip6_un1_hlim
#define ip6_hops  ip6_ctlun.ip6_un1.ip6_un1_hlim

struct ip
{
#if defined(ET_LITTLE_ENDIAN)
    unsigned int ip_hl : 4;               /* header length */
    unsigned int ip_v : 4;                /* version */
#elif defined(ET_BIG_ENDIAN)
    unsigned int ip_v : 4;                /* version */
    unsigned int ip_hl : 4;               /* header length */
#endif
    EUINT8 ip_tos;                    /* type of service */
    EUSHORT ip_len;                     /* total length */
    EUSHORT ip_id;                      /* identification */
    EUSHORT ip_off;                     /* fragment offset field */
#define IP_RF 0x8000                    /* reserved fragment flag */
#define IP_DF 0x4000                    /* dont fragment flag */
#define IP_MF 0x2000                    /* more fragments flag */
#define IP_OFFMASK 0x1fff               /* mask for fragmenting bits */
    EUINT8 ip_ttl;                    /* time to live */
    EUINT8 ip_p;                      /* protocol */
    EUSHORT ip_sum;                     /* checksum */
    struct in_addr ip_src, ip_dst;      /* source and dest address */
};
#define IP_MAXPACKET    65535           /* maximum packet size */
#define MAXTTL          255             /* maximum time to live (seconds) */
#define IPDEFTTL        64              /* default ttl, from RFC 1340 */
#define IPFRAGTTL       60              /* time to live for frags, slowhz */
#define IPTTLDEC        1               /* subtracted when forwarding */
#define IP_MSS          576             /* default maximum segment size */

#ifdef __FAVOR_BSD
struct udphdr
{
    EUINT16 uh_sport;           /* source port */
    EUINT16 uh_dport;           /* destination port */
    EUINT16 uh_ulen;            /* udp length */
    EUINT16 uh_sum;             /* udp checksum */
};
#else
struct udphdr
{
    EUINT16 source;
    EUINT16 dest;
    EUINT16 len;
    EUINT16 check;
};
#endif

/* Routing header options for IPv6.  */
#define IPV6_RTHDR_LOOSE	0	/* Hop doesn't need to be neighbour. */
#define IPV6_RTHDR_STRICT	1	/* Hop must be a neighbour.  */

#define IPV6_RTHDR_TYPE_0	0	/* IPv6 Routing header type 0.  */

#endif //WINDOWS

#ifdef LINUX
#ifndef _NET_ROUTE_H
struct in6_rtmsg
{
    struct in6_addr rtmsg_dst;
    struct in6_addr rtmsg_src;
    struct in6_addr rtmsg_gateway;
    EUINT32 rtmsg_type;
    EUINT16 rtmsg_dst_len;
    EUINT16 rtmsg_src_len;
    EUINT32 rtmsg_metric;
    unsigned long int rtmsg_info;
    EUINT32 rtmsg_flags;
    int rtmsg_ifindex;
};
#endif
#endif //LINUX

/************************************************************************
                               函数声明
*************************************************************************/

#endif //__ETSOCKLIB_H__
