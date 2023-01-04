/************************************************************************
Copyright (C), 2022-2023
File name:       etSockV6Lib.c
Author:
Date:           
Description:     对IPv4和IPv6的socket接口进行统一, 对上层提供统一socket接口
Others:          无
*************************************************************************/

/************************************************************************
                            引用的头文件
*************************************************************************/
#include "bearpub.h"
#include "sys.h"
#include "etSockLib.h"
#include "etSockV6Lib.h"

#if defined(LINUX)
//#include <net/if.h>
#endif

/************************************************************************
                            外部函数声明
*************************************************************************/

/************************************************************************
                            全局变量声明
*************************************************************************/

/************************************************************************
                              函数定义
*************************************************************************/

/************************************************************************
  Function:       TCPIPv6_init
  Description:    对IPv6的TCPIP协议进行初始化
  Input:          无
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 TCPIP_init(void)
{
#ifdef WINDOWS
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        return ETERROR;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        return ETERROR;
    }
    return ETOK;
#endif

#if defined(AIX) || defined(LINUX)
    return ETOK;
#endif

#ifdef VXWORKS
#ifndef INET6
    return ETERROR;
#endif
    return ETOK;
#endif
}

/************************************************************************
  Function:       TCPIPv6_discard
  Description:    IPv6的TCPIP协议终止程序
  Input:          无
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 TCPIP_discard(void)
{
#ifdef WINDOWS

    if (WSACleanup() != 0)
    {
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif

#if defined(VXWORKS) || defined(LINUX) || defined(AIX)
    return ETOK;
#endif
}

/************************************************************************
  Function:       CloseSocket
  Description:    关闭一个Socket
  Input:          id:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         合法的关闭应先shutdown接收完所有数据后再closesocket
                   此处为直接关闭相关所有服务
************************************************************************/
EINT32 CloseSocket(EFD_T id)
{
    if (id <= 2)  //对于标准io不做处理
    {
        return ETOK;
    }
#ifdef WINDOWS
    if (shutdown(id, SD_BOTH) < 0)  //停止所有包括SD_RECEIVE和SD_SEND相关的主动服务   
	{
        return ETERROR;
    }
    if (closesocket(id) != 0)  // 关闭id套接字句柄的使用
    {
        return ETERROR;
    }

    return ETOK;
#endif

#ifdef VXWORKS
    if (shutdown(id, SHUT_RDWR) < 0)   // 停止所有相关的主动服务
    {
        printf("CloseSocketV6 :%ld, shutdown err:%d-------1\r\n", (EADDR_T)id, errno);
        /*PrintfLog(LOG_ERROR, FID_BEAR,
                  "[error]: CloseSocket: shutdown error, errno = %d -- %s \r\n",errno,strerror(errno));*/
                  //return ETERROR;
    }

    if (close(id) < 0)  // 关闭id套接字句柄的使用
    {
        printf("CloseSocketV6:%ld, close err:%d-------2\r\n", (EADDR_T)id, errno);
        /*PrintfLog(LOG_ERROR, FID_BEAR,
                  "[error]: CloseSocket: close error, errno = %d -- %s \r\n",errno,strerror(errno));*/
        return ETERROR;
    }
    return ETOK;
#endif

#if defined(LINUX) || defined(AIX)
    if (close(id) < 0)
    {
        printf("[error]: CloseSocketV6: close error, errno = %d -- %s \r\n", errno, strerror(errno));
        return ETERROR;
    }
    return ETOK;
#endif
}

/************************************************************************
  Function:       CalculateChecksum
  Description:    计算IPv4/IPv6数据包校验和
  Input:          pData:需要计算校验的数据部分
                   len: 数据部分的长度,单位bit
  Output:         无
  Return:         校验和
  Others:         无
************************************************************************/
EUINT16 CalculateChecksum(ECHAR* pData, EUINT32 len)
{
    EUSHORT* buffer = (EUSHORT*)pData;
    unsigned long cksum = 0;
    EUINT32 lengap = sizeof(EUSHORT);

    while (len > 1)
    {
        cksum += ntohs(*buffer++);
        len -= lengap;
    }

    if (len)
    {
#if defined ET_BIG_ENDIAN
        cksum += 0 | ((*(EUCHAR*)buffer) << 8);
#else
        cksum += *(EUCHAR*)buffer;
#endif
    }
    cksum += len;
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return (EUSHORT)(~cksum);
}

