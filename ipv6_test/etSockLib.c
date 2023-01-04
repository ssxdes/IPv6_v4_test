/************************************************************************
Copyright (C), 2009-2010
File name:      etSocketLib.c
Author:        
Date:          
Description:    对不同操作系统的socket IPv4接口进行封装，对上层提供统一socket接口
Others:         无
*************************************************************************/

/************************************************************************
                            引用的头文件
*************************************************************************/
#include "etSockLib.h"
#include "bearpub.h"
#include "sys.h"
#include <errno.h>
#ifdef WINDOWS
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif //WINDOWS

#ifdef LINUX
#include <linux/route.h>
#endif
static EINT32  g_B2PeerIP = 0;
static EINT32  g_B2State = 0;
#define IP_ORIGDSTADDR 20

/**************************外部函数声明***************************/
#ifdef VXWORKS
extern int mRouteAdd(char *pDest, char *pGate,long mask, int tos, int flags);
#endif

/************************************************************************
                               函数定义
*************************************************************************/
void SetB2PeerIP(EINT32 ipAddr)
{
    g_B2PeerIP = ipAddr;
}
EINT32 GetB2PeerIP()
{
    return g_B2PeerIP;
}
void SetIcpChanSelect(EUINT32 flag)
{
    g_B2State = flag;
}
EINT32 GetIcpChanSelect()
{
    return g_B2State;
}
/************************************************************************
  Function:       OpenRawSocket
  Description:    打开一个原始IPv4 Socket
  Input:          protocol:接收的IP数据包类型
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
EFD_T OpenRawSocket(EINT32 protocol)
{
    EFD_T  sock;
    EINT32  optval;
    static int errNum = 0;

    optval=1;

    sock = socket(AF_INET, ESOCK_RAW, protocol);

    if(sock <= 0)
    {
#if defined(LINUX)        
        printf("[error]: OpenRawSocket: socket error, errno = %d -- %s \r\n",errno,strerror(errno));
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

    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0)  //将SOKCET地址置为可重用
    {
#if defined(LINUX)        
        printf("[error]: OpenRawSocket: setsockopt error, errno = %d -- %s \r\n",errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return sock;
    }

}

/************************************************************************
  Function:       OpenSocket
  Description:    打开一个IPv4 Socket
  Input:          type:ESOCK_STREAM:stream socket，ESOCK_DGRAM:datagram socket，
  Output:         无
  Return:         成功:Socket 句柄, 失败:ETERROR
  Others:         无
************************************************************************/
EFD_T OpenSocket(EINT32 type)
{
    EFD_T  sock;
    static int errNum = 0;

    switch(type)
    {
    case ESOCK_DGRAM:
    case ESOCK_STREAM:
        sock = socket(AF_INET, type, 0);
        break;
    case ESOCK_RAW:
        sock = socket(AF_INET, type,IPPROTO_IP);
        break;
    default:
        return ETERROR;
    }

    if(sock < 0)
    {
#if defined(LINUX)        
        printf("[error]: OpenSocket: socket error, errno = %d -- %s \r\n",errno,strerror(errno));
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
    if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
    {
        printf("OpenSocket fcntl err %d\n", errno);
        /*PrintfLog(LOG_ERROR, FID_BEAR,
                  "[error]: OpenSocket: fcntl error, errno = %d -- %s \r\n",errno,strerror(errno));*/
    }
#endif
#if defined (WINDOWS)
    SetHandleInformation((HANDLE)((EFD_T)0 + sock), HANDLE_FLAG_INHERIT, 0);
#endif
    return sock;

}

/************************************************************************
  Function:       SetSocketNonblock
  Description:    将IPv4 Socket设置为非阻塞方式
  Input:          sockv4:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetSocketNonblock(EFD_T sockv4)
{
#ifdef WINDOWS
    unsigned long flag = 1;            //0:阻塞方式, 1:非阻塞方式

    if(ioctlsocket(sockv4,FIONBIO,&flag) < 0)
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

    if(ioctl(sockv4, FIONBIO, (int)&flag) < 0)
    {
        //printf("[error]: SetSocketNonblock: ioctl error, errno = %d -- %s \r\n",errno,strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif

#if defined(LINUX) || defined(AIX)
    EUINT32 flag;

    flag = fcntl(sockv4, F_GETFL, 0);
    if(fcntl(sockv4, F_SETFL, flag|O_NONBLOCK) < 0)
    {
        printf("[error]: SetSocketNonblock: fcntl error, errno = %d -- %s \r\n",errno,strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }
#endif
}

/************************************************************************
Function:       SetSocketLoosRoute
Description:    将IPv4 Socket设置为松散路由模式
Input:          sockv4:Socket句柄, addr:指向路由IP地址列表的指针, num:路由表中路由地址的个数
Output:         无
Return:         成功:ETOK, 失败:ETERROR
Others:         无
************************************************************************/
EINT32 SetSocketLoosRoute(EFD_T sockv4, EUINT32 *addr, EUINT32 num)
{
#ifdef WINDOWS
    struct ip_option_hdr
    {
        EUCHAR    code;
        EUCHAR    length;
        EUCHAR    offset;
        EUCHAR    addrs[9];
    } opthdr;

    opthdr.code = 131;
    opthdr.length = (EUCHAR)(num*4 + 3);
    opthdr.offset = 4;
    memcpy(opthdr.addrs, addr, (size_t)num*4);
    return setsockopt(sockv4, IPPROTO_IP, IP_OPTIONS, (ECHAR *)&opthdr, opthdr.length);
#endif //WINDOWS

#ifdef VXWORKS
    IP_OPTION opthdr;

    opthdr.option_type = IP_LOOSE_SOURCE_AND_RECORD_ROUTE_TYPE;
    opthdr.option_data_length = 4*num + 8;
    memcpy(opthdr.option_data, addr, num*4);
    return setsockopt(sockv4, IPPROTO_IP, IP_OPTIONS, (char *)&opthdr, opthdr.option_data_length);
#endif //VXWORKS

#if defined(LINUX) || defined(AIX)
    return ETERROR;
#endif
}

/************************************************************************
  Function:       SetSocketQos
  Description:    设置IPv4 socket的qos类型
  Input:          sockv4:Socket句柄; buf[]: 缓存; qos:qos类型;
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetSocketQos(EFD_T sockv4, EUINT32 qos)
{
    EUINT32 ipTos = (qos<<2);

    if(setsockopt(sockv4, IPPROTO_IP, IP_TOS, (char *)&ipTos, sizeof(ipTos)) < 0)
    {
#if defined(LINUX)        
        printf("[error]: SetSocketQos: setsockopt error, errno = %d -- %s \r\n",errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return ETOK;
    }

}

/************************************************************************
  Function:       BindSocket
  Description:    将IPv4 Socket与本地端口绑定
  Input:          sockv4:Socket句柄, port: 端口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 BindSocket(EFD_T sockv4, EUINT16 port)
{
    struct sockaddr_in serverAddr;
    EUINT32 sockAddrSize;

    sockAddrSize = sizeof(struct sockaddr_in);
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = (EUINT16)etHtons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
#ifdef VXWORKS
    serverAddr.sin_len = (EUCHAR)sockAddrSize;
#endif

    if(bind(sockv4, (struct sockaddr *)&serverAddr, sockAddrSize) < 0)
    {
#if defined(LINUX)        
        printf("[error]: BindSocket: bind error, errno = %d -- %s \r\n",errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return ETOK;
    }
}

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
EINT32 BindSrcIpSocket(EFD_T sockv4, EUINT32 srcIp, EUINT16 port)
{
    struct sockaddr_in serverAddr;
    EUINT32 sockAddrSize;

    sockAddrSize = sizeof(struct sockaddr_in);
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = (EUINT16)etHtons(port);
    serverAddr.sin_addr.s_addr = srcIp;
#ifdef VXWORKS
    serverAddr.sin_len = (EUCHAR)sockAddrSize;
#endif

    if(bind(sockv4, (struct sockaddr *)&serverAddr, sockAddrSize) < 0)
    {
#if defined(LINUX)        
        printf("[error]: BindSrcIpSocket: bind error, errno = %d -- %s \r\n",errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return ETOK;
    }
}

/************************************************************************
  Function:       ListenSocket
  Description:    启动IPv4 Socket监听
  Input:          sockv4:Socket句柄
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ListenSocket(EFD_T sockv4)
{
    if (listen(sockv4, SERVER_MAX_CONNECTIONS) < 0)
    {
        printf("[error]: ListenSocket: listen error, errno = %d -- %s \r\n",errno,strerror(errno));
        return ETERROR;
    }
    else
    {
        return ETOK;
    }

}

/************************************************************************
  Function:       AcceptSocket
  Description:    接受IPv4客户端的请求
  Input:          sockv4:Socket句柄;
  Output:         pClientIpAddr: 客户端IP地址
  Return:         成功:新建的Socket句柄 , 失败:ETERROR
  Others:         无
************************************************************************/
EADDR_T AcceptSocket(EFD_T sockv4, EUINT32 *pClientIpAddr)
{
    struct sockaddr_in clientAddr;
    EINT32 sockAddrSize;
    EADDR_T fd;

    sockAddrSize = sizeof(clientAddr);
    memset(&clientAddr, 0, sockAddrSize);

    fd = accept(sockv4, (struct sockaddr *)&clientAddr, (socklen_t *)&sockAddrSize);
    if(fd < 0)
    {
#if defined(LINUX)        
        printf("[error]: AcceptSocket: accept error, errno = %d -- %s \r\n",errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
#if defined (LINUX) || defined(AIX)
        if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
        {
            printf("OpenSocket fcntl err %d\n", errno);
            /*PrintfLog(LOG_ERROR, FID_BEAR,
                      "[error]: AcceptSocket: fcntl error, errno = %d -- %s \r\n",errno,strerror(errno));*/
        }
#endif
#if defined (WINDOWS)
        SetHandleInformation((HANDLE )fd, HANDLE_FLAG_INHERIT, 0);
#endif
        *pClientIpAddr = clientAddr.sin_addr.s_addr;
        return fd;
    }

}

/************************************************************************
  Function:       ConnectSocket
  Description:    IPv4客户端呼叫服务器
  Input:          sockv4:Socket句柄; remoteIpAddr: 服务器IP地址;port:端口
  Output:         无
  Return:         成功:ETOK , 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ConnectSocket(EFD_T sockv4, EUINT32 remoteIpAddr, EUINT16 port)
{
#ifdef WINDOWS
    struct sockaddr_in serverAddr;
    EUINT32 sockAddrSize;
    struct timeval WaitTime;
    fd_set FdSet;
    unsigned long flag = 1;
  

    if(ioctlsocket(sockv4, FIONBIO, &flag) < 0)                       //set nublock mode
    {
        return ETERROR;
    }

    sockAddrSize = sizeof (struct sockaddr_in);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = (EUINT16)etHtons(port);
    serverAddr.sin_addr.s_addr = remoteIpAddr;
    connect((SOCKET)sockv4, (struct sockaddr *)&serverAddr, sockAddrSize);

    FD_ZERO(&FdSet);
    FD_SET(sockv4, &FdSet);
    WaitTime.tv_sec = 3;
    WaitTime.tv_usec = 0;
    if(select(FD_SETSIZE, 0, &FdSet, 0, &WaitTime) <= 0)                    //wait 3 sec
    {
        flag= 0;
        ioctlsocket(sockv4, FIONBIO, &flag);                        //resume block mode
        return ETERROR;
    }
    else
    {
        flag= 0;
        ioctlsocket(sockv4, FIONBIO, &flag);
        return ETOK;
    }
#endif

#if defined(LINUX) || defined(AIX)
    struct sockaddr_in serverAddr;
    EUINT32 sockAddrSize;
    struct timeval WaitTime;
    fd_set FdSet;
    EUINT32 flag;
    EINT8 str[16];

    flag = fcntl(sockv4, F_GETFL, 0);
    if(fcntl(sockv4, F_SETFL, flag|O_NONBLOCK) < 0)                       //set nublock mode
    {
        str[15] = '\0';
        GetIpStr(remoteIpAddr, str);
        printf("[error]:socket:0x%x ConnectSocket: fcntl error, remoteIp:%s, errno = %d -- %s \r\n", sockv4, str, errno, strerror(errno));
        return ETERROR;
    }

    sockAddrSize = sizeof (struct sockaddr_in);
    memset(&serverAddr,0,sizeof(sockAddrSize));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = (EUINT16)etHtons(port);
    serverAddr.sin_addr.s_addr = remoteIpAddr;
    connect(sockv4, (struct sockaddr *)&serverAddr, sockAddrSize);

    FD_ZERO(&FdSet);
    FD_SET(sockv4, &FdSet); WaitTime.tv_sec = 3;
    WaitTime.tv_usec = 0;
    if(select(FD_SETSIZE, 0, &FdSet, 0, &WaitTime) <= 0)                    //wait 3 sec
    {
        flag = fcntl(sockv4, F_GETFL, 0);
        fcntl(sockv4, F_SETFL, flag&~O_NONBLOCK);                        //resume block mode return ETERROR;
    }
    else
    {
        flag = fcntl(sockv4, F_GETFL, 0);
        fcntl(sockv4, F_SETFL, flag&~O_NONBLOCK);
        return ETOK;
    }
    return ETERROR;
#endif //LINUX

#ifdef VXWORKS
    struct sockaddr_in serverAddr;
    EUINT32 sockAddrSize;
    struct timeval WaitTime;

    sockAddrSize = sizeof (struct sockaddr_in);
    memset(&serverAddr,0,sizeof(sockAddrSize));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_len = (ECHAR) sockAddrSize;
    serverAddr.sin_port = etHtons(port);
    serverAddr.sin_addr.s_addr = remoteIpAddr;
    WaitTime.tv_sec=3;                //wait 3 sec
    WaitTime.tv_usec=0;

    if (connectWithTimeout(sockv4,(struct sockaddr *)&serverAddr,sockAddrSize,&WaitTime) < 0)
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
  Function:       ReadFrame
  Description:    接收IPv4 UDP帧数据
  Input:          sockv4: Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadFrame(EFD_T sockv4,  ECHAR buf[],  EUINT32 len, EUINT32 *pRemoteIpAddr)
{
    struct sockaddr_in remoteAddr;
    EINT32 sockAddrSize;
    EINT32 recvLen;

    sockAddrSize = sizeof(struct sockaddr_in);
    recvLen = recvfrom(sockv4, buf, len, 0, (struct sockaddr *)&remoteAddr, (socklen_t *)&sockAddrSize);
    if(recvLen <= 0)
    {
#if defined(LINUX)        
        printf("[error]:socket:0x%x ReadFrame: recvfrom error, errno = %d -- %s \r\n", sockv4, errno, strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        *pRemoteIpAddr = remoteAddr.sin_addr.s_addr;
        return recvLen;
    }
}

/************************************************************************
  Function:       ReadFrameExt
  Description:    接收IPv4 UDP帧数据
  Input:          sockv4: Socket句柄; len:缓冲区的长度
  Output:         buf[]: 缓存; pRemoteIpAddr: 数据帧源IP地址; pRemotePort: 数据帧源端口
  Return:         成功:大于0，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadFrameExt(EFD_T sockv4,  ECHAR buf[],  EUINT32 len, EUINT32 *pRemoteIpAddr, EUINT32 * pRemotePort)
{
    struct sockaddr_in remoteAddr;
    EINT32 sockAddrSize;
    EINT32 recvLen;

    sockAddrSize = sizeof(struct sockaddr_in);
    recvLen = recvfrom(sockv4, buf, len, 0, (struct sockaddr *)&remoteAddr, (socklen_t *)&sockAddrSize);
    if(recvLen <= 0)
    {
#if defined(LINUX)        
        printf("[error]:socket:0x%x ReadFrameExt: recvfrom error, errno = %d -- %s \r\n", sockv4, errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        *pRemoteIpAddr = remoteAddr.sin_addr.s_addr;
        *pRemotePort=    remoteAddr.sin_port;
        return recvLen;
    }
}

/************************************************************************
  Function:       SendFrame
  Description:    发送IPv4 UDP帧数据
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
                  remoteIpAddr: 数据帧目的IP地址;port:端口
  Output:         无
  Return:         成功: 发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SendFrame(EFD_T sockv4, ECHAR buf[], EUINT32 len, EINT32 remoteIpAddr, EUINT16 port)
{
    struct sockaddr_in remoteAddr;
    EUINT32 sockAddrSize;
    EINT32 sendlen;
#if defined(LINUX)   
    EINT8 str[16];
#endif

    if (0 == remoteIpAddr
        || 0 == len)
    {
        return ETERROR;
    }

    sockAddrSize = sizeof (struct sockaddr_in);
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = (EUINT16)etHtons(port);
    remoteAddr.sin_addr.s_addr = remoteIpAddr;
#ifdef VXWORKS
    remoteAddr.sin_len = (ECHAR)sockAddrSize;
#endif
    sendlen = sendto(sockv4, buf, len, 0, (struct sockaddr*)&remoteAddr, sockAddrSize);
    if (sendlen <= 0)
    {
#if defined(LINUX)        
        str[15] = '\0';
        GetIpStr(remoteIpAddr, str);
        printf("[error]:socket:0x%x SendFrame: sendto error,remoteIP:%s, errno = %d -- %s \r\n", sockv4, str, errno,strerror(errno));
#endif

        return ETERROR;
    }
    else
    {
        return sendlen;
    }

}

/************************************************************************
  Function:       ReadStream
  Description:    接收IPv4帧数据的缓冲区
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，接收到实际的数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 ReadStream(EFD_T sockv4, ECHAR buf[], EINT32 len)
{
    EINT32 recvlen;
    recvlen = recv(sockv4, buf, len, 0);
    if(recvlen <= 0)
    {
    #if defined(LINUX)    
        printf("[error]:socket:0x%x ReadStream: recv error, errno = %d -- %s \r\n", sockv4, errno, strerror(errno));
    #endif
    
        return ETERROR;
    }
    else
    {
        return recvlen;
    }
}

/************************************************************************
  Function:       SendStream
  Description:    发送IPv4帧数据
  Input:          sockv4:Socket句柄; buf[]: 缓存; len:缓冲区的长度;
  Output:         无
  Return:         成功:大于0，成功，发送的实际数据字节数, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SendStream(EFD_T sockv4,ECHAR buf[], EINT32 len)
{
    EINT32 sendlen;
    sendlen = send(sockv4, buf, len, 0);
    if(sendlen <= 0)
    {
    #if defined(LINUX)    
        printf("[error]:socket:0x%x SendStream: send error, errno = %d -- %s \r\n", sockv4, errno, strerror(errno));
    #endif
    
        return ETERROR;
    }
    else
    {
        return sendlen;
    }
}

/************************************************************************
  Function:       GetIpAddr
  Description:    将ASCII码形式的IPv4地址转换成整型
  Input:          s:指向IPv4地址ASCII码形式的指针
  Output:         无
  Return:         IP地址的整数类型
  Others:         无
************************************************************************/
EUINT32 GetIpAddr(ECHAR *s)
{
    return inet_addr(s);
}

/************************************************************************
  Function:       GetIpStr
  Description:    将整型的IPv4地址转换成ASCII码形式
  Input:          ipAddr:整数类型的IPv4地址
  Output:         s:指向IPv4地址ASCII码形式的指针
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 GetIpStr(EUINT32 ipAddr, ECHAR *s)
{
#if defined(WINDOWS) || defined(LINUX) || defined(AIX)
    ECHAR* str;
    struct in_addr tmpIpAddr;

    tmpIpAddr.s_addr = ipAddr;
    str = inet_ntoa(tmpIpAddr);
    strcpy(s,str);

    return ETOK;
#endif

#ifdef VXWORKS
    struct in_addr inetAddress;

    inetAddress.s_addr = ipAddr;
    inet_ntoa_b(inetAddress, s);

    return ETOK;
#endif
}

/************************************************************************
  Function:       ReadFromTo
  Description:    使用recvmsg接收数据包，使得可以获取ip头的目标地址
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
EINT32 ReadFromTo(EFD_T sockv4, void *pRecvBuf,EUINT16 bufLen,
                EUINT32 *pSrcIp, EUINT32 *pDstIp)
{
    EINT32 recvLen = -1;
    EUINT32 Addr = 0;
    ECHAR RecvBuf[1024];
    EUINT32 RecvBufLen =1024; 
#if defined(LINUX) || defined(AIX)
    struct msghdr msg;
    struct cmsghdr *pCmsg;
    struct iovec iov;
    struct sockaddr_in srcAddr = {0};
    ECHAR control[64] = {'\0'};
    if (NULL == pRecvBuf
        || 0 == bufLen
        || NULL == pSrcIp
        || NULL == pDstIp)
    {
        printf("%s(%d)--:"
            "In ReadFromTo: the parameter passed is unavailable!\r\n",
            __FILE__, __LINE__);
        msg.msg_name        = &Addr;
        msg.msg_namelen     = sizeof(struct sockaddr_in);
        iov.iov_base        = (ECHAR*)RecvBuf;
        iov.iov_len         = RecvBufLen;
        msg.msg_iov         = &iov;
        msg.msg_iovlen      = 1;
        msg.msg_control     = (void*)control;
        msg.msg_controllen  = sizeof(control);
        msg.msg_flags       = 0;
        recvLen = recvmsg(sockv4, &msg, 0);
        return ETERROR;
    }// restart error might be here.

    msg.msg_name        = &srcAddr;
    msg.msg_namelen     = sizeof(struct sockaddr_in);
    iov.iov_base        = (ECHAR*)pRecvBuf;
    iov.iov_len         = bufLen;
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;
    msg.msg_control     = (void*)control;
    msg.msg_controllen  = sizeof(control);
    msg.msg_flags       = 0;

    recvLen = recvmsg(sockv4, &msg, 0);
    if (recvLen < 0)
    {
        printf("%s(%d)--:"
            "In ReadFromTo: recvmsg return -1, errno = %d -- %s \r\n",
            __FILE__, __LINE__, errno,strerror(errno));
        return ETERROR;
    }

    *pSrcIp = srcAddr.sin_addr.s_addr;

    for(pCmsg=CMSG_FIRSTHDR(&msg); NULL!=pCmsg; pCmsg=CMSG_NXTHDR(&msg, pCmsg))
    {
        #if (defined(LINUX) && defined(IP_PKTINFO))
        if (pCmsg->cmsg_level == IPPROTO_IP
            && pCmsg->cmsg_type == IP_PKTINFO)
        {
            *pDstIp = ((struct in_pktinfo *)CMSG_DATA(pCmsg))->ipi_addr.s_addr;
            break;
        }
        #elif (defined(AIX) && defined(IP_RECVDSTADDR))
        if (pCmsg->cmsg_level == IPPROTO_IP
            && pCmsg->cmsg_type == IP_RECVDSTADDR)
        {
            *pDstIp = ((struct in_addr*)((EUINT32)CMSG_DATA(pCmsg)))->s_addr;
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
    (void)sockv4;
#endif
    return recvLen;

}

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
EINT32 EtCreateRoute(ECHAR *pDestIpAddr,
                            EUINT32 intIpMask,
                            ECHAR *pNextHop,
                            ECHAR *pDevName)

#else
EINT32 EtCreateRoute(ECHAR *pDestIpAddr,
                      EUINT32 intIpMask,
                      ECHAR *pNextHop)
#endif
{
    EINT32 result = -1;

#ifdef VXWORKS
    if (ETERROR == mRouteAdd(pDestIpAddr, pNextHop, intIpMask, 0, 0))
    {
        return ETERROR;
    }
    result = 0;
#elif defined(LINUX)
    struct rtentry rt;
    struct sockaddr_in *pSockDst             = (struct sockaddr_in *)(&rt.rt_dst);
    struct sockaddr_in *pSockGateway         = (struct sockaddr_in *)(&rt.rt_gateway);
    struct sockaddr_in *pSockMask             = (struct sockaddr_in *)(&rt.rt_genmask);
    EFD_T sockv4 = -1;

    memset(&rt, '\0', sizeof(rt));

    pSockDst->sin_addr.s_addr       = GetIpAddr(pDestIpAddr);
    pSockDst->sin_family            = AF_INET;
    pSockDst->sin_port                = 0;

    pSockMask->sin_addr.s_addr        = etHtonl(intIpMask);
    pSockMask->sin_family            = AF_INET;
    pSockMask->sin_port                = 0;

    pSockGateway->sin_addr.s_addr     = GetIpAddr(pNextHop);
    pSockGateway->sin_family        = AF_INET;
    pSockGateway->sin_port            = 0;

    rt.rt_dev     = pDevName;
    rt.rt_flags =  (RTF_UP|RTF_GATEWAY);

    sockv4 = OpenSocket(ESOCK_DGRAM);
    if (sockv4 == ETERROR)
    {

        printf("[error]: EtCreateRoute: OpenSocket error\n");
        return -1;
    }

    if (ioctl(sockv4, SIOCADDRT, &rt) < 0)
    {
        printf("[error]: EtCreateRoute: ioctl RTF_GATEWAY error,%s\n", strerror(errno));
    }
    else
    {
        result = 0;
    }

    rt.rt_flags =  (RTF_UP|RTF_HOST);
    if (ioctl(sockv4, SIOCADDRT, &rt) < 0)
    {
        printf("[error]: EtCreateRoute: ioctl RTF_HOST error, errno = %d -- %s \r\n",errno,strerror(errno));
        //printf("[error]: EtCreateRoute: ioctl RTF_HOST error\n");
    }
    else
    {
        result = 0;
    }

    CloseSocket(sockv4);

#elif 0 //defined(AIX)
    char tmpBuf[128] = "\0";
    int ret;
    int ip;

    ip = GetIpAddr(pNextHop);
    if (ip == 0)
    {
        printf("gateway = 0.0.0.0, not set gateway !\n");
        return 0;
    }
    
    memset(tmpBuf, 0, sizeof(tmpBuf));
    sprintf(tmpBuf, "route add -net %s %s -if %s ", pDestIpAddr, pNextHop, pDevName);

    ret = system(tmpBuf);
    if (ret != 0 && ret != 256)
    {
        printf("route add gateway err,%s,%s,%s\n", pDestIpAddr, pNextHop, pDevName);
    }

    result = 0;
    
#elif defined(WINDOWS)
    (void)pNextHop;
    (void)pDestIpAddr;
    (void)intIpMask;
#endif

return result;
}

/************************************************************************
  Function:       EtDeleteRoute
  Description:    删除IPv4路由
  Input:          pDestIpAddr: 目标地址
                  intIpMask: 目标地址掩码
                  pNextHop: 下一条地址
                  pDevName: 设备名称
  Output:         无
  Return:
  Others:         此接口用于linux\vxworks
************************************************************************/
#if defined(LINUX)
EINT32 EtDeleteRoute(ECHAR *pDestIpAddr,
                            EUINT32 intIpMask,
                            ECHAR *pNextHop,
                            ECHAR *pDevName)
{
    EINT32 result = -1;

    struct rtentry rt;
    struct sockaddr_in *pSockDst             = (struct sockaddr_in *)(&rt.rt_dst);
    struct sockaddr_in *pSockGateway         = (struct sockaddr_in *)(&rt.rt_gateway);
    struct sockaddr_in *pSockMask             = (struct sockaddr_in *)(&rt.rt_genmask);
    EFD_T sockv4 = -1;

    memset(&rt, '\0', sizeof(rt));

    pSockDst->sin_addr.s_addr       = GetIpAddr(pDestIpAddr);
    pSockDst->sin_family            = AF_INET;
    pSockDst->sin_port                = 0;

    pSockMask->sin_addr.s_addr        = etHtonl(intIpMask);
    pSockMask->sin_family            = AF_INET;
    pSockMask->sin_port                = 0;

    pSockGateway->sin_addr.s_addr     = GetIpAddr(pNextHop);
    pSockGateway->sin_family        = AF_INET;
    pSockGateway->sin_port            = 0;

    rt.rt_dev     = pDevName;
    rt.rt_flags =  (RTF_UP|RTF_GATEWAY);

    sockv4 = OpenSocket(ESOCK_DGRAM);
    if (sockv4 == ETERROR)
    {
        printf("[error]: EtDeleteRoute: OpenSocket error\r\n");
        return -1;
    }

    if (ioctl(sockv4, SIOCDELRT, &rt) < 0)
    {
        printf("[error]: EtDeleteRoute: ioctl RTF_GATEWAY error, errno = %d -- %s\r\n",errno,strerror(errno));
    }
    else
    {
        result = 0;
    }

    rt.rt_flags =  (RTF_UP|RTF_HOST);
    if (ioctl(sockv4, SIOCDELRT, &rt) < 0)
    {
        printf("[error]: EtDeleteRoute: ioctl RTF_HOST error, errno = %d -- %s \r\n",errno,strerror(errno));
    }
    else
    {
        result = 0;
    }

    CloseSocket(sockv4);
    
    return result;

}
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
EINT32 OpenSockBroadCast(EFD_T sockv4)
{
    EINT32 opt_val = ETTRUE;
    if (setsockopt(sockv4, SOL_SOCKET, SO_BROADCAST, (char*)&opt_val, sizeof(opt_val)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       CloseSockBroadCast
  Description:    关闭IPv4 Socket允许发送广播数据
  Input:          sockv4: 套接字
  Output:         无
  Return:         ETERROR - 处理失败  ETOK - 处理成功
  Others:         1、IPv6或不存在广播形式
                   2、仅UDP协议可使用此接口
************************************************************************/
EINT32 CloseSockBroadCast(EFD_T sockv4)
{
    EINT32 opt_val = ETFALSE;
    if (setsockopt(sockv4, SOL_SOCKET, SO_BROADCAST, (char*)&opt_val, sizeof(opt_val)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       SetIPMulticastTTL
  Description:    设置Socket IPv4数据包中多播地址TTL
  Input:          sockv4:套接字句柄
                  TTL:存活时间
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPMulticastTTL(EINT32 TTL, EFD_T sockv4)
{
    if (setsockopt(sockv4, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&TTL, sizeof(TTL)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetIPMulticastTTL
  Description:    获取Socket IPv4数据包中多播地址TTL
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:hoplimit, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 GetIPMulticastTTL(EFD_T sockv4)
{
    EINT32 TTL = ETERROR;
    socklen_t len = sizeof(TTL);
    if (getsockopt(sockv4, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&TTL, &len) == 0)
        return TTL;
    else
        return ETERROR;
}

/************************************************************************
  Function:       SetIPMCInterface
  Description:    设置Socket IPv4多播外出接口(网口)
  Input:          sockv4:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPMCInterface(IfIndex If, EFD_T sockv4)
{
    EINT32 result = ETERROR;
    struct in_addr addrIf;
    memset((void*)&addrIf, 0, sizeof(addrIf));

    addrIf.s_addr = If;
    if (setsockopt(sockv4, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addrIf, sizeof(addrIf)) == 0)
        result = ETOK;

    return result;
}

/************************************************************************
  Function:       GetIPMCInterface
  Description:    获取Socket IPv4多播外出接口(网口)
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
IfIndex GetIPMCInterface(EFD_T sockv4)
{
    struct in_addr addrIf;
    socklen_t opt_len = sizeof(addrIf);
    memset((void*)&addrIf, 0, sizeof(addrIf));

    getsockopt(sockv4, IPPROTO_IP, IP_MULTICAST_IF, (char*)&addrIf, &opt_len);

    return addrIf.s_addr;
}

/************************************************************************
  Function:       SetIPUCInterface
  Description:    设置Socket IPv4单播外出接口(网口)
  Input:          sockv4:套接字句柄
                  If:外出接口
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPUCInterface(IfIndex If, EFD_T sockv4)
{
    EINT32 result = ETERROR;

#ifdef IP_UNICAST_IF
    struct in_addr addrIf;
    memset((void *)&addrIf, 0, sizeof(addrIf));

    addrIf.s_addr = If;
    if (setsockopt(sockv4, IPPROTO_IP, IP_UNICAST_IF, (char*)&addrIf, sizeof(addrIf)) == 0)
        result = ETOK;
#endif
    return result;
}

/************************************************************************
  Function:       GetIPUCInterface
  Description:    设置SocketIPv4单播外出接口(网口)
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:外出接口If, 失败:ETERROR
  Others:         无
************************************************************************/
IfIndex GetIPUCInterface(EFD_T sockv4)
{
#ifdef IP_UNICAST_IF
    struct in_addr addrIf;
    socklen_t opt_len = sizeof(addrIf);
    memset((void *)&addrIf, 0, sizeof(addrIf));

    getsockopt(sockv4, IPPROTO_IP, IP_UNICAST_IF, (char*)&addrIf, &opt_len);

    return addrIf.s_addr;
#else
    return ETERROR;
#endif
}

/************************************************************************
  Function:       SetIPIfLoop
  Description:    设置Socket IPv4是否环回
  Input:          sockv4:套接字句柄
                  IfLoop:是否环回判定值ETTRUE/ETFALSE
  Output:         无
  Return:         成功:ETOK, 失败:ETERROR
  Others:         无
************************************************************************/
EINT32 SetIPIfLoop(EUCHAR IfLoop, EFD_T sockv4)
{
    if (setsockopt(sockv4, IPPROTO_IPV6, IP_MULTICAST_LOOP, &IfLoop, sizeof(IfLoop)) != 0)
        return ETERROR;
    else
        return ETOK;
}

/************************************************************************
  Function:       GetIPIfLoop
  Description:    获取Socket IPv4是否环回
  Input:          sockv4:套接字句柄
  Output:         无
  Return:         成功:IfLoop(ETTRUE/ETFALSE), 失败:ETERROR
  Others:         无
************************************************************************/
EUCHAR GetIPIfLoop(EFD_T sockv4)
{
    EUCHAR IfLoop = ETERROR;
    socklen_t opt_len = sizeof(IfLoop);
    if (getsockopt(sockv4, IPPROTO_IPV6, IP_MULTICAST_LOOP, &IfLoop, &opt_len) != 0)
        return ETERROR;
    else
        return IfLoop;
}

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
EINT32 SetIPMultiCast(ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq mreq4;
    EUINT16 version = GetIPVersion(Addr);

    if (Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    memset((void *)&mreq4, 0, sizeof(mreq4));
    mreq4.imr_multiaddr.s_addr = inet_addr(Addr);
    if (IfAddr == 0)
        mreq4.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq4.imr_interface.s_addr = IfAddr;

    if (mreq4.imr_interface.s_addr != 0)
        if (setsockopt(sockv4, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq4, sizeof(mreq4)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 SetIPMultiCastExt(ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq_source mreq4;
    EUINT16 version = GetIPVersion(Addr);

    if (Addr == NULL || SrcAddr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    memset((void *)&mreq4, 0, sizeof(mreq4));
    mreq4.imr_multiaddr.s_addr = inet_addr(Addr);
    mreq4.imr_sourceaddr.s_addr = inet_addr(SrcAddr);
    if (IfAddr == 0)
        mreq4.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq4.imr_interface.s_addr = IfAddr;
    
    if (mreq4.imr_interface.s_addr != 0)
        if (setsockopt(sockv4, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char*)&mreq4, sizeof(mreq4)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 DropIPMultiCast(ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq mreq4;
    EUINT16 version = GetIPVersion(Addr);

    if (Addr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    memset((void *)&mreq4, 0, sizeof(mreq4));
    mreq4.imr_multiaddr.s_addr = inet_addr(Addr);
    if (IfAddr == 0)
        mreq4.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq4.imr_interface.s_addr = IfAddr;

    if (mreq4.imr_interface.s_addr != 0)
        if (setsockopt(sockv4, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq4, sizeof(mreq4)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 DropIPMultiCastExt(ECHAR* Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq_source mreq4;
    EUINT16 version = GetIPVersion(Addr);

    if (Addr == NULL || SrcAddr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    memset((void *)&mreq4, 0, sizeof(mreq4));
    mreq4.imr_multiaddr.s_addr = inet_addr(Addr);
    mreq4.imr_sourceaddr.s_addr = inet_addr(SrcAddr);
    if (IfAddr == 0)
        mreq4.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq4.imr_interface.s_addr = IfAddr;

    if (mreq4.imr_interface.s_addr != 0)
        if (setsockopt(sockv4, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&mreq4, sizeof(mreq4)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 BlockIPSource(ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq_source mreq;
    EUINT16 version = GetIPVersion(Addr);

    memset((void *)&mreq, 0, sizeof(mreq));
    if (Addr == NULL || SrcAddr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    mreq.imr_multiaddr.s_addr = inet_addr(Addr);
    mreq.imr_sourceaddr.s_addr = inet_addr(SrcAddr);
    if (IfAddr == 0)
        mreq.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq.imr_interface.s_addr = IfAddr;

        if (setsockopt(sockv4, IPPROTO_IP, IP_BLOCK_SOURCE, (char*)&mreq, sizeof(mreq)) == 0)
            result = ETOK;

    return result;
}

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
EINT32 UnblockIPSource(ECHAR * Addr, EFD_T sockv4, IfIndex IfAddr, ECHAR * SrcAddr)
{
    EINT32 result = ETERROR;
    struct ip_mreq_source mreq;
    EUINT16 version = GetIPVersion(Addr);

    memset((void*)&mreq, 0, sizeof(mreq));
    if (Addr == NULL || SrcAddr == NULL || version == (EUINT16)ETERROR || version != GetSockIPVersion(sockv4) || version != IPv4)
        return ETERROR;

    mreq.imr_multiaddr.s_addr = inet_addr(Addr);
    mreq.imr_sourceaddr.s_addr = inet_addr(SrcAddr);
    if (IfAddr == 0)
        mreq.imr_interface.s_addr = GetIPMCInterface(sockv4);
    else
        mreq.imr_interface.s_addr = IfAddr;

    if (setsockopt(sockv4, IPPROTO_IP, IP_UNBLOCK_SOURCE, (char*)&mreq, sizeof(mreq)) == 0)
        result = ETOK;

    return result;
}
