/************************************************************************
Copyright (C), 2009-2010,
File name:      main_client.c
Author:       
Date:          
Description:    客户端IPv6测试
Others:         无
*************************************************************************/

/************************************************************************
                            引用的头文件
*************************************************************************/
#include "sys.h"
#include "bearpub.h"
#include <errno.h>

#ifdef WINDOWS
#define pthread_t HANDLE
#define pthread_cancel(handle) ((TerminateThread(handle,0) < 0) ? (-1) : CloseHandle(handle))
#endif

int ifTestMC = 0;
char cliIPv6Addr[MAX_IPV6_ADDRSTR_LEN];

void* ThreadListenSockV6(void * sock)
{
    EFD_T sockv6 = *((EFD_T*)sock);
    char tempbuff[MAX_BUFF_LEN];
    int i = 1;
    for (memset(tempbuff, 0, sizeof(char)* MAX_BUFF_LEN); tempbuff[0] != 'Q' || tempbuff[0] != 'q'; i++)
    {
        memset(tempbuff, 0, sizeof(char) * MAX_BUFF_LEN);
        if (ifTestMC == 0)
        {
            if (ReadFrameV6(sockv6, tempbuff, MAX_BUFF_LEN, SER_IPV6_ADDR) != ETERROR)
            {
                if (tempbuff[0] == 'Q' || tempbuff[0] == 'q')
                {
                    printf("--->receive client quit request!\n");
                    break;
                }
                else
                    printf("--->ReadFrameV6 %d: %s\n", i, tempbuff);
            }
        }
        else
        {
            if (ReadFrameV6(sockv6, tempbuff, MAX_BUFF_LEN, MC_IPV6_ADDR) != ETERROR)
            {
                if (tempbuff[0] == 'Q' || tempbuff[0] == 'q')
                {
                    printf("--->ReadFrameV6 MC client quit request!\n");
                    break;
                }
                else
                    printf("--->ReadFrameV6 MC %d: %s\n", i, tempbuff);
            }
        }
    }
}

