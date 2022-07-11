/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-29 23:52:14
 * @LastEditTime: 2021-12-03 19:26:48
 */
//#pragma once
#include <iostream>
#include <time.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define RECVBUFFERLENGTH 1024
#define PORT 1666
using namespace std;

int main(int argc, char** argv)
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
	}
	//设置连接信息
	int clientAddrlen;
	sockaddr sockaddrlen;
	sockaddr_in clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(PORT);
	clientaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	clientAddrlen = sizeof(sockaddrlen);
    //发起连接
	if (connect(ConnectSocket, (sockaddr*)&clientaddr, clientAddrlen) < 0)
	{
		cout << "连接失败!" << endl;
		cout << "错误号为:" << WSAGetLastError() << endl;
	}
	//接收信息
	char time[RECVBUFFERLENGTH]; //设置接收缓冲区
	memset(time, 0, RECVBUFFERLENGTH); //以0初始化
	int iResult = 1; //记录返回值
	cout << "当前时间是:" << endl;
	//循环接收
	do 
	{
		iResult = recv(ConnectSocket, time, RECVBUFFERLENGTH, 0) ; //接收信息
		if (iResult > 0) //返回值正确，输出时间 
			cout << time;
		else //返回值不当，输出错误信息
		{
			if (iResult == 0)
				cout << "连接已断开";
			else
				cout << "recv出错";
		}
		memset(time, 0, RECVBUFFERLENGTH); //清空缓冲区，为下次接收做准备
	} while (iResult > 0);
	//关闭连接
	shutdown(ConnectSocket,2);
	closesocket(ConnectSocket);
	//释放dll所使用的资源
    WSACleanup();
	return 0;
}