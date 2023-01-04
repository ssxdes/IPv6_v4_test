/************************************************************************
Copyright (C), 2022-2023,
File name:      etSockV6Lib.c
Author:         
Date:          
Description:    对不同操作系统的socket IPv6接口进行封装，对上层提供统一socket接口
Others:         无
*************************************************************************/

/************************************************************************
                            引用的头文件
*************************************************************************/
#include "etSockV6Lib.h"
#include "bearpub.h"
#include "sys.h"
#include <errno.h>

#ifdef WINDOWS
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif //WINDOWS

#ifdef LINUX
#include <linux/route.h>
//<net/route.h> & <linux/route.h> redefinition
//<linux/in6.h> & <netinet/in.h> redefinition
#endif

/************************************************************************
                             外部函数声明
*************************************************************************/

#ifdef LINUX
extern socklen_t inet6_rth_space(int __type, int __segments);
extern void* inet6_rth_init(void* __bp, socklen_t __bp_len, int __type, int __segments);
extern int inet6_rth_add(void* __bp, const struct in6_addr* __addr);
#endif

#ifdef VXWORKS
extern STATUS ipRouteCreate
(
    struct sockaddr* pDstAddr, 	/* destination network/host address */
    struct sockaddr* pNetmask, 	/* network mask, NULL for host routes*/
    struct sockaddr* pGateway, 	/* gateway address */
    ULONG 		routeIndex,	        /* optional index value for interface*/
    ULONG 		routeTos, 	        /* route type-of-service value */
    ULONG 		routeOwner, 	    /* routing protocol information */
    ULONG 		routeSource, 	    /* API classifier, for data format */
    ULONG 		routeFormat,	    /* identifier for routing table */
    UCHAR 		routeWeight, 	    /* Relative priority, for policy */
    ULONG		routeFlags,	        /* route flags */
    void* pRouteData,	        /* Additional data, based on format */
    void* pCustomData, 	    /* and source parameters */
    struct rtentry** ppRoute		/* Where to return route */
);
#endif

/************************************************************************
                             全局变量定义
*************************************************************************/

/************************************************************************
                               函数定义
*************************************************************************/
/************************************************************************
  Function:       GetIpAddrV6
  Description:    将ASCII码形式的IP地址转换成IPv6Addr_T结构体类型
  Input:          s:指向IP地址ASCII码形式的字符串指针
  Output:         无
  Return:         IP地址的IPv6Addr_T结构体类型
  Others:         无
************************************************************************/
IPv6Addr_T GetIpAddrV6(ECHAR* s)
{
    struct sockaddr_in6 sockAddrV6;
    memset(&sockAddrV6, 0, sizeof(sockAddrV6));
    if (s != NULL)
        inet_pton(AF_INET6, s, &sockAddrV6.sin6_addr);
    return sockAddrV6.sin6_addr;
}

/************************************************************************
  Function:       GetIpAddrV6Str
  Description:    将IPv6Addr_T结构体形式的IPv6地址转换成ASCII码形式的字符串
  Input:          sin6_addr:IPv6地址结构体
  Output:         IpAddrV6Str:指向IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
EINT32 GetIpAddrV6Str(IPv6Addr_T sin6_addr, ECHAR* IpAddrV6Str)
{
    ECHAR s[MAX_IPV6_ADDRSTR_LEN];
    if (inet_ntop(AF_INET6, (void*)&sin6_addr, (void*)s, (size_t)MAX_IPV6_ADDRSTR_LEN) != NULL)
        if (memcpy(IpAddrV6Str, s, (size_t)(sizeof(s) > MAX_IPV6_ADDRSTR_LEN ? MAX_IPV6_ADDRSTR_LEN : sizeof(s))) != NULL)
            return ETOK;
    return ETERROR;
}

/************************************************************************
  Function:       IsIpAddrV6Equal
  Description:    判断两个IPv6ASCII码形式的字符串的地址是否相同
  Input:          IPv6a: 第一个IPv6地址字符串指针
                  IPv6b: 第二个IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
EINT32 IsIpAddrV6Equal(ECHAR* IPv6a, ECHAR* IPv6b)
{
    IPv6Addr_T ipv6a = GetIpAddrV6(IPv6a);
    IPv6Addr_T ipv6b = GetIpAddrV6(IPv6b);
#ifdef WINDOWS
    return IN6_ADDR_EQUAL(&ipv6a, &ipv6b) == 0 ? ETFALSE : ETTRUE;
#elif defined(LINUX) || defined(VXWORKS) || defined(AIX)
    return IN6_ARE_ADDR_EQUAL(&ipv6a, &ipv6b) == 0 ? ETFALSE : ETTRUE;
#else
    return ETFALSE;
#endif
}

/************************************************************************
  Function:       IsIpAddrV6LoopBack
  Description:    判断一个IPv6ASCII码形式的字符串的地址是否表示环回地址
  Input:          IpAddrV6Str: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不是/异常)  ETTRUE - 判断成功(是)
  Others:         无
************************************************************************/
EINT32 IsIpAddrV6LoopBack(ECHAR* IpAddrV6Str)
{
    IPv6Addr_T ipv6 = GetIpAddrV6(IpAddrV6Str);
    return IN6_IS_ADDR_LOOPBACK(&ipv6) == 0 ? ETFALSE : ETTRUE;
}

