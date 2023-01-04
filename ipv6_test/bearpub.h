/************************************************************************
Copyright (C), 2009-2010, 
File name:      socketLib.h
Author:       
Date:         
Description:    对不同操作系统的socket接口进行封装，对上层提供统一socket接口
Others:         无
  *************************************************************************/
#ifndef __BEARPUB_H__
#define __BEARPUB_H__

#ifdef __cplusplus
extern "C" {
#endif
/************************************************************************
                            引用的头文件
*************************************************************************/
#include "sys.h"

/************************************************************************
                            宏定义
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
                             函数声明
*************************************************************************/
/*---------------------------socket support---------------------------------*/
#define  IPv4                      AF_INET
#define  IPv6                      AF_INET6
#define  IPv4_Addr_Any             INADDR_ANY
#define  IPv4_Addr_LoopBack        INADDR_LOOPBACK
#define  IPv4_Addr_BroadCast       INADDR_BROADCAST
//       IPv4  ::  单播 / 组播 / 广播
#define  IPv6_Addr_Any             in6addr_any         //地址::/128
#define  IPv6_Addr_LoopBack        in6addr_loopback    //地址::1/128
//       IPv6  ::  单播 / 组播 / 泛播
//       单播 : 唯一本地地址       FC00::/7
//              链路本地地址前缀   FE80::/10
//       组播 : 组播地址前缀       FF00::/8

// IPv6不再支持中间节点分片，仅支持端到端的分片

typedef struct in_addr             IPv4Addr_T;
typedef struct in6_addr            IPv6Addr_T;

#ifndef  IPV6_ADDRESS_BITS         //IPv6地址的字符串位数
#define  IPV6_ADDRESS_BITS         (sizeof(IN6_ADDR) * 8)
#endif
#ifdef   INET6_ADDRSTRLEN
#define  MAX_IPV6_ADDRSTR_LEN      INET6_ADDRSTRLEN
#elif defined(INET6_ADDR_LEN)
#define  MAX_IPV6_ADDRSTR_LEN      INET6_ADDR_LEN
#else                               
#define  MAX_IPV6_ADDRSTR_LEN      64
#endif   //(>=46):IPv6地址字符串的最大长度

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
  Description:    对IPv6的TCPIP协议进行初始化
  Input:          无
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 TCPIP_init (void);

/************************************************************************
  Function:       TCPIPv6_discard
  Description:    IPv6的TCPIP协议终止程序
  Input:          无
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 TCPIP_discard (void);

/************************************************************************
  Function:       CloseSocket
  Description:    关闭一个Socket
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         合法的关闭应先shutdown接收完所有数据后再closesocket
                  此处为直接关闭相关所有服务
************************************************************************/
extern EINT32 CloseSocket (EFD_T id);

/************************************************************************
  Function:       CalculateChecksum
  Description:    计算IPv4/IPv6数据包校验和
  Input:          pData:需要计算校验的数据部分
                  len: 数据部分的长度,单位bit
  Output:         无
  Return:         校验和
  Others:         无
************************************************************************/
extern EUINT16 CalculateChecksum (ECHAR * pData, EUINT32 len);

/************************************************************************
  Function:       MapIPv4toIPv6
  Description:    将整数形式的IPv4地址映射成合法的ASCII码形式的IPv6地址
  Input:          ipv4: 整数形式的IPv4地址
  Output:         ipv6: 输出IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
extern EINT32 MapIPv4toIPv6 (EUINT32 ipv4, ECHAR * ipv6);

/************************************************************************
  Function:       GetIPVersion
  Description:    根据IP地址获取其IP版本
  Input:          ip: 主机名(IP地址)的ASCII码形式的字符串指针
  Output:         无
  Return:         ETERROR - 处理失败  IPv4/IPv6 - 处理成功
  Others:         无
************************************************************************/
extern EUINT16 GetIPVersion (ECHAR * ip);

/************************************************************************
  Function:       GetSockIPVersion
  Description:    获取Socket的IP版本
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  IPv4/IPv6 - 处理成功
  Others:         此接口仅用于已经绑定bind的socket
************************************************************************/
extern EUINT16 GetSockIPVersion (EFD_T sockId);

/************************************************************************
  Function:       GetSockPort
  Description:    获取Socket绑定的端口
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  else - 处理成功
  Others:         此接口兼容IPv4/IPv6但仅用于已经绑定bind的socket
************************************************************************/
extern EUINT16 GetSockPort (EFD_T sockId);

/************************************************************************
  Function:       GetSockAddr
  Description:    获取Socket绑定的IP地址
  Input:          sockId: 套接字
  Output:         s: 指向IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         此接口兼容IPv4/IPv6但仅用于已经绑定bind的socket
************************************************************************/
extern EINT32 GetSockAddr (EFD_T sockId, ECHAR * s);

/************************************************************************
  Function:       OpenSockReuse
  Description:    设置Socket套接字ID可(close之后)重用
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 OpenSockReuse (EFD_T sock);

/************************************************************************
  Function:       CloseSockReuse
  Description:    禁止Socket套接字ID可重用
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 CloseSockReuse (EFD_T sock);

/************************************************************************
  Function:       SetSockDontRoute
  Description:    设置socket不经过路由/网关而直接发往目标主机
  Input:          sockId: 设置的socketID
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
extern EINT32 SetSockDontRoute (EFD_T sockId);

/************************************************************************
  Function:       IsIpAddrV6MappedFromV4
  Description:    判断IPv6ASCII码形式的字符串的地址是否是由IPv4地址映射而来
  Input:          IPAddrv6: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
extern EINT32 IsIpAddrV6MappedFromV4 (ECHAR * IPAddrv6);

/************************************************************************
  Function:       IsIpAddrV6CompatV4
  Description:    判断IPv6ASCII码形式的字符串的地址是否和IPv4地址兼容
  Input:          IPAddrv6: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
extern EINT32 IsIpAddrV6CompatV4 (ECHAR * IPAddrv6);

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    设置Socket接收缓冲区大小
  Input:          sock:套接字句柄
                  bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 SetSockRCVBuffer (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    获取Socket接收缓冲区大小
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 GetSockRCVBuffer (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    设置Socket发送缓冲区大小
  Input:          sock:套接字句柄
                  bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 SetSockSNDBuffer (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    获取Socket发送缓冲区大小
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 GetSockSNDBuffer (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVLowWat
  Description:    设置Socket接收缓冲区低水位标记
                  低水位标记由IO复用函数select、poll、epoll函数使用
                  其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
                  bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 SetSockRCVLowWat (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockRCVLowWat
  Description:    获取Socket接收缓冲区低水位标记
                  低水位标记由IO复用函数select、poll、epoll函数使用
                  其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 GetSockRCVLowWat (EFD_T sock);

/************************************************************************
  Function:       SetSockSNDLowWat
  Description:    设置Socket发送缓冲区低水位标记
                  低水位标记由IO复用函数select、poll、epoll函数使用
                  其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
                  bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 SetSockSNDLowWat (EFD_T sock, EINT32 bufferSize);

/************************************************************************
  Function:       GetSockSNDLowWat
  Description:    获取Socket发送缓冲区低水位标记
                  低水位标记由IO复用函数select、poll、epoll函数使用
                  其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
extern EINT32 GetSockSNDLowWat (EFD_T sock);

/************************************************************************
  Function:       SetSockRCVTimeout
  Description:    设置Socket套接字的接收超时值
  Input:          sock:套接字句柄
                  time: 设置的超时值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  接收超时影响的主要函数:read/readv/recv/recvfrom/recvmsg
                  可以通过设置值为0来禁止超时
************************************************************************/
extern EINT32 SetSockRCVTimeout (EFD_T sock, EINT32 time);

/************************************************************************
  Function:       GetSockRCVTimeout
  Description:    获取Socket套接字的接收超时值
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:TimeoutValue, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  接收超时影响的主要函数:read/readv/recv/recvfrom/recvmsg
************************************************************************/
extern EINT32 GetSockRCVTimeout (EFD_T sock);

/************************************************************************
  Function:       SetSockSNDTimeout
  Description:    设置Socket套接字的发送超时值
  Input:          sock:套接字句柄
                  time: 设置的超时值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  发送超时影响的主要函数:write/writev/send/sendto/sendmsg
                  可以通过设置值为0来禁止超时
************************************************************************/
extern EINT32 SetSockSNDTimeout (EFD_T sock, EINT32 time);

/************************************************************************
  Function:       GetSockSNDTimeout
  Description:    获取Socket套接字的发送超时值
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:TimeoutValue, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  发送超时影响的主要函数:write/writev/send/sendto/sendmsg
************************************************************************/
extern EINT32 GetSockSNDTimeout (EFD_T sock);

/************************************************************************
  Function:       SetSockLinger
  Description:    设置Socket套接字(若有数据待发送)延迟关闭
  Input:          sock:套接字句柄
                  onoff: 为1则打开linger, 为0则关闭linger
                  linger_time: linger逗留的时间, 单位ms
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  linger值由closesocket()调用,当有数据没发送完毕的时候容许逗留
************************************************************************/
extern EINT32 SetSockLinger (EFD_T sock, EINT32 onoff, EINT32 linger_time);

/************************************************************************
  Function:       GetSockLinger
  Description:    获取Socket套接字(若有数据待发送)延迟关闭时间
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:linger_time逗留的时间, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                  linger值由closesocket()调用,当有数据没发送完毕的时候容许逗留
************************************************************************/
extern EINT32 GetSockLinger (EFD_T sock);

/************************************************************************
  Function:       GetIfIndex
  Description:    根据网口名获取其索引值
  Input:          ifname:网口名字符串地址
  Output:         无
  Return:         成功:IfIndex, 失败:ETERROR
  Others:         此接口适用linux/vxworks
************************************************************************/
extern IfIndex GetIfIndex(ECHAR * ifname);

/************************************************************************
  Function:       GetIfName
  Description:    根据网口索引值获取其名字
  Input:          ifindex:网口索引值
  Output:         ifname:网口名字符串地址
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口适用linux/vxworks
************************************************************************/
extern EINT32 GetIfName(IfIndex ifindex, ECHAR * ifname);

/************************************************************************
  Function:       OpenRawSocket
  Description:    打开一个原始IPv4 Socket
  Input:          protocol:接收的IP数据包类型
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
extern EFD_T OpenRawSocket (EINT32 protocol);

/************************************************************************
  Function:       OpenSocket
  Description:    打开一个IPv4 Socket
  Input:          type:ESOCK_STREAM:stream socket，ESOCK_DGRAM:datagram socket，
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
extern EFD_T OpenSocket (EINT32 type);

/************************************************************************
  Function:       SetSocketNonblock
  Description:    将IPv4 Socket设置为非阻塞方式
  Input:          sockv4:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetSocketNonblock (EFD_T sockv4);

/************************************************************************
Function:       SetSocketLoosRoute
Description:    将IPv4 Socket设置为松散路由模式
Input:          sockv4:Socket句柄, addr:指向路由IP地址列表的指针, num:路由表中路由地址的个数
Output:         无
Return:         成功:ETOK, 失败:ETERROR
Others:         无
************************************************************************/
extern EINT32 SetSocketLoosRoute (EFD_T sockv4, EUINT32 * addr, EUINT32 num);

/************************************************************************
  Function:       SetSocketQos
  Description:    设置IPv4 socket的qos类型
  Input:          sockv4:Socket句柄; buf[]: 缓存; qos:qos类型;
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetSocketQos (EFD_T sockv4, EUINT32 qos);

/************************************************************************
  Function:       BindSocket
  Description:    将IPv4 Socket与本地端口绑定
  Input:          sockv4:Socket句柄, port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 BindSocket (EFD_T sockv4, EUINT16 port);

/************************************************************************
  Function:       BindSrcIpSocket
  Description:    将IPv4 Socket与本地端口绑定
  Input:          sockv4: Socket句柄
                   srcIp: ip地址
                   port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 BindSrcIpSocket (EFD_T sockv4, EUINT32 srcIp, EUINT16 port);

/************************************************************************
  Function:       ListenSocket
  Description:    启动IPv4 Socket监听
  Input:          sockv4:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ListenSocket (EFD_T sockv4);

/************************************************************************
  Function:       AcceptSocket
  Description:    接受IPv4客户端的请求
  Input:          sockv4:Socket句柄;
  Output:         pClientIpAddr: 客户端IP地址
  Return:         成功:新建的Socket句柄 , 失败:ETERROR
  Others:         无
************************************************************************/
extern EADDR_T AcceptSocket (EFD_T sockv4, EUINT32 * pClientIpAddr);

/************************************************************************
  Function:       ConnectSocket
  Description:    IPv4客户端呼叫服务器
  Input:          sockv4:Socket句柄; remoteIpAddr: 服务器IP地址;port:端口
  Output:         无
  Return:         成功:ETOK , 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ConnectSocket (EFD_T sockv4, EUINT32 remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadFrame
  Description:    接收IPv4 UDP帧数据
  Input:          sockv4:Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadFrame (EFD_T sockv4, ECHAR buf[], EUINT32 len, EUINT32 * pRemoteIpAddr);

/************************************************************************
  Function:       ReadFrameExt
  Description:    接收IPv4 UDP帧数据
  Input:          sockv4: Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址; pRemotePort: 数据帧源端口
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadFrameExt (EFD_T sockv4, ECHAR buf[], EUINT32 len, EUINT32* pRemoteIpAddr, EUINT32 * pRemotePort);

/************************************************************************
  Function:       SendFrame
  Description:    发送IPv4 UDP帧数据
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
                   remoteIpAddr: 数据帧目的IP地址;port:端口
  Output:         无
  Return:         成功: 发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SendFrame (EFD_T sockv4, ECHAR buf[], EUINT32 len, EINT32 remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadStream
  Description:    接收IPv4帧数据的缓冲区
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadStream (EFD_T sockv4, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       SendStream
  Description:    发送IPv4帧数据
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SendStream (EFD_T sockv4, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       GetIpAddr
  Description:    将ASCII码形式的IPv4地址转换成整型
  Input:          s:指向IPv4地址ASCII码形式的指针
  Output:         无
  Return:         IP地址的整数类型
  Others:         无
************************************************************************/
extern EUINT32 GetIpAddr (ECHAR * s);

/************************************************************************
  Function:       GetIpStr
  Description:    将整型的IPv4地址转换成ASCII码形式
  Input:          ipAddr:整数类型的IPv4地址
  Output:         s:指向IP地址ASCII码形式的指针
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 GetIpStr (EUINT32 ipAddr, ECHAR * s);

/************************************************************************
  Function:       ReadFromTo
  Description:    使用recvmsg接收IPv4数据包，使得可以获取ip头的目标地址
  Input:          sockv4: 接收套接字
                  pRecvBuf: 存放数据的缓冲区
                  bufLen: 缓冲区大小
                  pSrcIp: 数据包的源端地址(网络字节序)
                  pSrcPort: 数据包的源端口号(主机序)
                  pDstIp: 数据包的目的地址(网络字节序)
                  pDstPort: 数据包的目的端口号(主机字节序)
  Output:         无
  Return:         成功: 接收数据的长度;失败: ETERROR
  Others:         此接口用于linux\vxworks\unix
************************************************************************/
extern EINT32 ReadFromTo (EFD_T sockv4, void * pRecvBuf, EUINT16 bufLen,
    EUINT32 * pSrcIp, EUINT32 * pDstIp);

/************************************************************************
  Function:       EtCreateRoute
  Description:    增加IPv4路由
  Input:          pDestIpAddr: 目标地址
                  intIpMask: 目标地址掩码
                  pNextHop: 下一条地址
  Output:         无
  Return:
  Others:         此接口用于linux\vxworks
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
  Description:    删除IPv4路由
  Input:          pDestIpAddr: 目标地址
                  intIpMask: 目标地址掩码
                  pNextHop: 下一条地址
  Output:         无
  Return:
  Others:         此接口用于linux\vxworks
************************************************************************/
#if defined(LINUX)
extern EINT32 EtDeleteRoute (ECHAR * pDestIpAddr,
    EUINT32 intIpMask,
    ECHAR * pNextHop,
    ECHAR * pDevName);
#endif

/************************************************************************
  Function:       OpenSockBroadCast
  Description:    打开IPv4 Socket允许发送广播数据
  Input:          sockv4: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         1、IPv6或不存在广播形式
                  2、仅UDP协议可使用此接口
************************************************************************/
extern EINT32 OpenSockBroadCast (EFD_T sockv4);

/************************************************************************
  Function:       CloseSockBroadCast
  Description:    关闭IPv4 Socket允许发送广播数据
  Input:          sockv4: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         1、IPv6或不存在广播形式
                  2、仅UDP协议可使用此接口
************************************************************************/
extern EINT32 CloseSockBroadCast (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMulticastTTL
  Description:    设置Socket IPv4数据包中多播地址TTL
  Input:          sockv4:套接字句柄
                  TTL:存活时间
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPMulticastTTL (EINT32 TTL, EFD_T sockv4);

/************************************************************************
  Function:       GetIPMulticastTTL
  Description:    获取Socket IPv4数据包中多播地址TTL
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 GetIPMulticastTTL (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMCInterface
  Description:    设置Socket IPv4多播外出接口(网口)
  Input:          sockv4:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPMCInterface (IfIndex If, EFD_T sockv4);

/************************************************************************
  Function:       GetIPMCInterface
  Description:    获取Socket IPv4多播外出接口(网口)
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
extern IfIndex GetIPMCInterface (EFD_T sockv4);

/************************************************************************
  Function:       SetIPUCInterface
  Description:    设置Socket IPv4单播外出接口(网口)
  Input:          sockv4:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPUCInterface (IfIndex If, EFD_T sockv4);

/************************************************************************
  Function:       GetIPUCInterface
  Description:    设置SocketIPv4单播外出接口(网口)
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
extern IfIndex GetIPUCInterface (EFD_T sockv4);

/************************************************************************
  Function:       SetIPIfLoop
  Description:    设置Socket IPv4是否环回
  Input:          sockv4:套接字句柄
                  IfLoop:是否环回判定值ETTRUE/ETFALSE
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPIfLoop (EUCHAR IfLoop, EFD_T sockv4);

/************************************************************************
  Function:       GetIPIfLoop
  Description:    获取Socket IPv4是否环回
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:IfLoop(ETTRUE/ETFALSE), 失败:ETERROR
  Others:         无
************************************************************************/
extern EUCHAR GetIPIfLoop (EFD_T sockv4);

/************************************************************************
  Function:       SetIPMultiCast
  Description:    设置Socket IPv4多播地址(加入多播组)
  Input:          Addr:多播地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 SetIPMultiCast (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr);

/************************************************************************
  Function:       SetIPMultiCastExt
  Description:    设置Socket IPv4多播地址(加入多播组)
  Input:          Addr:多播地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取SrcAddr
                  SrcAddr: 源IPv4地址
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 SetIPMultiCastExt (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       DropIPMultiCast
  Description:    取消Socket IPv4多播地址(离开多播组)
  Input:          Addr:多播地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 DropIPMultiCast (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr);

/************************************************************************
  Function:       DropIPMultiCastExt
  Description:    取消SocketIPv4多播地址(离开多播组)
  Input:          Addr:多播地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
                  SrcAddr: 源IPv4地址
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 DropIPMultiCastExt (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       BlockIPSource
  Description:    阻塞SocketIP多播源地址
  Input:          Addr:多播源地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
                  SrcAddr: 源IPv4地址
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 BlockIPSource (ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       UnblockIPSource
  Description:    开通阻塞的SocketIP多播源地址
  Input:          Addr:多播源地址
                  sockv4:套接字句柄
                  IfAddr: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
                  SrcAddr: 源IPv4地址
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 UnblockIPSource (ECHAR* Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr);

/************************************************************************
  Function:       GetIpAddrV6
  Description:    将ASCII码形式的IP地址转换成IPv6Addr_T结构体类型
  Input:          s:指向IP地址ASCII码形式的字符串指针
  Output:         无
  Return:         IP地址的IPv6Addr_T结构体类型
  Others:         无
************************************************************************/
extern IPv6Addr_T GetIpAddrV6 (ECHAR * s);

/************************************************************************
  Function:       GetIpAddrV6Str
  Description:    将IPv6Addr_T结构体形式的IPv6地址转换成ASCII码形式的字符串
  Input:          sin6_addr:IPv6地址结构体
  Output:         IpAddrV6Str:指向IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
extern EINT32 GetIpAddrV6Str (IPv6Addr_T sin6_addr, ECHAR * IpAddrV6Str);

/************************************************************************
  Function:       IsIpAddrV6Equal
  Description:    判断两个IPv6ASCII码形式的字符串的地址是否相同
  Input:          IPv6a: 第一个IPv6地址字符串指针
                   IPv6b: 第二个IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
extern EINT32 IsIpAddrV6Equal (ECHAR * IPv6a, ECHAR * IPv6b);

/************************************************************************
  Function:       IsIpAddrV6LoopBack
  Description:    判断一个IPv6ASCII码形式的字符串的地址是否表示环回地址
  Input:          IpAddrV6Str: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不是/异常)  ETTRUE - 判断成功(是)
  Others:         无
************************************************************************/
extern EINT32 IsIpAddrV6LoopBack (ECHAR * IpAddrV6Str);

/************************************************************************
  Function:       OpenRawSocketV6
  Description:    打开一个原始IPv6 Socket
  Input:          protocol:接收的IP数据包类型
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
extern EFD_T OpenRawSocketV6 (EINT32 protocol);

/************************************************************************
  Function:       OpenSocketV6
  Description:    打开一个IPv6 Socket
  Input:          type:WSOCK_STREAM:stream socket，WSOCK_DGRAM:datagram socket，
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
extern EFD_T OpenSocketV6 (EINT32 type);

/************************************************************************
  Function:       SetSocketNonblockV6
  Description:    将IPv6 Socket设置为非阻塞方式
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetSocketNonblockV6 (EFD_T id);

/************************************************************************
Function:       SetSocketLoosRouteV6
Description:    将IPv6 Socket设置为松散路由模式/添加路由地址
Input:          id: Socket句柄
                segments: 网段总数
                addr: 在路由首部中添加的路由地址的指针
Output:         无
Return:         成功:ETOK, 失败:ETERROR
Others:         此接口仅适用于linux/vxworks
************************************************************************/
extern EINT32 SetSocketLoosRouteV6 (EFD_T id, EINT32 segments, IPv6Addr_T * addr);

/************************************************************************
  Function:       SetSocketQosV6
  Description:    设置IPv6 socket的qos类型
  Input:          id: Socket句柄; qos: qos类型;
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口不适用于windows
************************************************************************/
extern EINT32 SetSocketQosV6 (EFD_T id, EUINT32 qos);

/************************************************************************
  Function:       BindSocketIPv6
  Description:    将IPv6 Socket与本地端口绑定
  Input:          id:Socket句柄, port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
#define BindSocketIPv6(sockv6, port) BindSocketIPv6Ext(sockv6, NULL, port, 0, 0, 0)

/************************************************************************
  Function:       BindSrcSocketIPv6
  Description:    将IPv6 Socket与本地端口绑定
  Input:          id: Socket句柄
                  srcIp: ip地址
                  port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
#define BindSrcSocketIPv6(sockv6, ip, port) BindSocketIPv6Ext(sockv6, ip, port, 0, 0, 0)

/************************************************************************
  Function:       BindSocketIPv6Ext
  Description:    将IPv6 Socket与本地端口绑定
  Input:          id: Socket句柄
                  srcIp: ip地址(NULL则绑定任意IP)
                  port: 端口
                  qos: qos类型(占8位 & 255)
                  flowLable: 流标签(占20位 & 1048575)
                  scopeId: 作用域ID(仅用于本地链接，类似于网口加%追加在IPv6地址后，相同ID的socket才能通信)
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         默认设置后三位入参为0(may deprecated)
************************************************************************/
extern EINT32 BindSocketIPv6Ext (EFD_T id, ECHAR * srcIp, EUINT16 port, EUINT32 qos, EUINT32 flowLabel, IfIndex scopeId);

/************************************************************************
  Function:       ListenSocketV6
  Description:    启动IPv6 Socket监听
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ListenSocketV6 (EFD_T id);

/************************************************************************
  Function:       AcceptSocketV6
  Description:    接受IPv6客户端的请求
  Input:          id:Socket句柄;
  Output:         pClientIpAddr: 客户端IP地址
  Return:         成功: 新建的Socket句柄, 失败:ETERROR
  Others:         无
************************************************************************/
extern EFD_T AcceptSocketV6 (EFD_T id, ECHAR * pClientIpAddr);

/************************************************************************
  Function:       ConnectSocketV6
  Description:    IPv6 客户端呼叫服务器
  Input:          id:Socket句柄; remoteIpAddr: 服务器IP地址; port: 端口
  Output:         无
  Return:         成功:ETOK , 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ConnectSocketV6 (EFD_T id, ECHAR * remoteIpAddr, EUINT16 port);

/************************************************************************
  Function:       ReadFrameV6
  Description:    接收IPv6 UDP帧数据
  Input:          id: Socket句柄; len: 缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadFrameV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * pRemoteIpAddr);

/************************************************************************
  Function:       ReadFrameExtV6
  Description:    接收IPv6 UDP帧数据
  Input:          id:Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址; pRemotePort: 数据帧源IP端口
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadFrameExtV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * pRemoteIpAddr, EUINT32 * pRemotePort);

/************************************************************************
  Function:       SendFrameV6
  Description:    发送IPv6 UDP帧数据
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
                  remoteIpAddr: 数据帧目的IP地址; dstPort: 端口
  Output:         无
  Return:         成功: 发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SendFrameV6 (EFD_T id, ECHAR buf[], EUINT32 len, ECHAR * remoteIpAddr, EUINT16 dstPort);

/************************************************************************
  Function:       ReadStreamV6
  Description:    接收IPv6帧数据的缓冲区
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 ReadStreamV6 (EFD_T id, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       SendStreamV6
  Description:    发送IPv6帧数据
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SendStreamV6 (EFD_T id, ECHAR buf[], EINT32 len);

/************************************************************************
  Function:       ReadFromToV6
  Description:    使用recvmsg接收数据包，使得可以获取ipv6头的目标地址
  Input:          s: 接收套接字
                  bufLen: 缓冲区大小
  Output:         pRecvBuf: 存放数据的缓冲区
                  pSrcIp: 数据包的源端IPv6地址(网络字节序)
                  pDstIp: 数据包的目的IPv6地址(网络字节序)
  Return:         成功: 接收数据的长度;失败: ETERROR
  Others:         此接口用于linux\vxworks\unix
************************************************************************/
extern EINT32 ReadFromToV6 (EFD_T s, void * pRecvBuf, EUINT16 bufLen,
    ECHAR * pSrcIp, ECHAR * pDstIp);

/************************************************************************
  Function:       EtCreateRouteV6
  Description:    增加路由
  Input:          pDestIpAddr: 目标IPv6地址
                  pNextHop: 下一条地址
                  pDevName: 设备名称
                  pNetMask: 子网掩码
                  mtu: 链路层最大传输单元
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         此接口用于linux\vxworks
************************************************************************/
#ifdef LINUX
extern EINT32 EtCreateRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pDevName, EUINT32 mtu);
#else  //VXWORKS
extern EINT32 EtCreateRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pNetMask);
#endif

