/************************************************************************
Copyright (C), 2009-2010
File name:      etSocketLib.c
Author:        
Date:          
Description:    �Բ�ͬ����ϵͳ��socket IPv4�ӿڽ��з�װ�����ϲ��ṩͳһsocket�ӿ�
Others:         ��
*************************************************************************/

/************************************************************************
                            ���õ�ͷ�ļ�
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

/**************************�ⲿ��������***************************/
#ifdef VXWORKS
extern int mRouteAdd(char *pDest, char *pGate,long mask, int tos, int flags);
#endif

/************************************************************************
                               ��������
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
  Description:    ��һ��ԭʼIPv4 Socket
  Input:          protocol:���յ�IP���ݰ�����
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
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

    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0)  //��SOKCET��ַ��Ϊ������
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
  Description:    ��һ��IPv4 Socket
  Input:          type:ESOCK_STREAM:stream socket��ESOCK_DGRAM:datagram socket��
  Output:         ��
  Return:         �ɹ�:Socket ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��IPv4 Socket����Ϊ��������ʽ
  Input:          sockv4:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
************************************************************************/
EINT32 SetSocketNonblock(EFD_T sockv4)
{
#ifdef WINDOWS
    unsigned long flag = 1;            //0:������ʽ, 1:��������ʽ

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
Description:    ��IPv4 Socket����Ϊ��ɢ·��ģʽ
Input:          sockv4:Socket���, addr:ָ��·��IP��ַ�б��ָ��, num:·�ɱ���·�ɵ�ַ�ĸ���
Output:         ��
Return:         �ɹ�:ETOK, ʧ��:ETERROR
Others:         ��
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
  Description:    ����IPv4 socket��qos����
  Input:          sockv4:Socket���; buf[]: ����; qos:qos����;
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��IPv4 Socket�뱾�ض˿ڰ�
  Input:          sockv4:Socket���, port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��IPv4 Socket�뱾�ض˿ڰ�
  Input:          sockv4: Socket���
                  srcIp: ip��ַ
                  port: �˿�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4 Socket����
  Input:          sockv4:Socket���
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4�ͻ��˵�����
  Input:          sockv4:Socket���;
  Output:         pClientIpAddr: �ͻ���IP��ַ
  Return:         �ɹ�:�½���Socket��� , ʧ��:ETERROR
  Others:         ��
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
  Description:    IPv4�ͻ��˺��з�����
  Input:          sockv4:Socket���; remoteIpAddr: ������IP��ַ;port:�˿�
  Output:         ��
  Return:         �ɹ�:ETOK , ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4 UDP֡����
  Input:          sockv4: Socket���; len:�������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4 UDP֡����
  Input:          sockv4: Socket���; len:�������ĳ���
  Output:         buf[]: ����; pRemoteIpAddr: ����֡ԴIP��ַ; pRemotePort: ����֡Դ�˿�
  Return:         �ɹ�:����0�����յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4 UDP֡����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
                  remoteIpAddr: ����֡Ŀ��IP��ַ;port:�˿�
  Output:         ��
  Return:         �ɹ�: ���͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4֡���ݵĻ�����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������յ�ʵ�ʵ������ֽ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����IPv4֡����
  Input:          sockv4:Socket���; buf[]: ����; len:�������ĳ���;
  Output:         ��
  Return:         �ɹ�:����0���ɹ������͵�ʵ�������ֽ���, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��ASCII����ʽ��IPv4��ַת��������
  Input:          s:ָ��IPv4��ַASCII����ʽ��ָ��
  Output:         ��
  Return:         IP��ַ����������
  Others:         ��
************************************************************************/
EUINT32 GetIpAddr(ECHAR *s)
{
    return inet_addr(s);
}

/************************************************************************
  Function:       GetIpStr
  Description:    �����͵�IPv4��ַת����ASCII����ʽ
  Input:          ipAddr:�������͵�IPv4��ַ
  Output:         s:ָ��IPv4��ַASCII����ʽ��ָ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ʹ��recvmsg�������ݰ���ʹ�ÿ��Ի�ȡipͷ��Ŀ���ַ
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
  Description:    ����IPv4·��
  Input:          pDestIpAddr: Ŀ���ַ
                  intIpMask: Ŀ���ַ����
                  pNextHop: ��һ����ַ
  Output:         ��
  Return:
  Others:         �˽ӿ�����linux\vxworks
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
  Description:    ɾ��IPv4·��
  Input:          pDestIpAddr: Ŀ���ַ
                  intIpMask: Ŀ���ַ����
                  pNextHop: ��һ����ַ
                  pDevName: �豸����
  Output:         ��
  Return:
  Others:         �˽ӿ�����linux\vxworks
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
  Description:    ��IPv4 Socket�����͹㲥����
  Input:          sockv4: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         1��IPv6�򲻴��ڹ㲥��ʽ
                  2����UDPЭ���ʹ�ô˽ӿ�
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
  Description:    �ر�IPv4 Socket�����͹㲥����
  Input:          sockv4: �׽���
  Output:         ��
  Return:         ETERROR - ����ʧ��  ETOK - ����ɹ�
  Others:         1��IPv6�򲻴��ڹ㲥��ʽ
                   2����UDPЭ���ʹ�ô˽ӿ�
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
  Description:    ����Socket IPv4���ݰ��жಥ��ַTTL
  Input:          sockv4:�׽��־��
                  TTL:���ʱ��
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��ȡSocket IPv4���ݰ��жಥ��ַTTL
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:hoplimit, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����Socket IPv4�ಥ����ӿ�(����)
  Input:          sockv4:�׽��־��
                  If:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��ȡSocket IPv4�ಥ����ӿ�(����)
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����Socket IPv4��������ӿ�(����)
  Input:          sockv4:�׽��־��
                  If:����ӿ�
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����SocketIPv4��������ӿ�(����)
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:����ӿ�If, ʧ��:ETERROR
  Others:         ��
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
  Description:    ����Socket IPv4�Ƿ񻷻�
  Input:          sockv4:�׽��־��
                  IfLoop:�Ƿ񻷻��ж�ֵETTRUE/ETFALSE
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         ��
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
  Description:    ��ȡSocket IPv4�Ƿ񻷻�
  Input:          sockv4:�׽��־��
  Output:         ��
  Return:         �ɹ�:IfLoop(ETTRUE/ETFALSE), ʧ��:ETERROR
  Others:         ��
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
  Description:    ����Socket IPv4�ಥ��ַ(����ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
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
  Description:    ȡ��Socket IPv4�ಥ��ַ(�뿪�ಥ��)
  Input:          Addr:�ಥ��ַ
                  sockv4:�׽��־��
                  IfAddr: �����鲥ͨѶ�ĳ��ڶ˿ڵ�ַ��
                           ��Ϊ0��ͨ��GetIPMCInterface��ȡ
  Output:         ��
  Return:         �ɹ�:ETOK, ʧ��:ETERROR
  Others:         �׽������ͱ���SOCK_DGRAM��SOCK_RAW
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
