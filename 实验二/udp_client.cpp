/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-06 17:01:50
 * @LastEditTime: 2021-12-06 17:26:01
 */
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#define PORT 1666
#define BUFFER_SIZE 100
using namespace std;

int main(){
    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //创建套接字
    SOCKET clientsocket = socket(PF_INET, SOCK_DGRAM, 0);

    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);

    //不断获取用户输入并发送给服务器，然后接受服务器数据
    sockaddr fromAddr;
    int addrLen = sizeof(fromAddr);
    char buffer[BUFFER_SIZE];
    while(1){
        memset(buffer,0,BUFFER_SIZE);
        fgets(buffer,BUFFER_SIZE,stdin);
        if(*buffer=='q')
        {
            cout<<"客户端退出..."<<endl;
            closesocket(clientsocket);
            WSACleanup();
            return 0;
        }
        sendto(clientsocket, buffer, strlen(buffer), 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
        memset(buffer,0,BUFFER_SIZE);//清空缓冲区，用以接收
        int strLen = recvfrom(clientsocket, buffer, BUFFER_SIZE, 0, &fromAddr, &addrLen);
        buffer[strLen] = 0;
        fputs(buffer,stdout);
        cout<<endl;
    }
    closesocket(clientsocket);
    WSACleanup();
    return 0;
}