/************************************************************************
  Function:       EtDeleteRouteV6
  Description:    删除路由
  Input:          pDestIpAddr: 目标IPv6地址
                  pNextHop: 下一条地址
                  pDevName: 设备名称
                  pNetMask: 子网掩码
                  mtu: 链路层最大传输单元
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         此接口用于linux\vxworks
************************************************************************/
#ifdef LINUX
extern EINT32 EtDeleteRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pDevName, EUINT32 mtu);
#else  //VXWORKS
extern EINT32 EtDeleteRouteV6 (ECHAR * pDestIpAddr, ECHAR * pNextHop, ECHAR * pNetMask);
#endif

/************************************************************************
  Function:       OpenIPv6Only
  Description:    打开IPv6套接字仅监听IPv6报文
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
extern EINT32 OpenIPv6Only (EFD_T sockId);

/************************************************************************
  Function:       CloseIPv6Only
  Description:    关闭IPv6套接字仅监听IPv6报文功能
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
extern EINT32 CloseIPv6Only (EFD_T sockId);

/************************************************************************
  Function:       SetIPv6UnicastHops
  Description:    设置SocketIPv6数据包中单播地址跳数限制
  Input:          sockv6:套接字句柄
                  hopLimit:单播地址跳数限制
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPv6UnicastHops (EINT32 hopLimit, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6UnicastHops
  Description:    获取SocketIPv6数据包中单播地址跳数限制
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 GetIPv6UnicastHops (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MulticastHops
  Description:    设置SocketIPv6数据包中多播地址跳数限制
  Input:          sockv6:套接字句柄
                  hopLimit:多播地址跳数限制
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPv6MulticastHops (EINT32 hopLimit, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6MulticastHops
  Description:    获取Socket IPv6数据包中多播地址跳数限制
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 GetIPv6MulticastHops (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MCInterface
  Description:    设置Socket IPv6多播外出接口(网口)
  Input:          sockv6:套接字句柄
                  iF:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPv6MCInterface (IfIndex If, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6MCInterface
  Description:    获取Socket IPv6多播外出接口(网口)
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
extern IfIndex GetIPv6MCInterface (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6UCInterface
  Description:    设置Socket IPv6单播外出接口(网口)
  Input:          sockv6:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPv6UCInterface (IfIndex If, EFD_T sockv6);

/************************************************************************
  Function:       GetIPv6UCInterface
  Description:    获取Socket IPv6单播外出接口(网口)
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
extern IfIndex GetIPv6UCInterface (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    设置SocketIPv6是否环回
  Input:          sockv6:套接字句柄
                  IfLoop:是否环回判定值ETTRUE/ETFALSE
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 SetIPv6IfLoop (EUCHAR IfLoop, EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    设置SocketIPv6是否环回
  Input:          sockv6:套接字句柄
                   IfLoop:是否环回判定值
  Output:         无
  Return:         成功:IfLoop(ETTRUE/ETFALSE), 失败:ETERROR
  Others:         无
************************************************************************/
extern EUCHAR GetIPv6IfLoop (EFD_T sockv6);