/************************************************************************
  Function:       OpenRawSocketV6
  Description:    打开一个原始IPv6 Socket
  Input:          protocol:接收的IP数据包类型
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
EFD_T OpenRawSocketV6(EINT32 protocol)
{
    EFD_T sockv6;
    EINT32  optval;
    static int errNum = 0;

    optval = 1;

    sockv6 = socket(AF_INET6, ESOCK_RAW, protocol);

    if (sockv6 <= 0)
    {
#if defined(LINUX)        
        printf("[error]: OpenRawSocketV6: socket error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif

        if (errno == EMFILE)
        {
            errNum++;
            if (errNum >= 3)
            {
                printf("errNum >= 3! [%s,%d]\r\n", __FILE__, __LINE__);
            }
        }
        return ETERROR;
    }

    if (setsockopt(sockv6, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) < 0)  //将SOKCET地址置为可重用
    {
#if defined(LINUX)        
        printf("[error]: OpenRawSocketV6: setsockopt error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return sockv6;
    }
}

/************************************************************************
  Function:       OpenSocketV6
  Description:    打开一个IPv6 Socket
  Input:          type:WSOCK_STREAM:stream socket，WSOCK_DGRAM:datagram socket，
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
EFD_T OpenSocketV6(EINT32 type)
{
    EFD_T  sockv6;
    static int errNum = 0;

    switch (type)
    {
    case ESOCK_DGRAM:
    case ESOCK_STREAM:
        sockv6 = socket(AF_INET6, type, 0);
        break;
    case ESOCK_RAW:
        sockv6 = socket(AF_INET6, type, IPPROTO_IPV6);
        break;
    default:
        return ETERROR;
    }

    if (sockv6 < 0)
    {
#if defined(LINUX)        
        printf("[error]: OpenSocketV6: socket error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif

        if (errno == EMFILE)
        {
            errNum++;
            if (errNum >= 3)
            {
                printf("errNum >= 3! [%s,%d]\r\n", __FILE__, __LINE__);
            }
        }

        return ETERROR;
    }
#if defined (LINUX) || defined(AIX)
    if (fcntl(sockv6, F_SETFD, FD_CLOEXEC) < 0)
    {
        printf("OpenSocketV6 fcntl err %d\n", errno);
        /*PrintfLog(LOG_ERROR, FID_BEAR,
                  "[error]: OpenSocketV6: fcntl error, errno = %d -- %s \r\n",errno,strerror(errno));*/
    }
#endif
#if defined (WINDOWS)
    SetHandleInformation((HANDLE)sockv6, HANDLE_FLAG_INHERIT, 0);
#endif
    return sockv6;

}

