/************************************************************************
Copyright (C), 2009-2010,
File name:      socketLib.h
Author:           
Date:            
Description:    对不同操作系统的socket接口进行封装，对上层提供统一socket接口
Others:         无
  *************************************************************************/
#ifndef __ETSOCKLIB_H__
#define __ETSOCKLIB_H__

/************************************************************************
                            引用的头文件
*************************************************************************/
#include "sys.h"
#ifdef AIX
#include "../../os/ospub.h" 
#endif

/************************************************************************
                             宏/类型定义
*************************************************************************/
// #define    ESOCK_STREAM    1
// #define    ESOCK_DGRAM     2
// #define    ESOCK_RAW       3

#ifndef  SERVER_MAX_CONNECTIONS
#define  SERVER_MAX_CONNECTIONS    10    //listen允许的最大连接数
#endif
#define    MAX_OPTION_DATA_LENGTH	        38

typedef enum IP_OPTION_TYPE
{
	IP_RECORD_ROUTE_TYPE = 7,
	IP_INTERNET_TIMESTAMP_TYPE = 68,
	IP_SECURITY_TYPE = 130,
	IP_LOOSE_SOURCE_AND_RECORD_ROUTE_TYPE = 131,
	IP_STREAM_IDENTIFIER_TYPE = 136,
	IP_STRICT_SOURCE_AND_RECORD_ROUTE_TYPE = 137
} IP_OPTION_TYPE;

typedef struct IP_OPTION
{
	IP_OPTION_TYPE	        	            option_type;
	unsigned int	                    	option_data_length;
	unsigned char	                    	option_data[MAX_OPTION_DATA_LENGTH];
} IP_OPTION;

/************************************************************************
                              函数申明
*************************************************************************/
// EINT32 TCPIP_init(void);
// 
// EINT32 TCPIP_discard(void);
// 
// EINT32 CloseSocket(EUINT32 id);
// 
// EINT32 OpenSocket(EINT32 type);
// 
// EINT32 SetSocketNonblock(EUINT32 id);
// 
// EINT32 BindSocket(EUINT32 id, E_UINT16 port);
// 
// EINT32 ListenSocket(EUINT32 id);
// 
// EINT32 AcceptSocket(EUINT32 id, EUINT32 *pClientIpAddr);
// 
// EINT32 ConnectSocket(EUINT32 id, EUINT32 remoteIpAddr, E_UINT16 port);
// 
// EINT32 ReadFrame(EUINT32 id,  ECHAR buf[],  EUINT32 len, EUINT32 *pRemoteIpAddr);
// 
// EINT32 SendFrame(EUINT32 id, ECHAR buf[], EUINT32 len, EINT32 remoteIpAddr, E_UINT16 port);
// 
// EINT32 ReadStream(EUINT32 id, ECHAR buf[], EINT32 len);
// 
// EINT32 SendStream(EUINT32 id ,ECHAR buf[], EINT32 len);
// 
// EINT32 GetIpAddr(ECHAR *s);
// 
// EINT32 GetIpStr(EUINT32 ipAddr, ECHAR *s);

#endif //__ETSOCKLIB_H__
