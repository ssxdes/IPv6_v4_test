/************************************************************************
Copyright (C), 2009-2010, 
File name:      socketLib.h
Author:       
Date:         
Description:    �Բ�ͬ����ϵͳ��socket�ӿڽ��з�װ�����ϲ��ṩͳһsocket�ӿ�
Others:         ��
  *************************************************************************/
#ifndef __BEARPUB_H__
#define __BEARPUB_H__

#ifdef __cplusplus
extern "C" {
#endif
/************************************************************************
                            ���õ�ͷ�ļ�
*************************************************************************/
#include "sys.h"

/************************************************************************
                            �궨��
*************************************************************************/
#ifndef SOCK_STREAM
#define    ESOCK_STREAM    1        //TCP
#define    ESOCK_DGRAM     2        //UDP
#define    ESOCK_RAW       3
#else
#define    ESOCK_STREAM    SOCK_STREAM        //TCP
#define    ESOCK_DGRAM     SOCK_DGRAM        //UDP
#define    ESOCK_RAW       SOCK_RAW
#endif

#if	defined ET_BIG_ENDIAN
#define	etHtonl(x)    (x)
#define	etNtohl(x)    (x)
#define	etHtons(x)    (x)
#define	etNtohs(x)    (x)
#elif defined ET_LITTLE_ENDIAN
#define etHtonl(x)    ((((x) & 0x000000ff) << 24) | \
             (((x) & 0x0000ff00) <<  8) | \
             (((x) & 0x00ff0000) >>  8) | \
             (((x) & 0xff000000) >> 24))
#define etNtohl(x)    ((((x) & 0x000000ff) << 24) | \
             (((x) & 0x0000ff00) <<  8) | \
             (((x) & 0x00ff0000) >>  8) | \
             (((x) & 0xff000000) >> 24))
#define etHtons(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))
#define etNtohs(x)    ((((x) & 0x00ff) << 8) | \
             (((x) & 0xff00) >> 8))
#endif //ET_LITTLE_ENDIAN


//the tag of IpQos
#define QOS_VAL(x,y) ((x<<3)+(y<<1))
enum
{
	QOS_BEST,
	QOS_AF11= QOS_VAL(1,1),
	QOS_AF12= QOS_VAL(1,2),
	QOS_AF13= QOS_VAL(1,3),
	QOS_AF21= QOS_VAL(2,1),
	QOS_AF22= QOS_VAL(2,2),
	QOS_AF23= QOS_VAL(2,3),
	QOS_AF31= QOS_VAL(3,1),
	QOS_AF32= QOS_VAL(3,2),
	QOS_AF33= QOS_VAL(3,3),
	QOS_AF41= QOS_VAL(4,1),
	QOS_AF42= QOS_VAL(4,2),
	QOS_AF43= QOS_VAL(4,3),
	QOS_EF= QOS_VAL(5,3),
	QOS_INC= QOS_VAL(6,0),
	QOS_NC= QOS_VAL(7,0),
    QOS_PRIORITY_1 = QOS_VAL(1,0),
    QOS_PRIORITY_2 = QOS_VAL(2,0),
    QOS_PRIORITY_3 = QOS_VAL(3,0),
	QOS_PRIORITY_4 = QOS_VAL(4,0)
};

/************************************************************************
                             ��������
*************************************************************************/
/*---------------------------socket support---------------------------------*/
#define  IPv4                      AF_INET
#define  IPv6                      AF_INET6
#define  IPv4_Addr_Any             INADDR_ANY
#define  IPv4_Addr_LoopBack        INADDR_LOOPBACK
#define  IPv4_Addr_BroadCast       INADDR_BROADCAST
//       IPv4  ::  ���� / �鲥 / �㲥
#define  IPv6_Addr_Any             in6addr_any         //��ַ::/128
#define  IPv6_Addr_LoopBack        in6addr_loopback    //��ַ::1/128
//       IPv6  ::  ���� / �鲥 / ����
//       ���� : Ψһ���ص�ַ       FC00::/7
//              ��·���ص�ַǰ׺   FE80::/10
//       �鲥 : �鲥��ַǰ׺       FF00::/8

// IPv6����֧���м�ڵ��Ƭ����֧�ֶ˵��˵ķ�Ƭ

typedef struct in_addr             IPv4Addr_T;
typedef struct in6_addr            IPv6Addr_T;

#ifndef  IPV6_ADDRESS_BITS         //IPv6��ַ���ַ���λ��
#define  IPV6_ADDRESS_BITS         (sizeof(IN6_ADDR) * 8)
#endif
#ifdef   INET6_ADDRSTRLEN
#define  MAX_IPV6_ADDRSTR_LEN      INET6_ADDRSTRLEN
#elif defined(INET6_ADDR_LEN)
#define  MAX_IPV6_ADDRSTR_LEN      INET6_ADDR_LEN
#else                               
#define  MAX_IPV6_ADDRSTR_LEN      64
#endif   //(>=46):IPv6��ַ�ַ�������󳤶�

/* Interface index */
#ifdef WINDOWS
#define  IfIndex                   EULONG  //DWORD
#else
#define  IfIndex                   EUINT32
#endif

