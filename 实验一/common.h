/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 08:41:30
 * @LastEditTime: 2021-11-30 08:42:32
 */
#pragma once
#include <iostream>
#include <time.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define SendBuffer 1024   
#define RecvBuffer 1024
#define PORT 1666
using namespace std;
int startup()
{
    WSADATA wsadata; //保存库版本有关信息
	WORD wVersionRequested = MAKEWORD(2, 1); //获得Winsock库版本
	WSAStartup(wVersionRequested, &wsadata); //初始化Winsocks dll
	SOCKET ConnectSocket  = INVALID_SOCKET; //连接套接字
	//初始化ConnectSocket
	ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ConnectSocket  < 0)
	{
		cout << "Socket建立失败!" << endl;
		cout << "错误码为:" <<WSAGetLastError() << endl;
        return -1;
	}
    return 1;
}