/************************************************************************
  Function:       MapIPv4toIPv6
  Description:    将整数形式的IPv4地址映射成合法的ASCII码形式的IPv6地址
  Input:          ipv4: 整数形式的IPv4地址
  Output:         ipv6: 输出IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
EINT32 MapIPv4toIPv6(EUINT32 ipv4, ECHAR* ipv6)
{
    struct sockaddr_in6 sockAddrV6;

    if (ipv6 == NULL)
        return ETERROR;

    memset(&sockAddrV6, 0, sizeof(sockAddrV6));
    sprintf(ipv6, "::ffff:%x:%x", (EUINT16)(ipv4 >> 16), (EUINT16)(ipv4 & 0xffff));
    if (inet_pton(AF_INET6, ipv6, &sockAddrV6.sin6_addr) > 0)  //检验IPv6地址的合理性
        return ETOK;
    return ETERROR;
}

/************************************************************************
  Function:       GetIPVersion
  Description:    根据IP地址获取其IP版本
  Input:          ip: 主机名(IP地址)的ASCII码形式的字符串指针
  Output:         无
  Return:         ETERROR - 处理失败  IPv4/IPv6 - 处理成功
  Others:         无
************************************************************************/
EUINT16 GetIPVersion(ECHAR* ip)
{
    struct addrinfo* result;
    if (ip == NULL)
        return ETERROR;
    if (getaddrinfo(ip, NULL, NULL, &result) == 0)
    {
        EUINT16 version = (EUINT16)(result->ai_family);
        freeaddrinfo(result);
        return version;
    }
    else
        return ETERROR;
}

/************************************************************************
  Function:       GetSockIPVersion
  Description:    获取Socket的IP版本
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  IPv4/IPv6 - 处理成功
  Others:         此接口仅用于已经绑定bind的socket
************************************************************************/
EUINT16 GetSockIPVersion(EFD_T sockId)
{
    struct sockaddr_in6 name;
    struct sockaddr* tempname;
    socklen_t maxNameLen = sizeof(struct sockaddr_in6);
    memset(&name, 0, maxNameLen);
    tempname = (struct sockaddr*)&name;
    if (0 != getsockname(sockId, tempname, &maxNameLen))
    {
#if defined(LINUX)    
        printf("[error]: GetIPVersion: getsockname error, socket maybe inexistent or not bind! "
            "errno = %d -- %s \r\n", errno, strerror(errno));
#endif
        return ETERROR;
    }

    if (tempname->sa_family != AF_INET && tempname->sa_family != AF_INET6)
        return ETERROR;
    else
        return tempname->sa_family;
}

/************************************************************************
  Function:       GetSockPort
  Description:    获取Socket绑定的端口
  Input:          sockId: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  else - 处理成功
  Others:         此接口兼容IPv4/IPv6但仅用于已经绑定bind的socket
************************************************************************/
EUINT16 GetSockPort(EFD_T sockId)
{
    struct sockaddr_in6 tempname;
    struct sockaddr* pname;
    socklen_t maxnamelen = sizeof(struct sockaddr_in6);
    memset(&tempname, 0, maxnamelen);
    pname = (struct sockaddr*)&tempname;
    if (0 != getsockname(sockId, pname, &maxnamelen))
    {
#if defined(LINUX)    
        printf("[error]: GetIPVersion: getsockname error, socket maybe inexistent or not bind! "
            "errno = %d -- %s \r\n", errno, strerror(errno));
#endif
        return ETERROR;
    }

    if (pname->sa_family != AF_INET && pname->sa_family != AF_INET6)
        return ETERROR;

    else if (pname->sa_family == AF_INET)  //IPv4
    {
        struct sockaddr_in* namev4 = (struct sockaddr_in*)pname;
        return etNtohs(namev4->sin_port);
    }
    else if (pname->sa_family == AF_INET6)  //IPv6
    {
        return etNtohs(tempname.sin6_port);
    }
    else
        return ETERROR;
}

