/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-06 17:02:12
 * @LastEditTime: 2021-12-06 18:45:43
 */
#include <iostream>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define PORT 1666
#define BUFFER_SIZE 100
using namespace std;

int main(){
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);

    //创建套接字
    SOCKET serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    //绑定套接字
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;  //使用IPv4地址
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
    servAddr.sin_port = htons(PORT);  //端口
    bind(serversocket, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));

    //接收客户端请求
    SOCKADDR clientAddr;  //客户端地址信息
    int nSize = sizeof(SOCKADDR);
    char buffer[BUFFER_SIZE];  //缓冲区
    char echo_buffer[BUFFER_SIZE]; //带echo:的发送缓冲区
    while(1){ //循环服务器
        memset(buffer,0,BUFFER_SIZE);
        int strLen = recvfrom(serversocket, buffer, BUFFER_SIZE, 0, &clientAddr, &nSize);
        memset(echo_buffer,0,BUFFER_SIZE);
        strcpy_s(echo_buffer,"echo:");
        strcat(echo_buffer,buffer);
        sendto(serversocket, echo_buffer, strlen(echo_buffer), 0, &clientAddr, nSize);
    }
    closesocket(serversocket);
    WSACleanup();
    return 0;
}