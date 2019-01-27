#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#include <pthread.h>
#include <Windows.h>
#define MAX_THREADS 10
struct ScanData
{
    unsigned int ip;
    int port;
};
const struct ScanData NILDATA;
pthread_attr_t t_c; //线程属性
void usage();
void Init();
unsigned int getip(char *);
char *ipback(unsigned int);
void scan(unsigned int, unsigned int, unsigned int, unsigned int);
void *threadscan(void *);
void usage()
{
    printf("Usage:\n"
           "program StartIp EndIp Port [Thread](Default 10)\n"
           "Example:myscan 192.168.1.1 192.168.1.254 80\n"
           "        myscan 192.168.1.1 192.168.1.254 80 256\n");
}
void Init()
{
    WSADATA wd;
    int ret = 0;
    ret = WSAStartup(MAKEWORD(2, 2), &wd); /*1.初始化操作*/
    if (ret != 0)
    {
        printf("Init Fail");
        exit(1);
    }
    if (HIBYTE(wd.wVersion) != 2 || LOBYTE(wd.wVersion) != 2)
    {
        printf("Init Fail");
        WSACleanup();
        exit(1);
    }

    pthread_attr_init(&t_c);                                    //初始化线程属性
    pthread_attr_setdetachstate(&t_c, PTHREAD_CREATE_DETACHED); //设置线程属性
}
char *ipback(unsigned int ip) //把10进制无符号整形ip转换为点IP
{
    char *ipstr = (char *)malloc(17 * sizeof(char));
    unsigned int ip_temp_numbr = 24, ip_int_index[4];

    for (int j = 0; j < 4; j++)
    {
        ip_int_index[j] = (ip >> ip_temp_numbr) & 0xFF;
        ip_temp_numbr -= 8;
    }
    sprintf(ipstr, "%d.%d.%d.%d", ip_int_index[0], ip_int_index[1], ip_int_index[2], ip_int_index[3]);
    return ipstr;
}
unsigned int getip(char *ip) //把IP地址转换为10进制无符号整形
{
    char myip[20] = "";
    strcpy(myip, ip);
    char str_ip_index[4] = "";
    int k = 3, j = 0;
    unsigned int ip_add = 0;
    for (int i = 0; i <= strlen(myip); i++)
    {
        if (myip[i] == '.' || myip[i] == '\0')
        {
            unsigned int ip_int = atoi(str_ip_index);
            if (ip_int < 0 || ip_int > 255)
            {

                printf("!!!IP ERROR!!!   Not regular IP\n");
                exit(1);
            }
            ip_add += (ip_int * (unsigned int)(pow(256.0, k)));
            k--;
            for (int x = 0; x < 4; x++)
                str_ip_index[x] = '\0';
            j = 0;
            continue;
        }
        str_ip_index[j] = myip[i];
        j++;
    }
    return ip_add;
}
void scan(unsigned int StartIp, unsigned int EndIp, unsigned int Port, unsigned int Thread)
{
    if (StartIp > EndIp)
    {
        usage();
        printf("!!!ERROR!!!  Your StartIp is bigger than your EndIp.\n");
        exit(1);
    }
    for (int i = StartIp; i <= EndIp;)
    {
        int last = EndIp - i;
        if (last >= Thread)
        {
            last = Thread;
        }
        else if (last == 0)
            last = 1;
        struct ScanData pData[last];
        for (int k = 0; k < last; k++)
            pData[k] = NILDATA;
        pthread_t t[last];
        for (int j = 0; j < last; j++)
        {
            pData[j].ip = i;
            pData[j].port = Port;
            i++;
            if (pthread_create(&t[j], &t_c, threadscan, (void *)&pData[j]) != 0)
                printf("\nCREATE THREAD ERROR\n");
            else
                pthread_join(t[j], NULL);
            Sleep(10);
        }
        Sleep(1000);
    }
    Sleep(3000);
}
void *threadscan(void *sd)
{
    struct ScanData *pa = (struct ScanData *)sd;
    char ip[20] = "";
    sprintf(ip, "%u", pa->ip);

    SOCKET c;
    SOCKADDR_IN saddr;

    /*2.创建客户端socket*/
    c = socket(AF_INET, SOCK_STREAM, 0);
    /*3.定义要连接的服务端信息*/
    saddr.sin_addr.S_un.S_addr = inet_addr(ip);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(pa->port);
    /*4.连接服务端*/
    if (connect(c, (SOCKADDR *)&saddr, sizeof(SOCKADDR)) != -1)
    {
        printf("%-16s %d Open\n", ipback(pa->ip), pa->port);
    }
    closesocket(c);
    pthread_exit(NULL);
    return NULL;
}
int main(int argc, char **argv)
{

    Init();
    if (argc == 4) //ProgramName StartIp EndIp Port
        scan(getip(argv[1]), getip(argv[2]), atoi(argv[3]), MAX_THREADS);
    else if (argc == 5 && atoi(argv[4]) > 0) //ProgramName StartIp EndIp Port Thread
        scan(getip(argv[1]), getip(argv[2]), atoi(argv[3]), atoi(argv[4]));
    else
    {
        usage();
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}