/************************************************************************
  Function:       GetSockAddr
  Description:    获取Socket绑定的IP地址
  Input:          sockId: 套接字
  Output:         s: 指向IPv6地址ASCII码形式的字符串指针
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         此接口兼容IPv4/IPv6但仅用于已经绑定bind的socket
************************************************************************/
EINT32 GetSockAddr(EFD_T sockId, ECHAR* s)
{
    struct sockaddr_in6 tempname;
    struct sockaddr* pname;
    socklen_t maxnamelen = sizeof(struct sockaddr_in6);
    memset(&tempname, 0, maxnamelen);
    pname = (struct sockaddr*)&tempname;

    if (0 != getsockname(sockId, pname, &maxnamelen))
    {
#if defined(LINUX)    
        printf("[error]: GetIPVersion: getsockname error, socket maybe inexistent or not bind! "
            "errno = %d -- %s \r\n", errno, strerror(errno));
#endif
        return ETERROR;
    }

    if (pname->sa_family != AF_INET && pname->sa_family != AF_INET6)
        return ETERROR;

    else if (pname->sa_family == AF_INET)  //IPv4
    {
        struct sockaddr_in* namev4 = (struct sockaddr_in*)&tempname;
#if defined(WINDOWS) || defined(LINUX) || defined(AIX)
        ECHAR* str = inet_ntoa(namev4->sin_addr);
        if (str != NULL)
        {
            strcpy(s, str);
            return ETOK;
        }
#endif

#ifdef VXWORKS
        inet_ntoa_b(namev4->sin_addr, s);
        return ETOK;
#endif
    }
    else if (pname->sa_family == AF_INET6)  //IPv6
    {
        if (inet_ntop(AF_INET6, (void*)&(tempname.sin6_addr), (void*)s, (size_t)MAX_IPV6_ADDRSTR_LEN) != NULL)
            return ETOK;
    }

    return ETERROR;
}

/************************************************************************
  Function:       OpenSockReuse
  Description:    设置Socket套接字ID可(close之后)重用
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 OpenSockReuse(EFD_T sock)
{
# ifndef WINDOWS
    EINT32  optval = ETTRUE;
#else
    BOOL    optval = ETTRUE;
#endif
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       CloseSockReuse
  Description:    禁止Socket套接字ID可重用
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 CloseSockReuse(EFD_T sock)
{
#ifndef WINDOWS
    EINT32 optval = ETFALSE;
#else
    BOOL   optval = ETFALSE;
#endif
    if (0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)))
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       SetSockDontRoute
  Description:    设置socket不经过路由/网关而直接发往目标主机
  Input:          sockId: 设置的socketID
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         无
************************************************************************/
EINT32 SetSockDontRoute(EFD_T sockId)
{
    EBOOL flag = ETTRUE;

    if (0 != setsockopt(sockId, SOL_SOCKET, SO_DONTROUTE, (ECHAR*)&flag, sizeof(flag)))
    {
#if defined(LINUX)    
        printf("[error]: SetSockDontRoute: setsockopt error, errno = %d -- %s \r\n", errno, strerror(errno));
#endif
        return ETERROR;
    }
    return ETOK;
}