/************************************************************************
  Function:       SetIPv6MultiCast
  Description:    设置Socket IPv6多播地址(加入多播组)
  Input:          Addr:多播地址
                  sockv6:套接字句柄
                  IfAddr6: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 SetIPv6MultiCast (ECHAR * Addr, EFD_T sockv6, IfIndex IfAddr6);

/************************************************************************
  Function:       DropIPv6MultiCast
  Description:    取消Socket IPv6多播地址(离开多播组)
  Input:          Addr:多播地址
                  sockv6:套接字句柄
                  IfAddr6: 进行组播通讯的出口端口地址，
                           若为0则通过GetIPMCInterface获取
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         套接字类型必须SOCK_DGRAM或SOCK_RAW
************************************************************************/
extern EINT32 DropIPv6MultiCast (ECHAR * Addr, EFD_T sockv6, IfIndex IfAddr6);

/************************************************************************
  Function:       BlockIPv6Source
  Description:    阻塞SocketIP多播源地址
  Input:          Addr:多播源地址
                  sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口仅用于已经绑定bind的socket
                  源source信息由绑定信息获取
************************************************************************/
#define BlockIPv6Source(Addr, sock) BlockIPSourceExt(Addr, sock, 0) /* grp_port = 0 to Ignore */

/************************************************************************
  Function:       BlockIPv6SourceExt
  Description:    阻塞SocketIP多播源地址
  Input:          Addr:多播源地址
                  sock:套接字句柄
                  grp_port: 多播源端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口仅用于已经绑定bind的socket
                  源source信息由绑定信息获取
************************************************************************/
extern EINT32 BlockIPv6SourceExt (ECHAR * Addr, EFD_T sock, EUINT16 grp_port);

