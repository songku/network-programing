/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-05 19:29:33
 * @LastEditTime: 2021-12-05 20:16:41
 */
#include <iostream>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define PORT 1666
#define BUF_SIZE sizeof(packet)
using namespace std;

struct packet
{
	time_t time;//时间戳 
	uint32_t seq;//序列号
    uint32_t ack;//确认序列号  
	char data[1024];//数据长度
    void init_packet()
	{
		this->time = 0;
		this->seq = 0;
		ZeroMemory(this->data, 1024);
	}
};

int main(){
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);

    //创建套接字
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    //绑定套接字
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;  //使用IPv4地址
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
    servAddr.sin_port = htons(PORT);  //端口
    bind(sock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));

    //接收客户端请求
    SOCKADDR clntAddr;  //客户端地址信息
    int nSize = sizeof(SOCKADDR);
    char buffer[BUF_SIZE];  //缓冲区
    packet* pkt=new packet;
    pkt->init_packet();
    while(1){
        packet* pkt=new packet;
        pkt->init_packet();
        int strLen = recvfrom(sock, (char*)pkt, BUF_SIZE, 0, &clntAddr, &nSize);
        pkt->ack=pkt->seq; //对该seq的ack
        //Sleep(500); //服务器端休眠，会导致客户端超时，继续构造请求，发送请求
        sendto(sock, (char*)pkt, BUF_SIZE, 0, &clntAddr, nSize);
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}