/************************************************************************
  Function:       IsIpAddrV6MappedFromV4
  Description:    判断IPv6ASCII码形式的字符串的地址是否是由IPv4地址映射而来
  Input:          IPAddrv6: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
EINT32 IsIpAddrV6MappedFromV4(ECHAR* IPAddrv6)
{
    IPv6Addr_T ipv6 = GetIpAddrV6(IPAddrv6);
    return IN6_IS_ADDR_V4MAPPED(&ipv6) == 0 ? ETFALSE : ETTRUE;
}

/************************************************************************
  Function:       IsIpAddrV6CompatV4
  Description:    判断IPv6ASCII码形式的字符串的地址是否和IPv4地址兼容
  Input:          IPAddrv6: IPv6地址字符串指针
  Output:         无
  Return:         ETFALSE - 判断失败(不同/异常)  ETTRUE - 判断成功(相同)
  Others:         无
************************************************************************/
EINT32 IsIpAddrV6CompatV4(ECHAR* IPAddrv6)
{
    IPv6Addr_T ipv6 = GetIpAddrV6(IPAddrv6);
    return IN6_IS_ADDR_V4COMPAT(&ipv6) == 0 ? ETFALSE : ETTRUE;
}

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    设置Socket接收缓冲区大小
  Input:          sock:套接字句柄
                   bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 SetSockRCVBuffer(EFD_T sock, EINT32 bufferSize)
{
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, sizeof(bufferSize)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    获取Socket接收缓冲区大小
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 GetSockRCVBuffer(EFD_T sock)
{
    EINT32 bufferSize = ETERROR;
    socklen_t bufferSize_len = sizeof(bufferSize);
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&bufferSize, &bufferSize_len) != 0)
        return ETERROR;
    else
        return bufferSize;
}

/************************************************************************
  Function:       SetSockRCVBuffer
  Description:    设置Socket发送缓冲区大小
  Input:          sock:套接字句柄
                   bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 SetSockSNDBuffer(EFD_T sock, EINT32 bufferSize)
{
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, sizeof(bufferSize)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockRCVBuffer
  Description:    获取Socket发送缓冲区大小
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 GetSockSNDBuffer(EFD_T sock)
{
    EINT32 bufferSize = ETERROR;
    socklen_t bufferSize_len = sizeof(bufferSize);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&bufferSize, &bufferSize_len) != 0)
        return ETERROR;
    else
        return bufferSize;
}

/************************************************************************
  Function:       SetSockRCVLowWat
  Description:    设置Socket接收缓冲区低水位标记
                   低水位标记由IO复用函数select/poll/epoll函数使用
                   其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
                   bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 SetSockRCVLowWat(EFD_T sock, EINT32 bufferSize)
{
    if (setsockopt(sock, SOL_SOCKET, SO_RCVLOWAT, (char*)&bufferSize, sizeof(bufferSize)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockRCVLowWat
  Description:    获取Socket接收缓冲区低水位标记
                   低水位标记由IO复用函数select/poll/epoll函数使用
                   其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 GetSockRCVLowWat(EFD_T sock)
{
    EINT32 bufferSize = ETERROR;
    socklen_t bufferSize_len = sizeof(bufferSize);
    if (getsockopt(sock, SOL_SOCKET, SO_RCVLOWAT, (char*)&bufferSize, &bufferSize_len) != 0)
        return ETERROR;
    else
        return bufferSize;
}

/************************************************************************
  Function:       SetSockSNDLowWat
  Description:    设置Socket发送缓冲区低水位标记
                   低水位标记由IO复用函数select/poll/epoll函数使用
                   其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
                   bufferSize: 设置的大小值
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 SetSockSNDLowWat(EFD_T sock, EINT32 bufferSize)
{
    if (setsockopt(sock, SOL_SOCKET, SO_SNDLOWAT, (char*)&bufferSize, sizeof(bufferSize)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockSNDLowWat
  Description:    获取Socket发送缓冲区低水位标记
                   低水位标记由IO复用函数select/poll/epoll函数使用
                   其值为函数返回时套接字接收/发送缓冲区中所需的最少数据量
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:bufferSize, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
************************************************************************/
EINT32 GetSockSNDLowWat(EFD_T sock)
{
    EINT32 bufferSize = ETERROR;
    socklen_t bufferSize_len = sizeof(bufferSize);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDLOWAT, (char*)&bufferSize, &bufferSize_len) != 0)
        return ETERROR;
    else
        return bufferSize;
}

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
EINT32 SetSockRCVTimeout(EFD_T sock, EINT32 time)
{
    struct timeval timeval;
    memset(&timeval, 0, sizeof(timeval));
    if (time > 1000)
    {
        timeval.tv_usec = time % 1000;
        timeval.tv_sec = (time - timeval.tv_usec) / 1000;
    }
    else
        timeval.tv_usec = time;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeval, sizeof(timeval)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockRCVTimeout
  Description:    获取Socket套接字的接收超时值
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:TimeoutValue, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                   接收超时影响的主要函数:read/readv/recv/recvfrom/recvmsg
************************************************************************/
EINT32 GetSockRCVTimeout(EFD_T sock)
{
    struct timeval timeval;
    memset(&timeval, 0, sizeof(timeval));
    socklen_t timeval_len = sizeof(timeval);
    if (getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeval, &timeval_len) != 0)
        return ETERROR;
    else
        return ((timeval.tv_sec * 1000) + timeval.tv_usec);
}

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
EINT32 SetSockSNDTimeout(EFD_T sock, EINT32 time)
{
    struct timeval timeval;
    memset(&timeval, 0, sizeof(timeval));
    if (time > 1000)
    {
        timeval.tv_usec = time % 1000;
        timeval.tv_sec = (time - timeval.tv_usec) / 1000;
    }
    else
        timeval.tv_usec = time;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeval, sizeof(timeval)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetSockSNDTimeout
  Description:    获取Socket套接字的发送超时值
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:TimeoutValue, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                   发送超时影响的主要函数:write/writev/send/sendto/sendmsg
************************************************************************/
EINT32 GetSockSNDTimeout(EFD_T sock)
{
    struct timeval timeval;
    memset(&timeval, 0, sizeof(timeval));
    socklen_t timeval_len = sizeof(timeval);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeval, &timeval_len) != 0)
        return ETERROR;
    else
        return ((timeval.tv_sec * 1000) + timeval.tv_usec);
}

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
EINT32 SetSockLinger(EFD_T sock, EINT32 onoff, EINT32 linger_time)
{
    struct linger lingerv6;
    lingerv6.l_onoff = onoff;
    lingerv6.l_linger = linger_time;
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&lingerv6, sizeof(lingerv6)) == 0)
        return ETOK;
    else
        return ETERROR;
}