/************************************************************************
  Function:       SetSocketNonblockV6
  Description:    将IPv6 Socket设置为非阻塞方式
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetSocketNonblockV6(EFD_T id)
{
#ifdef WINDOWS
    EULONG flag = 1;            //0:阻塞方式, 1:非阻塞方式

    if (ioctlsocket(id, FIONBIO, &flag) < 0)
    {
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif

#ifdef VXWORKS
    EINT32 flag = 1;

    if (ioctl(id, FIONBIO, (int)&flag) < 0)
    {
        //printf("[error]: SetSocketNonblockV6: ioctl error, errno = %d -- %s \r\n",errno,strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif

#if defined(LINUX) || defined(AIX)
    EUINT32 flag;

    flag = fcntl(id, F_GETFL, 0);
    if (fcntl(id, F_SETFL, flag | O_NONBLOCK) < 0)
    {
        printf("[error]: SetSocketNonblockV6: fcntl error, errno = %d -- %s \r\n", errno, strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif
}

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
EINT32 SetSocketLoosRouteV6(EFD_T id, EINT32 segments, IPv6Addr_T* addr)
{
    if (addr == NULL)
        return ETERROR;

#ifdef WINDOWS
    return ETERROR;
#elif defined(LINUX) || defined(VXWORKS)

    socklen_t len = inet6_rth_space(IPV6_RTHDR_LOOSE, segments);
    EUCHAR rth_ptr[len];
    memset(rth_ptr, 0, len);
    EINT32 result = ETERROR;
    if (rth_ptr != NULL && len > 0)
    {
        inet6_rth_init(rth_ptr, len, IPV6_RTHDR_LOOSE, segments);
        if (addr != NULL)
            inet6_rth_add(rth_ptr, addr);
        if (setsockopt(id, IPPROTO_IPV6, IPV6_RTHDR, rth_ptr, len) == 0)
            result = ETOK;
    }
    return result;

#elif 0  //defined(VXWORKS)

    //IP_OPTION opthdr;
    //opthdr.option_type = IP_LOOSE_SOURCE_AND_RECORD_ROUTE_TYPE;
    //opthdr.option_data_length = 4*num + 8;
    //memcpy(opthdr.option_data, addr, num*4);
    //return setsockopt(id, IPPROTO_IPV6, IP_OPTIONS, (char *)&opthdr, opthdr.option_data_length);

#else //defined(AIX)
    return ETERROR;
#endif
}

/************************************************************************
  Function:       SetSocketQosV6
  Description:    设置IPv6 socket的qos类型
  Input:          id: Socket句柄; qos: qos类型;
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口不适用于windows
************************************************************************/
EINT32 SetSocketQosV6(EFD_T id, EUINT32 qos)
{
    EUINT32 ip6Tclass = (qos << 2);
#ifndef WINDOWS
    if (setsockopt(id, IPPROTO_IPV6, IPV6_TCLASS, (char*)&ip6Tclass, sizeof(ip6Tclass)) < 0)
    {
#if defined(LINUX)        
        printf("[error]: SetSocketQosV6: setsockopt error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#else  //WINDOWS
    return ETERROR;
#endif
}

/************************************************************************
  Function:       BindSocketIPv6
  Description:    将IPv6 Socket与本地端口绑定
  Input:          id:Socket句柄, port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
//#define BindSocketIPv6(sockv6, port) BindSocketIPv6Ext(sockv6, NULL, port, 0, 0, 0)

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
//#define BindSrcSocketIPv6(sockv6, ip, port) BindSocketIPv6Ext(sockv6, ip, port, 0, 0, 0)

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
EINT32 BindSocketIPv6Ext(EFD_T id, ECHAR* srcIp, EUINT16 port, EUINT32 qos, EUINT32 flowLabel, IfIndex scopeId)
{
    struct sockaddr_in6 serverAddrV6;
    EUINT32 sockAddrV6Size;
    EUINT32 ip6Tos = (qos << 2);

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    memset(&serverAddrV6, 0, sizeof(serverAddrV6));
    serverAddrV6.sin6_family = AF_INET6;
    serverAddrV6.sin6_port = (EUINT16)etHtons(port);
    serverAddrV6.sin6_flowinfo = (EUINT32)etHtonl(((((EUINT32)IPPROTO_IPV6_HDRVAL & 0xf) << 28) + ((ip6Tos & 0xff) << 20) + (flowLabel & 0xfffff)) & 0xffffffff);
    //                                               4bits  version             8bits  priority          20bits  flowlable
    serverAddrV6.sin6_scope_id = (IfIndex)etHtonl(scopeId);
    if (srcIp != NULL)
        inet_pton(AF_INET6, srcIp, &serverAddrV6.sin6_addr);
    else
        serverAddrV6.sin6_addr = in6addr_any;

#ifdef VXWORKS
    serverAddrV6.sin6_len = (EUCHAR)sockAddrV6Size;
#endif

    if (bind(id, (struct sockaddr*)&serverAddrV6, sockAddrV6Size) != 0)
    {
#if defined(LINUX)        
        printf("[error]: BindSrcIpSocketV6: bind error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
}

/************************************************************************
  Function:       ListenSocketV6
  Description:    启动IPv6 Socket监听
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ListenSocketV6(EFD_T id)
{
    if (listen(id, SERVERV6_MAX_CONNECTIONS) < 0)
    {
        printf("[error]: ListenSocketV6: listen error, errno = %d -- %s \r\n", errno, strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
}

/************************************************************************
  Function:       AcceptSocketV6
  Description:    接受IPv6客户端的请求
  Input:          id:Socket句柄;
  Output:         pClientIpAddr: 客户端IP地址
  Return:         成功:新建的Socket句柄 , 失败:ETERROR
  Others:         无
************************************************************************/
EFD_T AcceptSocketV6(EFD_T id, ECHAR* pClientIpAddr)
{
    struct sockaddr_in6 clientAddrV6;
    EINT32 sockAddrV6Size;
    EFD_T fd;

    sockAddrV6Size = sizeof(clientAddrV6);
    memset(&clientAddrV6, 0, sockAddrV6Size);

    fd = accept(id, (struct sockaddr*)&clientAddrV6, (socklen_t*)&sockAddrV6Size);
    if (fd < 0)
    {
#if defined(LINUX)        
        printf("[error]: AcceptSocketV6: accept error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
#if defined (LINUX) || defined(AIX)
        if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
        {
            printf("AcceptSocketV6 fcntl err %d\n", errno);
            /*PrintfLog(LOG_ERROR, FID_BEAR,
                      "[error]: AcceptSocketV6: fcntl error, errno = %d -- %s \r\n",errno,strerror(errno));*/
        }
#endif
#if defined (WINDOWS)
        SetHandleInformation((HANDLE)fd, HANDLE_FLAG_INHERIT, 0);
#endif
        if (GetIpAddrV6Str(clientAddrV6.sin6_addr, pClientIpAddr) == ETERROR)
            return ETERROR;
        return fd;
    }
}

/************************************************************************
  Function:       ConnectSocketV6
  Description:    IPv6 客户端呼叫服务器
  Input:          id:Socket句柄; remoteIpAddr: 服务器IP地址; port: 端口
  Output:         无
  Return:         成功:ETOK , 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ConnectSocketV6(EFD_T id, ECHAR* remoteIpAddr, EUINT16 port)
{
    if (remoteIpAddr == NULL)
        return ETERROR;

#ifdef WINDOWS
    struct sockaddr_in6 serverAddrV6;
    EUINT32 sockAddrV6Size;
    struct timeval WaitTime;
    fd_set FdSet;
    EULONG flag = 1;

    if (ioctlsocket(id, FIONBIO, &flag) < 0)                       //set nublock mode
    {
        return ETERROR;
    }

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    memset(&serverAddrV6, 0, sizeof(serverAddrV6));
    serverAddrV6.sin6_family = AF_INET6;
    serverAddrV6.sin6_port = (EUINT16)etHtons(port);
    inet_pton(AF_INET6, remoteIpAddr, &serverAddrV6.sin6_addr);

    connect((SOCKET)id, (struct sockaddr*)&serverAddrV6, sockAddrV6Size);

    FD_ZERO(&FdSet);
    FD_SET(id, &FdSet);
    WaitTime.tv_sec = 3;
    WaitTime.tv_usec = 0;
    if (select(FD_SETSIZE, 0, &FdSet, 0, &WaitTime) <= 0)                    //wait 3 sec
    {
        flag = 0;
        ioctlsocket(id, FIONBIO, &flag);                        //resume block mode
        return ETERROR;
    }
    else
    {
        flag = 0;
        ioctlsocket(id, FIONBIO, &flag);
        return ETOK;
    }
#endif

#if defined(LINUX) || defined(AIX)
    struct sockaddr_in6 serverAddrV6;
    EUINT32 sockAddrV6Size;
    struct timeval WaitTime;
    fd_set FdSet;
    EUINT32 flag;

    flag = fcntl(id, F_GETFL, 0);
    if (fcntl(id, F_SETFL, flag | O_NONBLOCK) < 0)                       //set nublock mode
    {
        printf("[error]:socket:0x%x ConnectSocketV6: fcntl error, remoteIp:%s, errno = %d -- %s \r\n", id, remoteIpAddr, errno, strerror(errno));
        return ETERROR;
    }

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    memset(&serverAddrV6, 0, sizeof(serverAddrV6));
    serverAddrV6.sin6_family = AF_INET6;
    serverAddrV6.sin6_port = (EUINT16)etHtons(port);
    inet_pton(AF_INET6, remoteIpAddr, &serverAddrV6.sin6_addr);

    connect(id, (struct sockaddr*)&serverAddrV6, sockAddrV6Size);

    FD_ZERO(&FdSet);
    FD_SET(id, &FdSet); WaitTime.tv_sec = 3;
    WaitTime.tv_usec = 0;
    if (select(FD_SETSIZE, 0, &FdSet, 0, &WaitTime) <= 0)                    //wait 3 sec
    {
        flag = fcntl(id, F_GETFL, 0);
        fcntl(id, F_SETFL, flag & ~O_NONBLOCK);                        //resume block mode return ETERROR;
    }
    else
    {
        flag = fcntl(id, F_GETFL, 0);
        fcntl(id, F_SETFL, flag & ~O_NONBLOCK);
        return ETOK;
    }
    return ETERROR;
#endif //LINUX

#ifdef VXWORKS
    struct sockaddr_in6 serverAddrV6;
    EUINT32 sockAddrV6Size;
    struct timeval WaitTime;

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    memset(&serverAddrV6, 0, sizeof(serverAddrV6));
    serverAddrV6.sin6_family = AF_INET6;
    serverAddrV6.sin6_port = (EUINT16)etHtons(port);
    inet_pton(AF_INET6, remoteIpAddr, &serverAddrV6.sin6_addr);
    serverAddrV6.sin6_len = (EUCHAR)sockAddrV6Size;

    WaitTime.tv_sec = 3;                //wait 3 sec
    WaitTime.tv_usec = 0;

    if (connectWithTimeout(id, (struct sockaddr*)&serverAddrV6, sockAddrV6Size, &WaitTime) < 0)
    {

        //printf("[error]: ConnectSocket: connectWithTimeout error, errno = %d -- %s \r\n",errno,strerror(errno));

        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif
}

/************************************************************************
  Function:       ReadFrameV6
  Description:    接收IPv6 UDP帧数据
  Input:          id: Socket句柄; len: 缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadFrameV6(EFD_T id, ECHAR buf[], EUINT32 len, ECHAR* pRemoteIpAddr)
{
    struct sockaddr_in6 remoteAddrV6;
    EUINT32 sockAddrV6Size;
    EINT32 recvLen;

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    recvLen = recvfrom(id, buf, len, 0, (struct sockaddr*)&remoteAddrV6, (socklen_t*)&sockAddrV6Size);
    if (recvLen <= 0)
    {
#if defined(LINUX)        
        printf("[error]:socket:0x%x ReadFrameV6: recvfrom error, errno = %d -- %s \r\n", id, errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        ECHAR addrstr[MAX_IPV6_ADDRSTR_LEN];
        if (GetIpAddrV6Str(remoteAddrV6.sin6_addr, addrstr) == ETERROR)
            return ETERROR;
        return recvLen;
    }
}

/************************************************************************
  Function:       ReadFrameExtV6
  Description:    接收IPv6 UDP帧数据
  Input:          id:Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址; pRemotePort: 数据帧源IP端口
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadFrameExtV6(EFD_T id, ECHAR buf[], EUINT32 len, ECHAR* pRemoteIpAddr, EUINT32* pRemotePort)
{
    struct sockaddr_in6 remoteAddrV6;
    EUINT32 sockAddrV6Size;
    EINT32 recvLen;

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    recvLen = recvfrom(id, buf, len, 0, (struct sockaddr*)&remoteAddrV6, (socklen_t*)&sockAddrV6Size);
    if (recvLen <= 0)
    {
#if defined(LINUX)        
        printf("[error]:socket:0x%x ReadFrameExtV6: recvfrom error, errno = %d -- %s \r\n", id, errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        if (GetIpAddrV6Str(remoteAddrV6.sin6_addr, pRemoteIpAddr) == ETERROR)
            return ETERROR;
        *pRemotePort = (EUINT16)etNtohs(remoteAddrV6.sin6_port);
        return recvLen;
    }
}

/************************************************************************
  Function:       SendFrameV6
  Description:    发送IPv6 UDP帧数据
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
                  remoteIpAddr: 数据帧目的IP地址; dstPort: 端口
  Output:         无
  Return:         成功: 发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SendFrameV6(EFD_T id, ECHAR buf[], EUINT32 len, ECHAR* remoteIpAddr, EUINT16 dstPort)
{
    struct sockaddr_in6 remoteAddrV6;
    EUINT32 sockAddrV6Size;
    EINT32 sendlen;

    if (0 == remoteIpAddr
        || 0 == len)
    {
        return ETERROR;
    }

    sockAddrV6Size = sizeof(struct sockaddr_in6);
    memset(&remoteAddrV6, 0, sizeof(remoteAddrV6));
    remoteAddrV6.sin6_family = AF_INET6;
    remoteAddrV6.sin6_port = (EUINT16)etHtons(dstPort);
    inet_pton(AF_INET6, remoteIpAddr, &remoteAddrV6.sin6_addr);
#ifdef VXWORKS
    remoteAddrV6.sin6_len = (EUCHAR)sockAddrV6Size;
#endif

    sendlen = sendto(id, buf, len, 0, (struct sockaddr*)&remoteAddrV6, sockAddrV6Size);
    if (sendlen <= 0)
    {
#if defined(LINUX)        
        printf("[error]:socket:0x%x SendFrameV6: sendto error,remoteIP:%s, errno = %d -- %s \r\n", id, remoteIpAddr, errno, strerror(errno));
#endif
        return ETERROR;
    }
    else
    {
        return sendlen;
    }
}

/************************************************************************
  Function:       ReadStreamV6
  Description:    接收IPv6帧数据的缓冲区
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadStreamV6(EFD_T id, ECHAR buf[], EINT32 len)
{
    EINT32 recvlen;
    recvlen = recv(id, buf, len, MSG_NOSIGNAL);
    if (recvlen <= 0)
    {
#if defined(LINUX)    
        printf("[error]:socket:0x%x ReadStreamV6: recv error, errno = %d -- %s \r\n", id, errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return recvlen;
    }
}

/************************************************************************
  Function:       SendStreamV6
  Description:    发送IPv6帧数据
  Input:          id: Socket句柄; buf[]: 缓存; len: 缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SendStreamV6(EFD_T id, ECHAR buf[], EINT32 len)
{
    EINT32 sendlen;
    sendlen = send(id, buf, len, MSG_NOSIGNAL);
    if (sendlen <= 0)
    {
#if defined(LINUX)    
        printf("[error]:socket:0x%x SendStreamV6: send error, errno = %d -- %s \r\n", id, errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return sendlen;
    }
}

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
EINT32 ReadFromToV6(EFD_T s, void* pRecvBuf, EUINT16 bufLen,
    ECHAR* pSrcIp, ECHAR* pDstIp)
{
    EINT32 recvLen = -1;
    EUINT32 Addr = 0;
    ECHAR RecvBuf[1024];
    EUINT32 RecvBufLen = 1024;
#if defined(LINUX) || defined(AIX) || defined(VXWORKS)
    struct msghdr msg;
    struct cmsghdr* pCmsg;
    struct iovec iov;
    struct sockaddr_in6 srcAddrV6 = { 0 };
    ECHAR control[64] = { '\0' };
    ECHAR strIPV6[MAX_IPV6_ADDRSTR_LEN] = { '\0' };
    if (NULL == pRecvBuf
        || 0 == bufLen
        || NULL == pSrcIp
        || NULL == pDstIp)
    {
        printf("%s(%d)--:"
            "In ReadFromToV6: the parameter passed is unavailable!\r\n",
            __FILE__, __LINE__);
        msg.msg_name = &Addr;
        msg.msg_namelen = sizeof(struct sockaddr_in6);
        iov.iov_base = (ECHAR*)RecvBuf;
        iov.iov_len = RecvBufLen;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = (void*)control;
        msg.msg_controllen = sizeof(control);
        msg.msg_flags = 0;
        recvLen = recvmsg(s, &msg, 0);
        return ETERROR;
    }// restart error might be here.

    msg.msg_name = &srcAddrV6;
    msg.msg_namelen = sizeof(struct sockaddr_in6);
    iov.iov_base = (ECHAR*)pRecvBuf;
    iov.iov_len = bufLen;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = (void*)control;
    msg.msg_controllen = sizeof(control);
    msg.msg_flags = 0;

    recvLen = recvmsg(s, &msg, 0);
    if (recvLen < 0)
    {
        printf("%s(%d)--:"
            "In ReadFromToV6: recvmsg return -1, errno = %d -- %s \r\n",
            __FILE__, __LINE__, errno, strerror(errno));
        return ETERROR;
    }

    GetIpAddrV6Str(srcAddrV6.sin6_addr, strIPV6);
    memcpy(pSrcIp, strIPV6, (size_t)MAX_IPV6_ADDRSTR_LEN);
    memset(strIPV6, '\0', (size_t)MAX_IPV6_ADDRSTR_LEN);

    for (pCmsg = CMSG_FIRSTHDR(&msg); NULL != pCmsg; pCmsg = CMSG_NXTHDR(&msg, pCmsg))
    {
#if defined(IPV6_RECVDSTADDR)
        if (pCmsg->cmsg_level == IPPROTO_IPV6
            && pCmsg->cmsg_type == IPV6_RECVDSTADDR)
        {
            GetIpAddrV6Str(*((struct in6_addr*)((EUINT32)CMSG_DATA(pCmsg))), strIPV6);
            memcpy(pDstIp, strIPV6, (size_t)MAX_IPV6_ADDRSTR_LEN);
            //*pDstIp = ((struct in_addr*)((EUINT32)CMSG_DATA(pCmsg)))->s_addr;
            break;
        }
#elif defined(IPV6_PKTINFO)
        if (pCmsg->cmsg_level == IPPROTO_IPV6
            && pCmsg->cmsg_type == IPV6_PKTINFO)
        {
            GetIpAddrV6Str(((struct inv6_pktinfo*)CMSG_DATA(pCmsg))->ipi6_addr, strIPV6);
            memcpy(pDstIp, strIPV6, (size_t)MAX_IPV6_ADDRSTR_LEN);
            //*pDstIp = ((struct in_pktinfo *)CMSG_DATA(pCmsg))->ipi_addr.s_addr;
            break;
        }
#endif
    }
#endif
#if defined(WINDOWS)
    (void)pDstIp;
    (void)pSrcIp;
    (void)bufLen;
    (void)pRecvBuf;
    (void)RecvBuf;
    (void)s;
#endif

    return recvLen;
}

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
EINT32 EtCreateRouteV6(ECHAR* pDestIpAddr, ECHAR* pNextHop, ECHAR* pDevName, EUINT32 mtu)
#else  //VXWORKS
EINT32 EtCreateRouteV6(ECHAR* pDestIpAddr, ECHAR* pNextHop, ECHAR* pNetMask)
#endif
{
    EINT32 result = ETERROR;
#ifdef LINUX

    struct in6_rtmsg rt;          /* ipv6 route struct */
    struct ifreq ifr;             /* interface request struct */
    EINT32 metric = 1;            /* +1 for binary compatibility!  */
    EINT32 prefix_len = 128;      /*to set RTF_HOST*/
    EINT32 skfd = -1;

    /* Clean out the RT structure. */
    memset(&rt, 0, sizeof(rt));
    memset(&ifr, 0, sizeof(ifr));

    /* Fill in the other fields. */
    if (pNextHop)
        inet_pton(AF_INET6, pNextHop, &rt.rtmsg_gateway);
    else
        return ETERROR;

    if (pDestIpAddr)
        inet_pton(AF_INET6, pDestIpAddr, &rt.rtmsg_dst);
    else
        rt.rtmsg_dst = in6addr_any;

    rt.rtmsg_metric = metric;
    rt.rtmsg_dst_len = prefix_len;
    rt.rtmsg_flags = (RTF_UP | RTF_GATEWAY);

    /* Create a socket to the INET6 kernel. */
    if ((skfd = OpenSocketV6(ESOCK_DGRAM)) == ETERROR)
    {
        printf("[error]: EtCreateRouteV6: OpenSocketV6 error!\n");
        return ETERROR;
    }

    if (pDevName)
    {/* device setting */
        strcpy(ifr.ifr_name, pDevName);

        if (ioctl(skfd, SIOGIFINDEX, &ifr) < 0)
        {
            printf("[error]: EtCreateRouteV6: device setting error!\n");
            return ETERROR;
        }
        rt.rtmsg_ifindex = ifr.ifr_ifindex;
    }

    if (mtu)
    {/* mtu setting */
        ifr.ifr_mtu = mtu;

        if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0)
        {
            printf("[error]: EtCreateRouteV6: mtu setting error!\n");
            return ETERROR;
        }
    }

    /* Tell the kernel to accept this route. */
    if (ioctl(skfd, SIOCADDRT, &rt) < 0)
    {
        printf("[error]: EtCreateRouteV6: ioctl RTF_GATEWAY error, errno = %d -- %s\n", errno, strerror(errno));
    }
    else
    {
        result = ETOK;
    }

    rt.rtmsg_flags = (RTF_UP | RTF_HOST);
    /* Tell the kernel to accept this route. */
    if (ioctl(skfd, SIOCADDRT, &rt) < 0)
    {
        printf("[error]: EtCreateRouteV6: ioctl RTF_HOST error, errno = %d -- %s\n", errno, strerror(errno));
    }
    else
    {
        result = ETOK;
    }
    /* Close the socket. */
    CloseSocket(skfd);

#elif defined(VXWORKS)
    //mRouteAdd(pDestIpAddr, pNextHop, intIpMask, 0, 0) for IPv4
    struct sockaddr_in6 dst, mask, gate;

    if (pDestIpAddr == NULL || pNextHop == NULL)
        return ETERROR;
    memset(&dst, 0, sizeof(dst));
    memset(&mask, 0, sizeof(mask));
    memset(&gate, 0, sizeof(gate));

    dst.sin6_family = AF_INET6;
    inet_pton(AF_INET6, pDestIpAddr, &dst.sin6_addr);
    gate.sin6_family = AF_INET6;
    inet_pton(AF_INET6, pNextHop, &gate.sin6_addr);

    if (pNetMask != NULL)
    {   // set RTF_GATEWAY
        mask.sin6_family = AF_INET6;
        inet_pton(AF_INET6, pNetMask, &mask.sin6_addr);
        if (0 > ipRouteCreate((struct sockaddr*)&dst, (struct sockaddr*)&mask, (struct sockaddr*)&gate,
            0, 0, M2_ipRouteProto_other, ROUTE_KERNEL,
            ROUTE_USAGE_IPV6, 0, RTF_UP, NULL, NULL, NULL))
        {
            printf("[error]: EtCreateRouteV6: ipRouteCreate with mask error!\r\n");
        }
    }

    // for a host route require a 0 mask (RTF_HOST)
    if (0 > ipRouteCreate((struct sockaddr*)&dst, NULL, (struct sockaddr*)&gate,
        0, 0, M2_ipRouteProto_other, ROUTE_KERNEL,
        ROUTE_USAGE_IPV6, 0, RTF_UP, NULL, NULL, NULL))
    {
        printf("[error]: EtCreateRouteV6: ipRouteCreate without mask error!\r\n");
    }
    else
    {
        result = ETOK;
    }
#else  //WINDOWS & AIX
    (void)pNextHop;
    (void)pDestIpAddr;
    (void)pNetMask;
#endif

    return result;
}

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
EINT32 EtDeleteRouteV6(ECHAR* pDestIpAddr, ECHAR* pNextHop, ECHAR* pDevName, EUINT32 mtu)
#else  //VXWORKS
EINT32 EtDeleteRouteV6(ECHAR* pDestIpAddr, ECHAR* pNextHop, ECHAR* pNetMask)
#endif
{
    EINT32 result = ETERROR;

#ifdef LINUX
    struct in6_rtmsg rt;          /* ipv6 route struct */
    struct ifreq ifr;             /* interface request struct */
    EINT32 metric = 1;            /* +1 for binary compatibility!  */
    EINT32 prefix_len = 128;      /*to set RTF_HOST*/
    EINT32 skfd = -1;

    /* Clean out the RT structure. */
    memset(&rt, 0, sizeof(rt));
    memset(&ifr, 0, sizeof(ifr));

    /* Fill in the other fields. */
    if (pNextHop)
        inet_pton(AF_INET6, pNextHop, &rt.rtmsg_gateway);
    else
        return ETERROR;

    if (pDestIpAddr)
        inet_pton(AF_INET6, pDestIpAddr, &rt.rtmsg_dst);
    else
        rt.rtmsg_dst = in6addr_any;

    rt.rtmsg_metric = metric;
    rt.rtmsg_dst_len = prefix_len;
    rt.rtmsg_flags = (RTF_UP | RTF_GATEWAY);

    /* Create a socket to the INET6 kernel. */
    if ((skfd = OpenSocketV6(ESOCK_DGRAM)) == ETERROR)
    {
        printf("[error]: EtCreateRouteV6: OpenSocketV6 error!\n");
        return ETERROR;
    }

    if (pDevName)
    {/* device setting */
        strcpy(ifr.ifr_name, pDevName);

        if (ioctl(skfd, SIOGIFINDEX, &ifr) < 0)
        {
            printf("[error]: EtCreateRouteV6: device setting error!\n");
            return ETERROR;
        }
        rt.rtmsg_ifindex = ifr.ifr_ifindex;
    }

    if (mtu)
    {/* mtu setting */
        ifr.ifr_mtu = mtu;

        if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0)
        {
            printf("[error]: EtCreateRouteV6: mtu setting error!\n");
            return ETERROR;
        }
    }

    /* Tell the kernel to delete this route. */
    if (ioctl(skfd, SIOCDELRT, &rt) < 0)
    {
        printf("[error]: EtCreateRouteV6: ioctl RTF_GATEWAY error, errno = %d -- %s\n", errno, strerror(errno));
    }
    else
    {
        result = ETOK;
    }

    rt.rtmsg_flags = (RTF_UP | RTF_HOST);
    /* Tell the kernel to delete this route. */
    if (ioctl(skfd, SIOCDELRT, &rt) < 0)
    {
        printf("[error]: EtCreateRouteV6: ioctl RTF_HOST error, errno = %d -- %s\n", errno, strerror(errno));
    }
    else
    {
        result = ETOK;
    }
    /* Close the socket. */
    CloseSocket(skfd);

#elif defined(VXWORKS)

    struct sockaddr_in6 dst, mask, gate;

    if (pDestIpAddr == NULL || pNextHop == NULL)
        return ETERROR;
    memset(&dst, 0, sizeof(dst));
    memset(&mask, 0, sizeof(mask));
    memset(&gate, 0, sizeof(gate));

    dst.sin6_family = AF_INET6;
    inet_pton(AF_INET6, pDestIpAddr, &dst.sin6_addr);
    gate.sin6_family = AF_INET6;
    inet_pton(AF_INET6, pNextHop, &gate.sin6_addr);

    if (pNetMask != NULL)
    {   // set RTF_GATEWAY
        mask.sin6_family = AF_INET6;
        inet_pton(AF_INET6, pNetMask, &mask.sin6_addr);
        if (0 > ipRouteDelete((struct sockaddr*)&dst, (struct sockaddr*)&mask, (struct sockaddr*)&gate,
            0, 0, M2_ipRouteProto_other, ROUTE_KERNEL,
            ROUTE_USAGE_IPV6, 0, RTF_UP, NULL, NULL, NULL))
        {
            printf("[error]: EtCreateRouteV6: ipRouteDelete with mask error!\r\n");
        }
    }

    // for a host route require a 0 mask (RTF_HOST)
    if (0 > ipRouteDelete((struct sockaddr*)&dst, NULL, (struct sockaddr*)&gate,
        0, 0, M2_ipRouteProto_other, ROUTE_KERNEL,
        ROUTE_USAGE_IPV6, 0, RTF_UP, NULL, NULL, NULL))
    {
        printf("[error]: EtCreateRouteV6: ipRouteDelete without mask error!\r\n");
    }
    else
    {
        result = ETOK;
    }
#endif

    return result;
}