/************************************************************************
  Function:       UnblockIPv6Source
  Description:    开通阻塞的SocketIP多播源地址
  Input:          Addr:多播源地址
                  sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口仅用于已经绑定bind的socket
                  源source信息由绑定信息获取
************************************************************************/
#define UnblockIPv6Source(Addr, sock) UnblockIPSourceExt(Addr, sock, 0)  /* grp_port = 0 to Ignore */

/************************************************************************
  Function:       UnblockIPv6SourceExt
  Description:    开通阻塞的SocketIP多播源地址
  Input:          Addr:多播源地址
                  sock:套接字句柄
                  grp_port: 多播源端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口仅用于已经绑定bind的socket
                  源source信息由绑定信息获取
************************************************************************/
extern EINT32 UnblockIPv6SourceExt (ECHAR * Addr, EFD_T sock, EUINT16 grp_port);

/************************************************************************
  Function:       SetIPv6PMTUD
  Description:    设置IPv6路径MTU发现功能开关
  Input:          sock:套接字句柄
                  val: 为 1 表示: PMTUD功能不必执行，为避免分片分组就使用最小MTU发送
                        为 0 表明: PMTUD功能对于所有目的地都得执行
                        为 -1表示: PMTUD功能仅对单播目的地执行，对于多播目的地就使用最小MTU
                        (此选项默认为-1(存疑))
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口在不同系统下的(宏)可行性存疑
************************************************************************/
extern EINT32 SetIPv6PMTUD (EFD_T sock, EINT32 val);