/************************************************************************
  Function:       GetSockLinger
  Description:    获取Socket套接字(若有数据待发送)延迟关闭时间
  Input:          sock:套接字句柄
  Output:         无
  Return:         成功:linger_time逗留的时间, 失败:ETERROR
  Others:         此接口兼容IPv4/IPv6
                   linger值由closesocket()调用,当有数据没发送完毕的时候容许逗留
************************************************************************/
EINT32 GetSockLinger(EFD_T sock)
{
    struct linger lingerv6;
    memset((void*)&lingerv6, 0, sizeof(lingerv6));
    socklen_t lingerv6Size = sizeof(lingerv6);
    if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&lingerv6, &lingerv6Size) == 0)
    {
        if (lingerv6.l_onoff != 0)
            return lingerv6.l_linger;
        else
            return ETERROR;
    }
    else
        return ETERROR;
}

/************************************************************************
  Function:       GetIfIndex
  Description:    根据网口名获取其索引值
  Input:          ifname:网口名字符串地址
  Output:         无
  Return:         成功:IfIndex, 失败:ETERROR
  Others:         此接口适用linux/vxworks
************************************************************************/
IfIndex GetIfIndex(ECHAR * ifname)
{
#if defined(LINUX) || defined(VXWORKS)
    IfIndex result = if_nametoindex(ifname);
    return (result > 0) ? result : ETERROR;
#else
    return ETERROR;
#endif
}

/************************************************************************
  Function:       GetIfName
  Description:    根据网口索引值获取其名字
  Input:          ifindex:网口索引值
  Output:         ifname:网口名字符串地址
  Return:         成功:ETOK, 失败:ETERROR
  Others:         此接口适用linux/vxworks
************************************************************************/
EINT32 GetIfName(IfIndex ifindex, ECHAR * ifname)
{
#if defined(LINUX) || defined(VXWORKS)
    return (NULL == if_indextoname(ifindex, ifname)) ? ETOK : ETERROR;
#else
    return ETERROR;
#endif
}
