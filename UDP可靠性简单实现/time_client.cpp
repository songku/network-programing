/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-05 19:57:16
 * @LastEditTime: 2021-12-05 20:26:02
 */
#include <iostream>
#include <WinSock2.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE sizeof(packet)
#define PORT 1666
struct timeval tv;
using namespace std;
uint32_t seq=1; //全局的包序号变量
struct packet //包装的数据包
{
	time_t time;//时间戳 
	uint32_t seq;//序列号 
    uint32_t ack;//确认序列号 
	char data[1024];//数据长度
    void init_packet()
	{
		this->time = 0;
		this->seq = 0;
		ZeroMemory(this->data, 1024); //数据初始化为空
	}
};
int main(){
    WSADATA wsaData; //初始化DLL
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET clientsock = socket(PF_INET, SOCK_DGRAM, 0); //创建套接字
    tv.tv_sec = 2; //接收超时设置为2s
    tv.tv_usec = 0; //不设置微秒
    setsockopt(clientsock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv,sizeof(timeval)); //设置recvfrom的超时时间
    sockaddr_in servAddr; //服务器地址信息
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);
    sockaddr fromAddr;    //不断获取用户输入并发送给服务器，然后接受服务器数据
    int addrLen = sizeof(fromAddr);
    while(1)
    {
        char buffer[BUF_SIZE] = {0}; //用作stdin的缓冲区
        cout<<"Input message:";
        fgets(buffer,BUF_SIZE,stdin);
        if(*buffer=='q'||*buffer=='Q') //终止发送条件
        {
            cout<<"close client";
            closesocket(clientsock);
            WSACleanup();
            return 0;
        }
        //构造请求（序号、地址、数据）
        packet* pkt_send=new packet; //发送数据包
        pkt_send->init_packet();
        pkt_send->seq=seq; //设置包序号
        strcpy(pkt_send->data,buffer); //将stdin输入到buffer的数据拷贝到构造的数据包中
        packet* pkt_recv=new packet;
        pkt_recv->init_packet();
        int strLen;
        do{
            pkt_send->time=time(NULL); //填充请求时间戳
            sendto(clientsock, (char*)pkt_send, sizeof(*pkt_send), 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
            seq++; //每发送一个数据包，全局的seq++
            cout<<"发送时间戳:"<<time(NULL)<<endl;
            cout<<"发送的pkt的seq为:"<<pkt_send->seq<<endl;
            strLen = recvfrom(clientsock, (char*)pkt_recv, BUF_SIZE, 0, &fromAddr, &addrLen);
        }while(strLen==10060); //如果超时，那么recvfrom会返回错误码为10060，则重新发送
        //序号是否正确
        if(pkt_recv->ack==pkt_send->seq) //如果序号正确，则输出数据
        {
            cout<<"接收的pkt_recv的ack为:"<<pkt_recv->ack<<endl;
            cout<<"Message form server:"<<pkt_recv->data;
            cout<<"接受的时间戳:"<<pkt_recv->time<<endl;
        }
    }
    closesocket(clientsock);
    WSACleanup();
    return 0;
}