/************************************************************************
  Function:       OpenIPv6Only
  Description:    打开IPv6套接字仅监听IPv6报文
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
EINT32 OpenIPv6Only(EFD_T sockId)
{
    int optval = ETTRUE;
    if (setsockopt(sockId, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) < 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       CloseIPv6Only
  Description:    关闭IPv6套接字仅监听IPv6报文功能
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
EINT32 CloseIPv6Only(EFD_T sockId)
{
    int optval = ETFALSE;
    if (setsockopt(sockId, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&optval, sizeof(optval)) < 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       SetIPv6UnicastHops
  Description:    设置SocketIPv6数据包中单播地址跳数限制
  Input:          sockv6:套接字句柄
                  hopLimit:单播地址跳数限制
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPv6UnicastHops(EINT32 hopLimit, EFD_T sockv6)
{
    if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char*)&hopLimit, sizeof(hopLimit)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetIPv6UnicastHops
  Description:    获取SocketIPv6数据包中单播地址跳数限制
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 GetIPv6UnicastHops(EFD_T sockv6)
{
    EINT32 hoplimit = ETERROR;
    socklen_t len = sizeof(hoplimit);
    if (getsockopt(sockv6, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char*)&hoplimit, &len) != 0)
        return ETERROR;
    else
        return hoplimit;
}

/************************************************************************
  Function:       SetIPv6MulticastHops
  Description:    设置SocketIPv6数据包中多播地址跳数限制
  Input:          sockv6:套接字句柄
                  hopLimit:多播地址跳数限制
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPv6MulticastHops(EINT32 hopLimit, EFD_T sockv6)
{
    if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&hopLimit, sizeof(hopLimit)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetIPv6MulticastHops
  Description:    获取Socket IPv6数据包中多播地址跳数限制
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 GetIPv6MulticastHops(EFD_T sockv6)
{
    EINT32 hoplimit = ETERROR;
    socklen_t len = sizeof(hoplimit);
    if (getsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&hoplimit, &len) == 0)
        return hoplimit;
    else
        return ETERROR;
}

/************************************************************************
  Function:       SetIPv6MCInterface
  Description:    设置Socket IPv6多播外出接口(网口)
  Input:          sockv6:套接字句柄
                  iF:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPv6MCInterface(IfIndex If, EFD_T sockv6)
{
    EINT32 result = ETERROR;

    if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*)&If, sizeof(If)) == 0)
        result = ETOK;
    else
        printf("[error]: SetIPv6MCInterface: setsockopt error, errno = %d -- %s\n", errno, strerror(errno));

    return result;
}

/************************************************************************
  Function:       GetIPv6MCInterface
  Description:    获取Socket IPv6多播外出接口(网口)
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
IfIndex GetIPv6MCInterface(EFD_T sockv6)
{
    IfIndex If = 0;
    socklen_t opt_len = sizeof(If);

    getsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*)&If, &opt_len);

    return If;
}

/************************************************************************
  Function:       SetIPv6UCInterface
  Description:    设置Socket IPv6单播外出接口(网口)
  Input:          sockv6:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPv6UCInterface(IfIndex If, EFD_T sockv6)
{
    EINT32 result = ETERROR;

#ifdef IPV6_UNICAST_IF
    if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_UNICAST_IF, (char*)&If, sizeof(If)) == 0)
        result = ETOK;
#endif

    return result;
}

/************************************************************************
  Function:       GetIPv6UCInterface
  Description:    获取Socket IPv6单播外出接口(网口)
  Input:          sockv6:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
IfIndex GetIPv6UCInterface(EFD_T sockv6)
{
    IfIndex If = 0;
    socklen_t opt_len = sizeof(If);

#ifdef IPV6_UNICAST_IF
    getsockopt(sockv6, IPPROTO_IPV6, IPV6_UNICAST_IF, (char*)&If, &opt_len);
#endif

    return If;
}

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    设置SocketIPv6是否环回
  Input:          sockv6:套接字句柄
                  IfLoop:是否环回判定值ETTRUE/ETFALSE
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPv6IfLoop(EUCHAR IfLoop, EFD_T sockv6)
{
    if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &IfLoop, sizeof(IfLoop)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       SetIPv6IfLoop
  Description:    设置SocketIPv6是否环回
  Input:          sockv6:套接字句柄
                  IfLoop:是否环回判定值
  Output:         无
  Return:         成功:IfLoop(ETTRUE/ETFALSE), 失败:ETERROR
  Others:         无
************************************************************************/
EUCHAR GetIPv6IfLoop(EFD_T sockv6)
{
    EUCHAR IfLoop = ETERROR;
    socklen_t opt_len = sizeof(IfLoop);
    if (getsockopt(sockv6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &IfLoop, &opt_len) != 0)
        return ETERROR;
    else
        return IfLoop;
}

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
EINT32 SetIPv6MultiCast(ECHAR* Addr, EFD_T sockv6, IfIndex IfAddr6)
{
    EINT32 result = ETERROR;
    struct ipv6_mreq mreq6;

    memset((void*)&mreq6, 0, sizeof(mreq6));

    EUINT16 version = GetIPVersion(Addr);
    if (Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv6) || version != IPv6)
    {
        printf("SetIPv6MultiCast:  version = %d   GetSockIPVersion = %d", version, GetSockIPVersion(sockv6));
        return ETERROR;
    }

    inet_pton(AF_INET6, Addr, &mreq6.ipv6mr_multiaddr);
    if (IfAddr6 == 0)
        mreq6.ipv6mr_interface = GetIPv6MCInterface(sockv6);
    else
        mreq6.ipv6mr_interface = IfAddr6;

    if (mreq6.ipv6mr_interface != 0)
    {
        if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq6, sizeof(mreq6)) == 0)
            result = ETOK;
        else
            printf("SetIPv6MultiCast:  setsockopt err\n");
    }
    else
        printf("SetIPv6MultiCast:  mreq6.ipv6mr_interface = %d\n", mreq6.ipv6mr_interface);

    return result;
}

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
EINT32 DropIPv6MultiCast(ECHAR* Addr, EFD_T sockv6, IfIndex IfAddr6)
{
    EINT32 result = ETERROR;
    struct ipv6_mreq mreq6;

    memset((void*)&mreq6, 0, sizeof(mreq6));

    EUINT16 version = GetIPVersion(Addr);
    if (Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv6) || version != IPv6)
        return ETERROR;

    inet_pton(AF_INET6, Addr, &mreq6.ipv6mr_multiaddr);
    if (IfAddr6 == 0)
        mreq6.ipv6mr_interface = GetIPv6MCInterface(sockv6);
    else
        mreq6.ipv6mr_interface = IfAddr6;

    if (mreq6.ipv6mr_interface != 0)
        if (setsockopt(sockv6, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (char*)&mreq6, sizeof(mreq6)) == 0)
            result = ETOK;

    return result;
}

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
//#define BlockIPv6Source(Addr, sock) BlockIPSourceExt(Addr, sock, 0) /* grp_port = 0 to Ignore */

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
EINT32 BlockIPv6SourceExt(ECHAR* Addr, EFD_T sock, EUINT16 grp_port)
{
    EINT32 result = ETERROR;
    struct group_source_req mreq;
    socklen_t sslen = sizeof(struct sockaddr_storage);
    struct sockaddr_in6* groupv6;
    struct sockaddr_in6* sourcev6;
    EUINT16 version = GetIPVersion(Addr);

    memset((void*)&mreq, ETERROR, sizeof(mreq));

    if (mreq.gsr_interface == ETERROR || Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sock))
        return ETERROR;

    mreq.gsr_interface = GetIPv6MCInterface(sock);

    groupv6 = (struct sockaddr_in6*)&mreq.gsr_group;
    groupv6->sin6_family = AF_INET6;
    inet_pton(AF_INET6, Addr, &groupv6->sin6_addr);
    groupv6->sin6_port = (EUINT16)etHtons(grp_port);

    sourcev6 = (struct sockaddr_in6*)&mreq.gsr_source;
    if (getsockname(sock, (struct sockaddr*)sourcev6, &sslen) == 0)
        if (setsockopt(sock, IPPROTO_IPV6, MCAST_BLOCK_SOURCE, (char*)&mreq, sizeof(mreq)) == 0)
            result = ETOK;

    return result;
}

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
//#define UnblockIPv6Source(Addr, sock) UnblockIPSourceExt(Addr, sock, 0)  /* grp_port = 0 to Ignore */

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
EINT32 UnblockIPv6SourceExt(ECHAR* Addr, EFD_T sock, EUINT16 grp_port)
{
    EINT32 result = ETERROR;
    struct group_source_req mreq;
    socklen_t sslen = sizeof(struct sockaddr_storage);
    struct sockaddr_in6* groupv6;
    struct sockaddr_in6* sourcev6;
    EUINT16 version = GetIPVersion(Addr);

    memset((void*)&mreq, ETERROR, sizeof(mreq));

    if (mreq.gsr_interface == ETERROR || Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sock))
        return ETERROR;

    mreq.gsr_interface = GetIPv6MCInterface(sock);

    groupv6 = (struct sockaddr_in6*)&mreq.gsr_group;
    groupv6->sin6_family = AF_INET6;
    inet_pton(AF_INET6, Addr, &groupv6->sin6_addr);
    groupv6->sin6_port = (EUINT16)etHtons(grp_port);

    sourcev6 = (struct sockaddr_in6*)&mreq.gsr_source;
    if (getsockname(sock, (struct sockaddr*)sourcev6, &sslen) == 0)
        if (setsockopt(sock, IPPROTO_IPV6, MCAST_UNBLOCK_SOURCE, (char*)&mreq, sizeof(mreq)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 SetIPv6PMTUD(EFD_T sock, EINT32 val)
{
#ifdef IPV6_MTU_DISCOVER
    if (val < IPV6_PMTUDISC_DONT || val > IPV6_PMTUDISC_PROBE)
        return ETERROR;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_MTU_DISCOVER, (const char*)&val, sizeof(val)) == 0)
        return ETOK;
#elif defined(IPV6_USE_MIN_MTU)
    if (val < IPV6_PMTUDISC_WANT || val > IPV6_PMTUDISC_DO)
        return ETERROR;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_USE_MIN_MTU, (const char*)&val, sizeof(val)) == 0)
        return ETOK;
#endif
    return ETERROR;
}

/************************************************************************
  Function:       GetIPv6PMTU
  Description:    获取IPv6的当前路径Path_MTU
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:PMTU值, 失败:ETERROR
  Others:         无
************************************************************************/
EUINT32 GetIPv6PMTU(EFD_T sock)
{
    struct ipv6_mtuinfo ipv6_mtuinfo;

    socklen_t mtuinfo_len = sizeof(ipv6_mtuinfo);
#ifdef IPV6_MTU
    if (getsockopt(sock, IPPROTO_IPV6, IPV6_MTU, (char*)&ipv6_mtuinfo, &mtuinfo_len) == 0)
        return ipv6_mtuinfo.ip6m_mtu;
#elif defined(IPV6_PATHMTU)
    if (getsockopt(sock, IPPROTO_IPV6, IPV6_PATHMTU, (char*)&ipv6_mtuinfo, &mtuinfo_len) == 0)
        return ipv6_mtuinfo.ip6m_mtu;
#endif
    return ETERROR;
}