int main()
{
    if (TCPIP_init() == ETERROR)
    {
        printf("--->TCPIP_init error!\n");
        return 0;
    }
    else
        printf("--->TCPIP_init ok!\n");
    
    memset(cliIPv6Addr, 0, MAX_IPV6_ADDRSTR_LEN);
    strcpy(cliIPv6Addr, CLI_IPV6_ADDR1);

    EINT32 proto = 0;

    printf("/----------------------------\\\n"
           "|  please enter your method  |\n"
           "|    1. ESOCK_STREAM         |\n"
           "|    2. ESOCK_DGRAM          |\n"
           "|    3. MULTICAST_TEST       |\n"
           "\\----------------------------/\n");

    scanf("%d%*c", &proto);

    if (proto == 1)
        proto = ESOCK_STREAM;
    else if (proto == 2)
        proto = ESOCK_DGRAM;
    else if (proto == 3)
    {
        ifTestMC = 1;
        printf("/---------------------------------\\\n"
               "|  please enter your cli number   |\n"
               "\\---------------------------------/\n");
        scanf("%d%*c", &proto);
        switch (proto)
        {
        case 1:
            strcpy(cliIPv6Addr, CLI_IPV6_ADDR1);
            break;
        case 2:
            strcpy(cliIPv6Addr, CLI_IPV6_ADDR2);
            break;
        case 3:
            strcpy(cliIPv6Addr, CLI_IPV6_ADDR3);
            break;
        default:
            printf("no match that number!\n");
            if (TCPIP_discard() == ETERROR)
            {
                printf("--->TCPIP_discard error!\n");
                return 0;
            }
            else
                printf("--->TCPIP_discard ok!\n");
            return 0;
            break;
        }
        proto = ESOCK_DGRAM;
    }
    else 
    {
        printf("--->ERROR proto!!\n");
        return 0;
    }

    EFD_T sockv6 = ETERROR;
    sockv6 = OpenSocketV6(proto);

    if (sockv6 == ETERROR)
        return 0;
    else
        printf("--->OpenSocketV6 OK! socketid = %d\n", sockv6);

    if (proto == ESOCK_DGRAM)
    {
        if (ETERROR == OpenSockReuse(sockv6))
        {
            printf("--->OpenSockReuse error!\n");
            return 0;
        }
        else
            printf("--->OpenSockReuse ok!\n");
    }

    if (ifTestMC == 0)
    {
        if (BindSrcSocketIPv6(sockv6, cliIPv6Addr, CLI_IPV6_DEFAULT_PORT) == ETERROR)
        {
            printf("--->BindSrcSocketIPv6 error!\n");
            return 0;
        }
        else
            printf("--->BindSrcSocketIPv6 ok!\n");
    }
    else
    {
        if (BindSocketIPv6(sockv6, MC_IPV6_DEFAULT_PORT) == ETERROR)
        {
            printf("--->BindSrcSocketIPv6 MC error!\n");
            return 0;
        }
        else
            printf("--->BindSrcSocketIPv6 MC ok!\n");
    }

    switch (GetIPVersion(cliIPv6Addr))
    {
    case IPv4:
        printf("--->GetIPVersion: IPv4 %s\n", cliIPv6Addr);
        break;
    case IPv6:
        printf("--->GetIPVersion: IPv6 %s\n", cliIPv6Addr);
        break;
    default:
        printf("--->GetIPVersion: ERROR %s\n", cliIPv6Addr);
        break;
    }

    ECHAR tempAddr[MAX_IPV6_ADDRSTR_LEN];
    memset(tempAddr, 0, MAX_IPV6_ADDRSTR_LEN);
    switch (GetSockIPVersion(sockv6))
    {
    case IPv4:
        if (GetSockAddr(sockv6, tempAddr) == ETOK)
            printf("--->GetSockIPVersion: IPv4 %s\n", tempAddr);
        else
            printf("--->GetSockIPVersion: IPv4\n--->GetSockAddr ERROR\n");
        break;
    case IPv6:
        if (GetSockAddr(sockv6, tempAddr) == ETOK)
            printf("--->GetSockIPVersion: IPv6 %s\n", tempAddr);
        else
            printf("--->GetSockIPVersion: IPv6\n--->GetSockAddr ERROR\n");
        break;
    default:
        if (GetSockAddr(sockv6, tempAddr) == ETOK)
            printf("--->GetSockIPVersion: ERROR %s\n", tempAddr);
        else
            printf("--->GetSockIPVersion: ERROR\n--->GetSockAddr ERROR\n");
        break;
    }

    if (proto == ESOCK_STREAM)
    {
        if (ConnectSocketV6(sockv6, SER_IPV6_ADDR, SER_IPV6_DEFAULT_PORT) == ETERROR)
        {
            printf("--->ConnectSocketV6 error!\n");
            return 0;
        }
        else
            printf("--->ConnectSocketV6 ok!\n");
    }

    char tempbuff[MAX_BUFF_LEN];
    int i = 0, is_listen_exist = 0;
    int isformaterr = 0;
    pthread_t tidp;

    printf("\n");
    printf("/----------------------------------------------\\\n"
           "|  enter q/Q to quit                           |\n"
           "|        send:***  to  send                    |\n"
           "|        createlisten to create listen thread  |\n"
           "\\----------------------------------------------/\n");

    while (tempbuff[0] != 'Q' && tempbuff[0] != 'q')
    {
        i++;
        if (i > 1)
        {
            if (!isformaterr)
            {
                if (i % 6)
                    printf("/----------------\\\n"
                           "|  please enter  |\n"
                           "\\----------------/\n");
                else
                {
                    if (is_listen_exist)//canclelisten to cancle listen thread
                        printf("/----------------------------------------------\\\n"
                               "|  enter q/Q to quit                           |\n"
                               "|        send:***  to  send                    |\n"
                               "|        createlisten to create listen thread  |\n"
                               "\\----------------------------------------------/\n");
                    else
                        printf("/----------------------------------------------\\\n"
                               "|  enter q/Q to quit                           |\n"
                               "|        send:***  to  send                    |\n"
                               "|        canclelisten to cancle listen thread  |\n"
                               "\\----------------------------------------------/\n");
                }
            }
            else
                isformaterr = 0;
        }

        memset(tempbuff, 0, sizeof(char) * MAX_BUFF_LEN);
        scanf("%[^\n]%*c", tempbuff);

        if (tempbuff[0] == 'Q' || tempbuff[0] == 'q')
        {
            if (proto == ESOCK_STREAM)
            {
                if (ifTestMC == 0)
                {
                    if (SendFrameV6(sockv6, "q", 1, SER_IPV6_ADDR, SER_IPV6_DEFAULT_PORT) != ETERROR)
                        printf("--->SendFrameV6 %d: %s\n", i, "q");
                    else
                        printf("--->SendFrameV6 err!!\n");
                }
                else
                {
                    if (SendFrameV6(sockv6, "q", 1, MC_IPV6_ADDR, MC_IPV6_DEFAULT_PORT) != ETERROR)
                        printf("--->SendFrameV6 %d: %s\n", i, "q");
                    else
                        printf("--->SendFrameV6 err!!\n");
                }
                printf("\n");
            }
            break;
        }

        if (0 == strncmp(tempbuff, "send:", 5))
        {
            if (ifTestMC == 0)
            {
                if (SendFrameV6(sockv6, tempbuff + 5, MAX_BUFF_LEN - 5, SER_IPV6_ADDR, SER_IPV6_DEFAULT_PORT) != ETERROR)
                    printf("--->SendFrameV6 %d: %s\n", i, tempbuff + 5);
                else
                    printf("--->SendFrameV6 err!!\n");
            }
            else
            {
                if (SendFrameV6(sockv6, tempbuff + 5, MAX_BUFF_LEN - 5, MC_IPV6_ADDR, MC_IPV6_DEFAULT_PORT) != ETERROR)
                    printf("--->SendFrameV6 MC %d: %s\n", i, tempbuff + 5);
                else
                    printf("--->SendFrameV6 MC err!!\n");
            }
            printf("\n");
            continue;
        }

        if (0 == strncmp(tempbuff, "createlisten", strlen("createlisten")))
        {
            if (is_listen_exist == 0)
            {
#ifdef LINUX
                if (ifTestMC == 1)
                {
                    struct if_nameindex* if_ni, * i;
                    if_ni = if_nameindex();
                    if (if_ni == NULL)
                    {
                        printf("--->Failed to get if_nameindex\n");
                        return ETERROR;
                    }

                    printf("\n--->GetIfIndexList:\n");
                    for (i = if_ni; !(i->if_index == 0 && i->if_name == NULL); i++)
                        printf("--->%u: %s\n", i->if_index, i->if_name);
                    printf("\n");
                    i = if_ni;
                    i++;
                    if (ETERROR == SetIPv6MCInterface(i->if_index, sockv6))
                        printf("--->SetIPv6MCInterface err, name = %s, index = %d!\n", i->if_name, i->if_index);
                    else
                    {
                        printf("--->SetIPv6MCInterface ok, name = %s, index = %d!\n", i->if_name, i->if_index);
                    }

                    printf("--->GetIPv6MCInterface = %d!\n", GetIPv6MCInterface(sockv6));
                    printf("--->GetIfIndex(\"eno16777736\") = %d!\n", GetIfIndex("eno16777736"));

                    if (ETERROR == SetIPv6MultiCast(MC_IPV6_ADDR, sockv6, 0))
                        printf("--->SetIPv6MultiCast err!\n");
                    else
                        printf("--->SetIPv6MultiCast ok!\n");

                    if_freenameindex(if_ni);
                }

                if (pthread_create(&tidp, NULL, ThreadListenSockV6, (void*)&sockv6) < 0)
                    printf("--->create listenthread error\n");
                else
                {
                    is_listen_exist = 1;
                    printf("--->create listenthread ok\n");
                }
#elif defined(WINDOWS)
                if ((tidp = CreateThread(0, 0x10000, (PTHREAD_START_ROUTINE)ThreadListenSockV6, (void*)&sockv6, 0, NULL)) < 0)

                    printf("--->create listenthread error\n");
                else
                {
                    is_listen_exist = 1;
                    printf("--->create listenthread ok\n");
                }
#endif
            }
            else
                printf("--->listenthread already exist!\n--->print canclelisten to cancle listen thread\n");
            printf("\n");
            continue;
        }

        if (0 == strncmp(tempbuff, "canclelisten", strlen("canclelisten")))
        {
            if (is_listen_exist == 1)
            {
                if (ifTestMC == 1)
                {
                    if (ETERROR == DropIPv6MultiCast(MC_IPV6_ADDR, sockv6, 0))
                        printf("--->DropIPv6MultiCast err!\n");
                    else
                        printf("--->DropIPv6MultiCast ok!\n");
                }

                if (pthread_cancel(tidp) < 0)
                    printf("cancle listen thread error\n");
                else
                {
                    is_listen_exist = 0;
                    printf("--->cancle listen thread ok\n");
                }
            }
            else
                printf("--->listenthread not exist!\n--->print createlisten to create listen thread\n");
            printf("\n");
            continue;
        }
        
        i--;
        isformaterr = 1;
        printf("/------------------\\\n"
               "|  unknown format  |\n"
               "\\------------------/\n");
        printf("\n");
    }

    if (is_listen_exist == 1)
    {
        if (ifTestMC == 1)
        {
            if (ETERROR == DropIPv6MultiCast(MC_IPV6_ADDR, sockv6, 0))
                printf("--->DropIPv6MultiCast err!\n");
            else
                printf("--->DropIPv6MultiCast ok!\n");
        }

        if (pthread_cancel(tidp) < 0)
            printf("--->cancle listen thread error\n");
        else
        {
            is_listen_exist = 0;
            printf("--->cancle listen thread ok\n");
        }
    }

    if (CloseSocket(sockv6) == ETERROR)
    {
        printf("--->CloseSocket error!\n");
        return 0;
    }
    else
        printf("--->CloseSocket ok!\n");

    if (TCPIP_discard() == ETERROR)
    {
        printf("--->TCPIP_discard error!\n");
        return 0;
    }
    else
        printf("--->TCPIP_discard ok!\n");
    return 1;
}