/************************************************************************
  Function:       GetIPv6PMTU
  Description:    获取IPv6的当前路径Path_MTU
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:PMTU值, 失败:ETERROR
  Others:         无
************************************************************************/
extern EUINT32 GetIPv6PMTU (EFD_T sock);

/*----------------------------multicast--------------------------------*/
/************************************************************************
  Function:       OpenMultiSock
  Description:    设置socket的组播发送接口地址及发送组播包的特性，禁用组播回环
  Input:          sId: 需要发送组播的socket句柄；ipAddr:默认发送组播包接口地址
  Output:         无
  Return:         成功ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 OpenMultiSock(EUINT32 sId, EUINT32 ipAddr);

/************************************************************************
  Function:       OpenMultiSockSlefLoop
  Description:    设置socket的组播发送接口地址及发送组播包的特性,组播自回环
  Input:          sId: 需要发送组播的socket句柄；ipAddr:默认发送组播包接口地址
  Output:         无
  Return:         成功ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 OpenMultiSockSlefLoop(EUINT32 sId, EUINT32 ipAddr);

/************************************************************************
  Function:       JoinMultiGroup
  Description:    将socket加入指定的组播
  Input:          recvIpAddr: 进行组播通讯端口的IP地址
                  multiIpAddr:组播地址；
                  sId:需要加入组播的socket
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 JoinMultiGroup(EUINT32 recvIpAddr, EUINT32 multiIpAddr, EUINT32 sId);

/************************************************************************
  Function:       LeaveMultiGroup
  Description:    将socket脱离指定的组播
  Input:          recvIpAddr: 进行组播通讯端口的IP地址
                  multiIpAddr:组播地址; 
                  sId:需要脱离组播的socket
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
extern EINT32 LeaveMultiGroup(EUINT32 recvIpAddr, EUINT32 multiIpAddr, EUINT32 sId);

#ifdef  __cplusplus
}
#endif
#endif //__BEARPUB_H__