/* IPV6_MTU_DISCOVER values */
#ifdef IPV6_MTU_DISCOVER
#ifndef IPV6_PMTUDISC_DONT
#define IPV6_PMTUDISC_DONT	   0	/* Never send DF frames.  */
#endif
#ifndef IPV6_PMTUDISC_WANT
#define IPV6_PMTUDISC_WANT	   1	/* Use per route hints.  */
#endif
#ifndef IPV6_PMTUDISC_DO
#define IPV6_PMTUDISC_DO	   2	/* Always DF.  */
#endif
#ifndef IPV6_PMTUDISC_PROBE
#define IPV6_PMTUDISC_PROBE	   3	/* Ignore dst pmtu.  */
#endif
#elif defined(IPV6_USE_MIN_MTU)
#define IPV6_PMTUDISC_DONT    0	/* Never send DF frames , use min MTU*/
#define IPV6_PMTUDISC_DO      1 /* Always DF.  */
#define IPV6_PMTUDISC_WANT   -1 /* Use per route hints.  */
#endif

#if defined(LINUX)
struct if_nameindex
{
    unsigned int if_index;	/* 1, 2, ... */
    char* if_name;		/* null terminated name: "eth0", ... */
};
#endif

#if defined(LINUX) || defined(VXWORKS)
/* Convert an interface name to an index, and vice versa.  */
extern unsigned int if_nametoindex(const char* __ifname);
extern char* if_indextoname(unsigned int __ifindex, char* __ifname);

/* Return a list of all interfaces and their indices.  */
extern struct if_nameindex* if_nameindex(void);

/* Free the data returned from if_nameindex.  */
extern void if_freenameindex(struct if_nameindex* __ptr);
#endif

/************************************************************************
  Function:       TCPIPv6_init
  Description:    ��IPv6��TCPIPЭ����г�ʼ��
  Input:          ��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 TCPIP_init (void);

/************************************************************************
  Function:       TCPIPv6_discard
  Description:    IPv6��TCPIPЭ����ֹ����
  Input:          ��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 TCPIP_discard (void);

/************************************************************************
  Function:       CloseSocket
  Description:    �ر�һ��Socket
  Input:          id:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �Ϸ��Ĺر�Ӧ��shutdown�������������ݺ���closesocket
                  �˴�Ϊֱ�ӹر�������з���
************************************************************************/
extern EINT32 CloseSocket (EFD_T id);

/************************************************************************
  Function:       CalculateChecksum
  Description:    ����IPv4/IPv6���ݰ�У���
  Input:          pData:��Ҫ����У������ݲ���
                  len: ���ݲ��ֵĳ���,��λbit
  Output:         ��
  Return:         У���
  Others:         ��
************************************************************************/
extern EUINT16 CalculateChecksum (ECHAR * pData, EUINT32 len);

/************************************************************************
  Function:       MapIPv4toIPv6
  Description:    ��������ʽ��IPv4��ַӳ��ɺϷ���ASCII����ʽ��IPv6��ַ
  Input:          ipv4: ������ʽ��IPv4��ַ
  Output:         ipv6: ���IPv6��ַASCII����ʽ���ַ���ָ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         ��
************************************************************************/
extern EINT32 MapIPv4toIPv6 (EUINT32 ipv4, ECHAR * ipv6);

/************************************************************************
  Function:       GetIPVersion
  Description:    ����IP��ַ��ȡ��IP�汾
  Input:          ip: ������(IP��ַ)��ASCII����ʽ���ַ���ָ��
  Output:         ��
  Return:         ETERROR - ����ʧ��  IPv4/IPv6 - ����ɹ�
  Others:         ��
************************************************************************/
extern EUINT16 GetIPVersion (ECHAR * ip);

/************************************************************************
  Function:       GetSockIPVersion
  Description:    ��ȡSocket��IP�汾
  Input:          sockId: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  IPv4/IPv6 - ����ɹ�
  Others:         �˽ӿڽ������Ѿ���bind��socket
************************************************************************/
extern EUINT16 GetSockIPVersion (EFD_T sockId);

/************************************************************************
  Function:       GetSockPort
  Description:    ��ȡSocket�󶨵Ķ˿�
  Input:          sockId: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  else - ����ɹ�
  Others:         �˽ӿڼ���IPv4/IPv6���������Ѿ���bind��socket
************************************************************************/
extern EUINT16 GetSockPort (EFD_T sockId);

/************************************************************************
  Function:       GetSockAddr
  Description:    ��ȡSocket�󶨵�IP��ַ
  Input:          sockId: �׽���
  Output:         s: ָ��IPv6��ַASCII����ʽ���ַ���ָ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         �˽ӿڼ���IPv4/IPv6���������Ѿ���bind��socket
************************************************************************/
extern EINT32 GetSockAddr (EFD_T sockId, ECHAR * s);

/************************************************************************
  Function:       OpenSockReuse
  Description:    ����Socket�׽���ID��(close֮��)����
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 OpenSockReuse (EFD_T sock);

/************************************************************************
  Function:       CloseSockReuse
  Description:    ��ֹSocket�׽���ID������
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 CloseSockReuse (EFD_T sock);

/************************************************************************
  Function:       SetSockDontRoute
  Description:    ����socket������·��/���ض�ֱ�ӷ���Ŀ������
  Input:          sockId: ���õ�socketID
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         ��
************************************************************************/
extern EINT32 SetSockDontRoute (EFD_T sockId);

/************************************************************************
  Function:       IsIpAddrV6MappedFromV4
  Description:    �ж�IPv6ASCII����ʽ���ַ����ĵ�ַ�Ƿ�����IPv4��ַӳ�����
  Input:          IPAddrv6: IPv6��ַ�ַ���ָ��
  Output:         ��
  Return:         ETFALSE - �ж�ʧ��(��ͬ/�쳣)  ETTRUE - �жϳɹ�(��ͬ)
  Others:         ��
************************************************************************/
extern EINT32 IsIpAddrV6MappedFromV4 (ECHAR * IPAddrv6);

