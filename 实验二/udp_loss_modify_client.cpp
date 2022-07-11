/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-06 21:03:02
 * @LastEditTime: 2021-12-07 15:03:39
 */
//丢包率测试，客户端
#ifndef NETWORK2_2_COMM_H
#define NETWORK2_2_COMM_H
#endif
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <inaddr.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")
#define MAXLINE 4096  // 接收缓冲区长度
#define PORT 1666  

using namespace std;

int main() {
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
    //创建套接字
    SOCKET clientsocket = socket(PF_INET, SOCK_DGRAM, 0);
    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);
    int data_len; //发送包长度
    int times; //发送包次数
    cout<<"输入发送数据包的个数：";
    cin>>times;
    cout<<"输入包长(整数格式)：";
    cin>>data_len;
    unsigned long sleep_time=0;
    cout<<"输入客户端每次发包后睡眠时长，调整发包速率:";
    cin>>sleep_time;
    int sock_res, i; //sock_res记录返回值，i记录发包次数
    char send_data[MAXLINE];
    memset(send_data,0,MAXLINE); //以0清空
    memset(send_data, 1, data_len); //以1覆盖
    for (i = 0; i < times; ++i) { //循环，发送times个数据包
        Sleep(sleep_time); //休眠一定时间，调整发包速率
        sock_res = sendto(clientsocket, send_data, (int) strlen(send_data), 0, (SOCKADDR *)&servAddr,sizeof(servAddr));
        if (sock_res == SOCKET_ERROR) {
            cout<<"send error"<<endl;
            return -1;
        }
    }
    closesocket(clientsocket);
    WSACleanup();
    return sock_res;
}