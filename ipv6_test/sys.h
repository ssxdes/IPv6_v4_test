/******************************************************************************
Copyright(C),2009-
Author:
Date:
Description:
Others:
******************************************************************************/
#pragma once

#ifndef __ETRA_SYS_H__
#define __ETRA_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BUFF_LEN     666
#define ET_LITTLE_ENDIAN

#define CLI_IPV6_DEFAULT_PORT     6666
#define SER_IPV6_DEFAULT_PORT     8888
#define MC_IPV6_DEFAULT_PORT      6060
#define CLI_IPV6_ADDR1 "2022:da8:e001::6:6:6"
#define SER_IPV6_ADDR  "2022:da8:e000::8:8:8"
#define MC_IPV6_ADDR   "ff02::2022:66"
#define CLI_IPV6_ADDR2 "2022:da8:e002::6:6:6"
#define CLI_IPV6_ADDR3 "2022:da8:e003::6:6:6"

#define ETOK            0
#define ETERROR         -1
#define ETNORMAL        1

#define ETTRUE  1
#define ETFALSE 0

#if (defined(_WIN32) && (!defined(WIN32)))
#define WIN32
#endif

#if (defined(WIN32) && (!defined(_WIN32)))
#define _WIN32
#endif

#if (defined(_WIN64) && (!defined(WIN64)))
#define WIN64
#endif

#if (defined(WIN64) && (!defined(_WIN64)))
#define _WIN64
#endif

#if (defined(WIN64) || defined(WIN32) || defined(_WINDOWS_)) && (!defined(WINDOWS))
#define WINDOWS
#endif           //auto adding macro WINDOWS where some compilers

#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#if (__BYTE_ORDER == __LITTLE_ENDIAN) && (!defined(ET_LITTLE_ENDIAN))
#define ET_LITTLE_ENDIAN
#elif (__BYTE_ORDER == __BIG_ENDIAN) && (!defined(ET_BIG_ENDIAN))
#define ET_BIG_ENDIAN
#endif
#endif

#if (!defined(ET_BIG_ENDIAN) && !defined(ET_LITTLE_ENDIAN) && defined(WINDOWS))
#define ET_LITTLE_ENDIAN
#endif          //auto adding endian_check macro where some compilers

#ifdef WINDOWS
#pragma warning(disable:4090)  /*assign different_type pointer when volatile or const*/
#pragma warning(disable:4115)
#pragma warning(disable:4127)  /* FDSET */
#pragma warning(disable:4244)  /*data lost*/
#pragma warning(disable:4996)  /*deprecated*/
#pragma warning(disable:94)    /*(struct empty array) the size of an array must be greater than zero*/
#ifdef _WIN64
#pragma warning(disable:4267)  /*data truncated*/
#pragma warning(disable:4311)  /*pointer truncated*/
#pragma warning(disable:6001)  /*ignore retrurned value*/
//#pragma warning(disable:26451)  /*avoid overfloat when compute before turning into wider type*/
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600  //0x0502

#include <winsock2.h>  //avoid duplicate contains before (windows.h or etwindows.h)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//#include "etwindows.h"
#include <ws2tcpip.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <process.h>
#endif /*WINDOWS*/

#ifdef LINUX
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sched.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <net/if_arp.h>   
#include <arpa/inet.h>
#include <elf.h>
#include <mntent.h>  
#include <sys/statfs.h>

#define __stdcall
#endif /*LINUX*/
#ifndef IMPORT
#define IMPORT extern
#endif

#define DUMP_MSG_PROCESSED_NUM   64
#define DUMP_HIST_MSG_NUM        128
#define EOS                      '\0'
#define NONE                     (-1)    /* for times when NULL won't do */

#ifndef LOCAL
#define LOCAL static
#endif

#ifdef WINDOWS
typedef signed char             ECHAR;
typedef unsigned char           EUCHAR;
typedef ECHAR                   EINT8;
typedef EUCHAR                  EUINT8;
typedef short                   EINT16;
typedef unsigned short          EUINT16;
typedef int                     EINT32;
typedef unsigned int            EUINT32;
typedef unsigned short          EUSHORT;
typedef unsigned int            EUINT;
typedef long                    ELONG;
typedef unsigned long           EULONG;
typedef int                     ESTATUS;
typedef int                     EBOOL;
typedef __int64                 EINT64;
typedef unsigned __int64        EUINT64;
typedef unsigned long           IP_ADDRESS;
typedef EUINT64                 ELONGSTATS;
typedef int                     socklen_t;
#ifdef _WIN64
typedef unsigned long long      EADDR_T;
typedef __int64                 EFD_T;
#else
typedef unsigned long           EADDR_T;
typedef int                     EFD_T;
#endif
typedef int (*FUNCPTR) ();     /* ptr to function returning int */
typedef void (*VOIDFUNCPTR) (); /* ptr to function returning void */
typedef unsigned long int ulong_t;
typedef unsigned char uchar_t;
//#define    EWOULDBLOCK    70        /* Operation would block */
#endif /*WINDOWS*/

#if defined(LINUX)
typedef char                    ECHAR;
typedef unsigned char           EUCHAR;
typedef ECHAR                   EINT8;
typedef EUCHAR                  EUINT8;
typedef short                   EINT16;
typedef unsigned short          EUINT16;
typedef int                     EINT32;
typedef unsigned int            EUINT32;
typedef unsigned short          EUSHORT;
typedef unsigned int            EUINT;
typedef long                    ELONG;
typedef unsigned long           EULONG;
typedef unsigned long           EADDR_T;
typedef int                     ESTATUS;
typedef int                     EBOOL;
typedef long long               EINT64;
typedef unsigned long long      EUINT64;
typedef unsigned int            IP_ADDRESS;
typedef EUINT64                 ELONGSTATS;
typedef int                     EFD_T;

#ifndef __ETRA_FUNCPTR__
#define __ETRA_FUNCPTR__
typedef int (*FUNCPTR) ();     /* ptr to function returning int */
#endif

typedef void (*VOIDFUNCPTR) (); /* ptr to function returning void */
typedef unsigned long int ulong_t;
typedef unsigned char uchar_t;

#ifndef BOOL
typedef int BOOL;
#endif
#endif

typedef void * ETRWLOCK_ID;

#ifdef __cplusplus
}
#endif

#endif  /*__ETRA_SYS_H__*/