/************************************************************************
  Function:       IsIpAddrV6CompatV4
  Description:    �ж�IPv6ASCII����ʽ���ַ����ĵ�ַ�Ƿ��IPv4��ַ����
  Input:          IPAddrv6: IPv6��ַ�ַ���ָ��
  Output:         ��
  Return:         ETFALSE - �ж�ʧ��(��ͬ/�쳣)  ETTRUE - �жϳɹ�(��ͬ)
  Others:         ��
************************************************************************/
extern EINT32 IsIpAddrV6CompatV4 (ECHAR * IPAddrv6);

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    ����Socket���ջ�������С
  Input:          sock:�׽��־��
                  bufferSize: ���õĴ�Сֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 SetSockRCVBuffer (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    ��ȡSocket���ջ�������С
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:bufferSize, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 GetSockRCVBuffer (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    ����Socket���ͻ�������С
  Input:          sock:�׽��־��
                  bufferSize: ���õĴ�Сֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 SetSockSNDBuffer (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    ��ȡSocket���ͻ�������С
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:bufferSize, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 GetSockSNDBuffer (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVLowWat
  Description:    ����Socket���ջ�������ˮλ���
                  ��ˮλ�����IO���ú���select��poll��epoll����ʹ��
                  ��ֵΪ��������ʱ�׽��ֽ���/���ͻ����������������������
  Input:          sock:�׽��־��
                  bufferSize: ���õĴ�Сֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 SetSockRCVLowWat (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVLowWat
  Description:    ��ȡSocket���ջ�������ˮλ���
                  ��ˮλ�����IO���ú���select��poll��epoll����ʹ��
                  ��ֵΪ��������ʱ�׽��ֽ���/���ͻ����������������������
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:bufferSize, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 GetSockRCVLowWat (EFD_T sock);

/************************************************************************
  Function:       SetSockSNDLowWat
  Description:    ����Socket���ͻ�������ˮλ���
                  ��ˮλ�����IO���ú���select��poll��epoll����ʹ��
                  ��ֵΪ��������ʱ�׽��ֽ���/���ͻ����������������������
  Input:          sock:�׽��־��
                  bufferSize: ���õĴ�Сֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 SetSockSNDLowWat (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockSNDLowWat
  Description:    ��ȡSocket���ͻ�������ˮλ���
                  ��ˮλ�����IO���ú���select��poll��epoll����ʹ��
                  ��ֵΪ��������ʱ�׽��ֽ���/���ͻ����������������������
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:bufferSize, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
************************************************************************/
extern EINT32 GetSockSNDLowWat (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVTimeout
  Description:    ����Socket�׽��ֵĽ��ճ�ʱֵ
  Input:          sock:�׽��־��
                  time: ���õĳ�ʱֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  ���ճ�ʱӰ�����Ҫ����:read/readv/recv/recvfrom/recvmsg
                  ����ͨ������ֵΪ0����ֹ��ʱ
************************************************************************/
extern EINT32 SetSockRCVTimeout (EFD_T sock, EINT32 time);

/************************************************************************
  Function:       GetSockRCVTimeout
  Description:    ��ȡSocket�׽��ֵĽ��ճ�ʱֵ
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:TimeoutValue, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  ���ճ�ʱӰ�����Ҫ����:read/readv/recv/recvfrom/recvmsg
************************************************************************/
extern EINT32 GetSockRCVTimeout (EFD_T sock);

/************************************************************************
  Function:       SetSockSNDTimeout
  Description:    ����Socket�׽��ֵķ��ͳ�ʱֵ
  Input:          sock:�׽��־��
                  time: ���õĳ�ʱֵ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  ���ͳ�ʱӰ�����Ҫ����:write/writev/send/sendto/sendmsg
                  ����ͨ������ֵΪ0����ֹ��ʱ
************************************************************************/
extern EINT32 SetSockSNDTimeout (EFD_T sock, EINT32 time);

/************************************************************************
  Function:       GetSockSNDTimeout
  Description:    ��ȡSocket�׽��ֵķ��ͳ�ʱֵ
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:TimeoutValue, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  ���ͳ�ʱӰ�����Ҫ����:write/writev/send/sendto/sendmsg
************************************************************************/
extern EINT32 GetSockSNDTimeout (EFD_T sock);

/************************************************************************
  Function:       SetSockLinger
  Description:    ����Socket�׽���(�������ݴ�����)�ӳٹر�
  Input:          sock:�׽��־��
                  onoff: Ϊ1���linger, Ϊ0��ر�linger
                  linger_time: linger������ʱ��, ��λms
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  lingerֵ��closesocket()����,��������û������ϵ�ʱ��������
************************************************************************/
extern EINT32 SetSockLinger (EFD_T sock, EINT32 onoff, EINT32 linger_time);

/************************************************************************
  Function:       GetSockLinger
  Description:    ��ȡSocket�׽���(�������ݴ�����)�ӳٹر�ʱ��
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:linger_time������ʱ��, ʧ��:ETERROR
  Others:         �˽ӿڼ���IPv4/IPv6
                  lingerֵ��closesocket()����,��������û������ϵ�ʱ��������
************************************************************************/
extern EINT32 GetSockLinger (EFD_T sock);

/************************************************************************
  Function:       GetIfIndex
  Description:    ������������ȡ������ֵ
  Input:          ifname:�������ַ�����ַ
  Output:         ��
  Return:         �ɹ�:IfIndex, ʧ��:ETERROR
  Others:         �˽ӿ�����linux/vxworks
************************************************************************/
extern IfIndex GetIfIndex(ECHAR * ifname);

/************************************************************************
  Function:       GetIfName
  Description:    ������������ֵ��ȡ������
  Input:          ifindex:��������ֵ
  Output:         ifname:�������ַ�����ַ
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿ�����linux/vxworks
************************************************************************/
extern EINT32 GetIfName(IfIndex ifindex, ECHAR * ifname);

/************************************************************************
  Function:       OpenRawSocket
  Description:    ��һ��ԭʼIPv4 Socket
  Input:          protocol:���յ�IP���ݰ�����
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EFD_T OpenRawSocket (EINT32 protocol);

/************************************************************************
  Function:       OpenSocket
  Description:    ��һ��IPv4 Socket
  Input:          type:ESOCK_STREAM:stream socket��ESOCK_DGRAM:datagram socket��
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EFD_T OpenSocket (EINT32 type);

/************************************************************************
  Function:       SetSocketNonblock
  Description:    ��IPv4 Socket����Ϊ��������ʽ
  Input:          sockv4:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetSocketNonblock (EFD_T sockv4);

/************************************************************************
Function:       SetSocketLoosRoute
Description:    ��IPv4 Socket����Ϊ��ɢ·��ģʽ
Input:          sockv4:Socket���, addr:ָ��·��IP��ַ�б��ָ��, num:·�ɱ���·�ɵ�ַ�ĸ���
Output:         ��
Return:         �ɹ�:ETOK, ʧ��:ETERROR
Others:         ��
************************************************************************/
extern EINT32 SetSocketLoosRoute (EFD_T sockv4, EUINT32 * addr, EUINT32 num);

/************************************************************************
  Function:       SetSocketQos
  Description:    ����IPv4 socket��qos����
  Input:          sockv4:Socket���; buf[]: ����; qos:qos����;
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetSocketQos (EFD_T sockv4, EUINT32 qos);

/************************************************************************
  Function:       BindSocket
  Description:    ��IPv4 Socket�뱾�ض˿ڰ�
  Input:          sockv4:Socket���, port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 BindSocket (EFD_T sockv4, EUINT16 port);

/************************************************************************
  Function:       BindSrcIpSocket
  Description:    ��IPv4 Socket�뱾�ض˿ڰ�
  Input:          sockv4: Socket���
                   srcIp: ip��ַ
                   port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 BindSrcIpSocket (EFD_T sockv4, EUINT32 srcIp, EUINT16 port);

/************************************************************************
  Function:       ListenSocket
  Description:    ����IPv4 Socket����
  Input:          sockv4:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ListenSocket (EFD_T sockv4);

/************************************************************************
  Function:       AcceptSocket
  Description:    ����IPv4�ͻ��˵�����
  Input:          sockv4:Socket���;
  Output:         pClientIpAddr: �ͻ���IP��ַ
  Return:         �ɹ�:�½���Socket��� , ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EADDR_T AcceptSocket (EFD_T sockv4, EUINT32 * pClientIpAddr);

/************************************************************************
  Function:       ConnectSocket
  Description:    IPv4�ͻ��˺��з�����
  Input:          sockv4:Socket���; remoteIpAddr: ������IP��ַ;port:�˿�
  Output:         ��
  Return:         �ɹ�:ETOK , ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ConnectSocket (EFD_T sockv4, EUINT32 remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadFrame
  Description:    ����IPv4 UDP֡����
  Input:          sockv4:Socket���; len:�������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadFrame (EFD_T sockv4, ECHAR buf[], EUINT32 len, EUINT32 * pRemoteIpAddr);

/************************************************************************
  Function:       ReadFrameExt
  Description:    ����IPv4 UDP֡����
  Input:          sockv4: Socket���; len:�������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ; pRemotePort: ����֡Դ�˿�
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadFrameExt (EFD_T sockv4, ECHAR buf[], EUINT32 len, EUINT32* pRemoteIpAddr, EUINT32 * pRemotePort);

/************************************************************************
  Function:       SendFrame
  Description:    ����IPv4 UDP֡����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
                   remoteIpAddr: ����֡Ŀ��IP��ַ;port:�˿�
  Output:         ��
  Return:         �ɹ�: ���͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SendFrame (EFD_T sockv4, ECHAR buf[], EUINT32 len, EINT32 remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadStream
  Description:    ����IPv4֡���ݵĻ�����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadStream (EFD_T sockv4, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       SendStream
  Description:    ����IPv4֡����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SendStream (EFD_T sockv4, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       GetIpAddr
  Description:    ��ASCII����ʽ��IPv4��ַת��������
  Input:          s:ָ��IPv4��ַASCII����ʽ��ָ��
  Output:         ��
  Return:         IP��ַ����������
  Others:         ��
************************************************************************/
extern EUINT32 GetIpAddr (ECHAR * s);

/************************************************************************
  Function:       GetIpStr
  Description:    �����͵�IPv4��ַת����ASCII����ʽ
  Input:          ipAddr:�������͵�IPv4��ַ
  Output:         s:ָ��IP��ַASCII����ʽ��ָ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 GetIpStr (EUINT32 ipAddr, ECHAR * s);

/************************************************************************
  Function:       ReadFromTo
  Description:    ʹ��recvmsg����IPv4���ݰ���ʹ�ÿ��Ի�ȡipͷ��Ŀ���ַ
  Input:          sockv4: �����׽���
                  pRecvBuf: ������ݵĻ�����
                  bufLen: ��������С
                  pSrcIp: ���ݰ���Դ�˵�ַ(�����ֽ���)
                  pSrcPort: ���ݰ���Դ�˿ں�(������)
                  pDstIp: ���ݰ���Ŀ�ĵ�ַ(�����ֽ���)
                  pDstPort: ���ݰ���Ŀ�Ķ˿ں�(�����ֽ���)
  Output:         ��
  Return:         �ɹ�: �������ݵĳ���;ʧ��: ETERROR
  Others:         �˽ӿ�����linux\vxworks\unix
************************************************************************/
extern EINT32 ReadFromTo (EFD_T sockv4, void * pRecvBuf, EUINT16 bufLen,
    EUINT32 * pSrcIp, EUINT32 * pDstIp);

/************************************************************************
  Function:       EtCreateRoute
  Description:    ����IPv4·��
  Input:          pDestIpAddr: Ŀ���ַ
                  intIpMask: Ŀ���ַ����
                  pNextHop: ��һ����ַ
  Output:         ��
  Return:
  Others:         �˽ӿ�����linux\vxworks
************************************************************************/
#if defined(LINUX)
extern EINT32 EtCreateRoute (ECHAR * pDestIpAddr,
    EUINT32 intIpMask,
    ECHAR * pNextHop,
    ECHAR * pDevName);

#else
extern EINT32 EtCreateRoute (ECHAR * pDestIpAddr,
    EUINT32 intIpMask,
    ECHAR * pNextHop);
#endif

/************************************************************************
  Function:       EtDeleteRoute
  Description:    ɾ��IPv4·��
  Input:          pDestIpAddr: Ŀ���ַ
                  intIpMask: Ŀ���ַ����
                  pNextHop: ��һ����ַ
  Output:         ��
  Return:
  Others:         �˽ӿ�����linux\vxworks
************************************************************************/
#if defined(LINUX)
extern EINT32 EtDeleteRoute (ECHAR * pDestIpAddr,
    EUINT32 intIpMask,
    ECHAR * pNextHop,
    ECHAR * pDevName);
#endif

/************************************************************************
  Function:       OpenSockBroadCast
  Description:    ��IPv4 Socket�����͹㲥����
  Input:          sockv4: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         1��IPv6�򲻴��ڹ㲥��ʽ
                  2����UDPЭ���ʹ�ô˽ӿ�
************************************************************************/
extern EINT32 OpenSockBroadCast (EFD_T sockv4);

/************************************************************************
  Function:       CloseSockBroadCast
  Description:    �ر�IPv4 Socket�����͹㲥����
  Input:          sockv4: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         1��IPv6�򲻴��ڹ㲥��ʽ
                  2����UDPЭ���ʹ�ô˽ӿ�
************************************************************************/
extern EINT32 CloseSockBroadCast (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMulticastTTL
  Description:    ����Socket IPv4���ݰ��жಥ��ַTTL
  Input:          sockv4:�׽��־��
                  TTL:���ʱ��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPMulticastTTL (EINT32 TTL, EFD_T sockv4);

/************************************************************************
  Function:       GetIPMulticastTTL
  Description:    ��ȡSocket IPv4���ݰ��жಥ��ַTTL
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:hoplimit, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 GetIPMulticastTTL (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMCInterface
  Description:    ����Socket IPv4�ಥ����ӿ�(����)
  Input:          sockv4:�׽��־��
                  If:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPMCInterface (IfIndex If, EFD_T sockv4);

/************************************************************************
  Function:       GetIPMCInterface
  Description:    ��ȡSocket IPv4�ಥ����ӿ�(����)
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern IfIndex GetIPMCInterface (EFD_T sockv4);

/************************************************************************
  Function:       SetIPUCInterface
  Description:    ����Socket IPv4��������ӿ�(����)
  Input:          sockv4:�׽��־��
                  If:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPUCInterface (IfIndex If, EFD_T sockv4);

/************************************************************************
  Function:       GetIPUCInterface
  Description:    ����SocketIPv4��������ӿ�(����)
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern IfIndex GetIPUCInterface (EFD_T sockv4);

/************************************************************************
  Function:       SetIPIfLoop
  Description:    ����Socket IPv4�Ƿ񻷻�
  Input:          sockv4:�׽��־��
                  IfLoop:�Ƿ񻷻��ж�ֵETTRUE/ETFALSE
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPIfLoop (EUCHAR IfLoop, EFD_T sockv4);

/************************************************************************
  Function:       GetIPIfLoop
  Description:    ��ȡSocket IPv4�Ƿ񻷻�
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:IfLoop(ETTRUE/ETFALSE), ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EUCHAR GetIPIfLoop (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMultiCast
  Description:    ����Socket IPv4�ಥ��ַ(����ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 SetIPMultiCast (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr);

/************************************************************************
  Function:       SetIPMultiCastExt
  Description:    ����Socket IPv4�ಥ��ַ(����ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡSrcAddr
                  SrcAddr: ԴIPv4��ַ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 SetIPMultiCastExt (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       DropIPMultiCast
  Description:    ȡ��Socket IPv4�ಥ��ַ(�뿪�ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 DropIPMultiCast (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr);

/************************************************************************
  Function:       DropIPMultiCastExt
  Description:    ȡ��SocketIPv4�ಥ��ַ(�뿪�ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
                  SrcAddr: ԴIPv4��ַ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 DropIPMultiCastExt (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       BlockIPSource
  Description:    ����SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
                  SrcAddr: ԴIPv4��ַ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 BlockIPSource (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       UnblockIPSource
  Description:    ��ͨ������SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
                  SrcAddr: ԴIPv4��ַ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 UnblockIPSource (ECHAR* Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       GetIpAddrV6
  Description:    ��ASCII����ʽ��IP��ַת����IPv6Addr_T�ṹ������
  Input:          s:ָ��IP��ַASCII����ʽ���ַ���ָ��
  Output:         ��
  Return:         IP��ַ��IPv6Addr_T�ṹ������
  Others:         ��
************************************************************************/
extern IPv6Addr_T GetIpAddrV6 (ECHAR * s);

/************************************************************************
  Function:       GetIpAddrV6Str
  Description:    ��IPv6Addr_T�ṹ����ʽ��IPv6��ַת����ASCII����ʽ���ַ���
  Input:          sin6_addr:IPv6��ַ�ṹ��
  Output:         IpAddrV6Str:ָ��IPv6��ַASCII����ʽ���ַ���ָ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         ��
************************************************************************/
extern EINT32 GetIpAddrV6Str (IPv6Addr_T sin6_addr, ECHAR * IpAddrV6Str);

/************************************************************************
  Function:       IsIpAddrV6Equal
  Description:    �ж�����IPv6ASCII����ʽ���ַ����ĵ�ַ�Ƿ���ͬ
  Input:          IPv6a: ��һ��IPv6��ַ�ַ���ָ��
                   IPv6b: �ڶ���IPv6��ַ�ַ���ָ��
  Output:         ��
  Return:         ETFALSE - �ж�ʧ��(��ͬ/�쳣)  ETTRUE - �жϳɹ�(��ͬ)
  Others:         ��
************************************************************************/
extern EINT32 IsIpAddrV6Equal (ECHAR * IPv6a, ECHAR * IPv6b);

/************************************************************************
  Function:       IsIpAddrV6LoopBack
  Description:    �ж�һ��IPv6ASCII����ʽ���ַ����ĵ�ַ�Ƿ��ʾ���ص�ַ
  Input:          IpAddrV6Str: IPv6��ַ�ַ���ָ��
  Output:         ��
  Return:         ETFALSE - �ж�ʧ��(����/�쳣)  ETTRUE - �жϳɹ�(��)
  Others:         ��
************************************************************************/
extern EINT32 IsIpAddrV6LoopBack (ECHAR * IpAddrV6Str);

/************************************************************************
  Function:       OpenRawSocketV6
  Description:    ��һ��ԭʼIPv6 Socket
  Input:          protocol:���յ�IP���ݰ�����
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EFD_T OpenRawSocketV6 (EINT32 protocol);

/************************************************************************
  Function:       OpenSocketV6
  Description:    ��һ��IPv6 Socket
  Input:          type:WSOCK_STREAM:stream socket��WSOCK_DGRAM:datagram socket��
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EFD_T OpenSocketV6 (EINT32 type);

/************************************************************************
  Function:       SetSocketNonblockV6
  Description:    ��IPv6 Socket����Ϊ��������ʽ
  Input:          id:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetSocketNonblockV6 (EFD_T id);

/************************************************************************
Function:       SetSocketLoosRouteV6
Description:    ��IPv6 Socket����Ϊ��ɢ·��ģʽ/���·�ɵ�ַ
Input:          id: Socket���
                segments: ��������
                addr: ��·���ײ�����ӵ�·�ɵ�ַ��ָ��
Output:         ��
Return:         �ɹ�:ETOK, ʧ��:ETERROR
Others:         �˽ӿڽ�������linux/vxworks
************************************************************************/
extern EINT32 SetSocketLoosRouteV6 (EFD_T id, EINT32 segments, IPv6Addr_T * addr);

/************************************************************************
  Function:       SetSocketQosV6
  Description:    ����IPv6 socket��qos����
  Input:          id: Socket���; qos: qos����;
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڲ�������windows
************************************************************************/
extern EINT32 SetSocketQosV6 (EFD_T id, EUINT32 qos);

/************************************************************************
  Function:       BindSocketIPv6
  Description:    ��IPv6 Socket�뱾�ض˿ڰ�
  Input:          id:Socket���, port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
#define BindSocketIPv6(sockv6, port) BindSocketIPv6Ext(sockv6, NULL, port, 0, 0, 0)

/************************************************************************
  Function:       BindSrcSocketIPv6
  Description:    ��IPv6 Socket�뱾�ض˿ڰ�
  Input:          id: Socket���
                  srcIp: ip��ַ
                  port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
#define BindSrcSocketIPv6(sockv6, ip, port) BindSocketIPv6Ext(sockv6, ip, port, 0, 0, 0)

/************************************************************************
  Function:       BindSocketIPv6Ext
  Description:    ��IPv6 Socket�뱾�ض˿ڰ�
  Input:          id: Socket���
                  srcIp: ip��ַ(NULL�������IP)
                  port: �˿�
                  qos: qos����(ռ8λ & 255)
                  flowLable: ����ǩ(ռ20λ & 1048575)
                  scopeId: ������ID(�����ڱ������ӣ����������ڼ�%׷����IPv6��ַ����ͬID��socket����ͨ��)
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         Ĭ�����ú���λ���Ϊ0(may deprecated)
************************************************************************/
extern EINT32 BindSocketIPv6Ext (EFD_T id, ECHAR * srcIp, EUINT16 port, EUINT32 qos, EUINT32 flowLabel, IfIndex scopeId);

/************************************************************************
  Function:       ListenSocketV6
  Description:    ����IPv6 Socket����
  Input:          id:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ListenSocketV6 (EFD_T id);

/************************************************************************
  Function:       AcceptSocketV6
  Description:    ����IPv6�ͻ��˵�����
  Input:          id:Socket���;
  Output:         pClientIpAddr: �ͻ���IP��ַ
  Return:         �ɹ�: �½���Socket���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EFD_T AcceptSocketV6 (EFD_T id, ECHAR * pClientIpAddr);

/************************************************************************
  Function:       ConnectSocketV6
  Description:    IPv6 �ͻ��˺��з�����
  Input:          id:Socket���; remoteIpAddr: ������IP��ַ; port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK , ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ConnectSocketV6 (EFD_T id, ECHAR * remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadFrameV6
  Description:    ����IPv6 UDP֡����
  Input:          id: Socket���; len: �������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadFrameV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * pRemoteIpAddr);

/************************************************************************
  Function:       ReadFrameExtV6
  Description:    ����IPv6 UDP֡����
  Input:          id:Socket���; len:�������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ; pRemotePort: ����֡ԴIP�˿�
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadFrameExtV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * pRemoteIpAddr, EUINT32 * pRemotePort);

/************************************************************************
  Function:       SendFrameV6
  Description:    ����IPv6 UDP֡����
  Input:          id: Socket���; buf[]: ����; len: �������ĳ���;
                  remoteIpAddr: ����֡Ŀ��IP��ַ; dstPort: �˿�
  Output:         ��
  Return:         �ɹ�: ���͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SendFrameV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * remoteIpAddr, EUINT16 dstPort);

/************************************************************************
  Function:       ReadStreamV6
  Description:    ����IPv6֡���ݵĻ�����
  Input:          id: Socket���; buf[]: ����; len: �������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 ReadStreamV6 (EFD_T id, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       SendStreamV6
  Description:    ����IPv6֡����
  Input:          id: Socket���; buf[]: ����; len: �������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SendStreamV6 (EFD_T id, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       ReadFromToV6
  Description:    ʹ��recvmsg�������ݰ���ʹ�ÿ��Ի�ȡipv6ͷ��Ŀ���ַ
  Input:          s: �����׽���
                  bufLen: ��������С
  Output:         pRecvBuf: ������ݵĻ�����
                  pSrcIp: ���ݰ���Դ��IPv6��ַ(�����ֽ���)
                  pDstIp: ���ݰ���Ŀ��IPv6��ַ(�����ֽ���)
  Return:         �ɹ�: �������ݵĳ���;ʧ��: ETERROR
  Others:         �˽ӿ�����linux\vxworks\unix
************************************************************************/
extern EINT32 ReadFromToV6 (EFD_T s, void * pRecvBuf, EUINT16 bufLen,
    ECHAR * pSrcIp, ECHAR * pDstIp);

/************************************************************************
  Function:       EtCreateRouteV6
  Description:    ����·��
  Input:          pDestIpAddr: Ŀ��IPv6��ַ
                  pNextHop: ��һ����ַ
                  pDevName: �豸����
                  pNetMask: ��������
                  mtu: ��·������䵥Ԫ
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         �˽ӿ�����linux\vxworks
************************************************************************/
#ifdef LINUX
extern EINT32 EtCreateRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pDevName, EUINT32 mtu);
#else  //VXWORKS
extern EINT32 EtCreateRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pNetMask);
#endif

/************************************************************************
  Function:       EtDeleteRouteV6
  Description:    ɾ��·��
  Input:          pDestIpAddr: Ŀ��IPv6��ַ
                  pNextHop: ��һ����ַ
                  pDevName: �豸����
                  pNetMask: ��������
                  mtu: ��·������䵥Ԫ
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         �˽ӿ�����linux\vxworks
************************************************************************/
#ifdef LINUX
extern EINT32 EtDeleteRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pDevName, EUINT32 mtu);
#else  //VXWORKS
extern EINT32 EtDeleteRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pNetMask);
#endif

/************************************************************************
  Function:       OpenIPv6Only
  Description:    ��IPv6�׽��ֽ�����IPv6����
  Input:          sockId: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         ��
************************************************************************/
extern EINT32 OpenIPv6Only (EFD_T sockId);

/************************************************************************
  Function:       CloseIPv6Only
  Description:    �ر�IPv6�׽��ֽ�����IPv6���Ĺ���
  Input:          sockId: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         ��
************************************************************************/
extern EINT32 CloseIPv6Only (EFD_T sockId);

/************************************************************************
  Function:       SetIPv6UnicastHops
  Description:    ����SocketIPv6���ݰ��е�����ַ��������
  Input:          sockv6:�׽��־��
                  hopLimit:������ַ��������
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPv6UnicastHops (EINT32 hopLimit, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6UnicastHops
  Description:    ��ȡSocketIPv6���ݰ��е�����ַ��������
  Input:          sockv6:�׽��־��
  Output:         ��
  Return:         �ɹ�:hoplimit, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 GetIPv6UnicastHops (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MulticastHops
  Description:    ����SocketIPv6���ݰ��жಥ��ַ��������
  Input:          sockv6:�׽��־��
                  hopLimit:�ಥ��ַ��������
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPv6MulticastHops (EINT32 hopLimit, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6MulticastHops
  Description:    ��ȡSocket IPv6���ݰ��жಥ��ַ��������
  Input:          sockv6:�׽��־��
  Output:         ��
  Return:         �ɹ�:hoplimit, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 GetIPv6MulticastHops (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MCInterface
  Description:    ����Socket IPv6�ಥ����ӿ�(����)
  Input:          sockv6:�׽��־��
                  iF:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPv6MCInterface (IfIndex If, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6MCInterface
  Description:    ��ȡSocket IPv6�ಥ����ӿ�(����)
  Input:          sockv6:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern IfIndex GetIPv6MCInterface (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6UCInterface
  Description:    ����Socket IPv6��������ӿ�(����)
  Input:          sockv6:�׽��־��
                  If:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPv6UCInterface (IfIndex If, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6UCInterface
  Description:    ��ȡSocket IPv6��������ӿ�(����)
  Input:          sockv6:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern IfIndex GetIPv6UCInterface (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    ����SocketIPv6�Ƿ񻷻�
  Input:          sockv6:�׽��־��
                  IfLoop:�Ƿ񻷻��ж�ֵETTRUE/ETFALSE
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 SetIPv6IfLoop (EUCHAR IfLoop, EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    ����SocketIPv6�Ƿ񻷻�
  Input:          sockv6:�׽��־��
                   IfLoop:�Ƿ񻷻��ж�ֵ
  Output:         ��
  Return:         �ɹ�:IfLoop(ETTRUE/ETFALSE), ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EUCHAR GetIPv6IfLoop (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MultiCast
  Description:    ����Socket IPv6�ಥ��ַ(����ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv6:�׽��־��
                  IfAddr6: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 SetIPv6MultiCast (ECHAR * Addr, EFD_T sockv6, IfIndex IfAddr6);

/************************************************************************
  Function:       DropIPv6MultiCast
  Description:    ȡ��Socket IPv6�ಥ��ַ(�뿪�ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv6:�׽��־��
                  IfAddr6: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
************************************************************************/
extern EINT32 DropIPv6MultiCast (ECHAR * Addr, EFD_T sockv6, IfIndex IfAddr6);

/************************************************************************
  Function:       BlockIPv6Source
  Description:    ����SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڽ������Ѿ���bind��socket
                  Դsource��Ϣ�ɰ���Ϣ��ȡ
************************************************************************/
#define BlockIPv6Source(Addr, sock) BlockIPSourceExt(Addr, sock, 0) /* grp_port = 0 to Ignore */

/************************************************************************
  Function:       BlockIPv6SourceExt
  Description:    ����SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sock:�׽��־��
                  grp_port: �ಥԴ�˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڽ������Ѿ���bind��socket
                  Դsource��Ϣ�ɰ���Ϣ��ȡ
************************************************************************/
extern EINT32 BlockIPv6SourceExt (ECHAR * Addr, EFD_T sock, EUINT16 grp_port);

/************************************************************************
  Function:       UnblockIPv6Source
  Description:    ��ͨ������SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڽ������Ѿ���bind��socket
                  Դsource��Ϣ�ɰ���Ϣ��ȡ
************************************************************************/
#define UnblockIPv6Source(Addr, sock) UnblockIPSourceExt(Addr, sock, 0)  /* grp_port = 0 to Ignore */

/************************************************************************
  Function:       UnblockIPv6SourceExt
  Description:    ��ͨ������SocketIP�ಥԴ��ַ
  Input:          Addr:�ಥԴ��ַ
                  sock:�׽��־��
                  grp_port: �ಥԴ�˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿڽ������Ѿ���bind��socket
                  Դsource��Ϣ�ɰ���Ϣ��ȡ
************************************************************************/
extern EINT32 UnblockIPv6SourceExt (ECHAR * Addr, EFD_T sock, EUINT16 grp_port);

/************************************************************************
  Function:       SetIPv6PMTUD
  Description:    ����IPv6·��MTU���ֹ��ܿ���
  Input:          sock:�׽��־��
                  val: Ϊ 1 ��ʾ: PMTUD���ܲ���ִ�У�Ϊ�����Ƭ�����ʹ����СMTU����
                        Ϊ 0 ����: PMTUD���ܶ�������Ŀ�ĵض���ִ��
                        Ϊ -1��ʾ: PMTUD���ܽ��Ե���Ŀ�ĵ�ִ�У����ڶಥĿ�ĵؾ�ʹ����СMTU
                        (��ѡ��Ĭ��Ϊ-1(����))
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �˽ӿ��ڲ�ͬϵͳ�µ�(��)�����Դ���
************************************************************************/
extern EINT32 SetIPv6PMTUD (EFD_T sock, EINT32 val);

/************************************************************************
  Function:       GetIPv6PMTU
  Description:    ��ȡIPv6�ĵ�ǰ·��Path_MTU
  Input:          sock:�׽��־��
  Output:         ��
  Return:         �ɹ�:PMTUֵ, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EUINT32 GetIPv6PMTU (EFD_T sock);

/*----------------------------multicast--------------------------------*/
/************************************************************************
  Function:       OpenMultiSock
  Description:    ����socket���鲥���ͽӿڵ�ַ�������鲥�������ԣ������鲥�ػ�
  Input:          sId: ��Ҫ�����鲥��socket�����ipAddr:Ĭ�Ϸ����鲥���ӿڵ�ַ
  Output:         ��
  Return:         �ɹ��oETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 OpenMultiSock(EUINT32 sId, EUINT32 ipAddr);

/************************************************************************
  Function:       OpenMultiSockSlefLoop
  Description:    ����socket���鲥���ͽӿڵ�ַ�������鲥��������,�鲥�Իػ�
  Input:          sId: ��Ҫ�����鲥��socket�����ipAddr:Ĭ�Ϸ����鲥���ӿڵ�ַ
  Output:         ��
  Return:         �ɹ��oETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 OpenMultiSockSlefLoop(EUINT32 sId, EUINT32 ipAddr);

/************************************************************************
  Function:       JoinMultiGroup
  Description:    ��socket����ָ�����鲥
  Input:          recvIpAddr: �����鲥ͨѶ�˿ڵ�IP��ַ
                  multiIpAddr:�鲥��ַ��
                  sId:��Ҫ�����鲥��socket
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 JoinMultiGroup(EUINT32 recvIpAddr, EUINT32 multiIpAddr, EUINT32 sId);

/************************************************************************
  Function:       LeaveMultiGroup
  Description:    ��socket����ָ�����鲥
  Input:          recvIpAddr: �����鲥ͨѶ�˿ڵ�IP��ַ
                  multiIpAddr:�鲥��ַ; 
                  sId:��Ҫ�����鲥��socket
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
extern EINT32 LeaveMultiGroup(EUINT32 recvIpAddr, EUINT32 multiIpAddr, EUINT32 sId);

#endif //__BEARPUB_H__
