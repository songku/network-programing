/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 23:28:13
 * @LastEditTime: 2021-12-01 00:48:25
 */
#include <iostream>
#include <winsock2.h>
#include <string>
#include <math.h>
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BUFFSIZE 80
#define PORT 1666
#define LISTENQ 80

using namespace std;
//辅助观察套接字连接的getname函数
void getname(SOCKET sock)
{
    sockaddr_in addr;
    int addr_length=sizeof(addr);
    int32_t ret = getpeername(sock, (sockaddr *)&addr, &addr_length);
    if(ret == 0)
    {
        cout<<"获得对等方socket信息:"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<endl;
    }
    else
    {
        cout<<"获得对等方socket信息失败,错误号为:"<<GetLastError()<<endl;
    }
}

int char2int(char *buffer,int numlen)
{
    string sum;
    int i=0;
    for(i=0;i<numlen;++i)
    {
        sum+=buffer[i];
    }
    return stoi(sum);
}

int main(int argc, char** argv)
{
	WSADATA wsadata; //保存库版本有关信息
	WORD wVersionRequested = MAKEWORD(2, 1); //获得Winsock库版本
	WSAStartup(wVersionRequested,&wsadata); //初始化Winsocks dll
	int  serverAddrlen;
	SOCKET ListenSocket,ClientSocket; //监听套接字和连接套接字
	//创建套接字
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket  < 0)
	{
		cout << "ListenSocket建立失败!" << endl;
		cout << "错误码为:" << WSAGetLastError() << endl;
	}
	//绑定信息
	sockaddr sockaddrlength;
	sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);//服务器的端口号
	servaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);// inet_addr("127.0.0.1");//INADDR_ANY会报错
	serverAddrlen = sizeof(sockaddrlength);
	if (bind(ListenSocket, (sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		cout << "绑定失败!" << endl;
		cout << "错误码为:" << WSAGetLastError() << endl;
	}
	//开始监听
	if (listen(ListenSocket, LISTENQ) < 0)
	{
		cout << "监听失败!" << endl;
		cout << "错误码为:" << WSAGetLastError() << endl;
	}
	//接收请求
	//采用循环服务器，接收多个连接
	for (; ; )
	{
		//accept后两个参数可以为NULL
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket < 0)
		{
			cout << "接收失败!" << endl;
			cout << "错误码为:" << WSAGetLastError() << endl;
		}
        getname(ClientSocket);
        int res;
        char recv_data[BUFFSIZE+1];
        do
        {
            int num1=0,num2=0,sum=0;
            // 清空接收缓存
            memset(recv_data, 0, BUFFSIZE);
            // 接收第一个数
            res = recv(ClientSocket, recv_data, BUFFSIZE, 0);
            if (res > 0)
            {
                cout << recv_data;
                num1=char2int(recv_data,strlen(recv_data));
            }
            else
            {
                if (res == 0)
                {
                    cout << "客户端关闭!" << endl;
                }
                else
                {
                    cout << WSAGetLastError() << "接收失败!" << endl;
                    res = -1;
                }
                break;
            }
            // 清空接收缓存
            memset(recv_data, 0, BUFFSIZE);
            // 接收第二个数
            res = recv(ClientSocket, recv_data, BUFFSIZE, 0);
            if (res > 0)
            {
                cout << recv_data;
                // 回射接收的数据
                num2=char2int(recv_data,strlen(recv_data));
            }
            else
            {
                if (res == 0)
                {
                    cout << "客户端关闭!" << endl;
                }
                else
                {
                    cout << WSAGetLastError() << "接收失败!" << endl;
                    res = -1;
                }
                break;
            }
            // 清空接收缓存
            memset(recv_data, 0, BUFFSIZE);
            sum=num1+num2;
            cout<<"和为"<<sum<<endl;
            snprintf(recv_data, sizeof(recv_data), "%d", sum);
            int len=strlen(recv_data);
            recv_data[len]='\n';
            res = send(ClientSocket, recv_data, sizeof(recv_data), 0);
            if (res == SOCKET_ERROR)
            {
                cout << WSAGetLastError() << "发送错误!" << endl;
                res = -1;
            }
            else
            {
                cout << "发送成功:" << recv_data << endl;
            }

        } while (res > 0);
        //主动关闭连接
        if (shutdown(ClientSocket, SD_SEND) < 0)
        {
        	cout << "shutdown函数调用错误，错误码为:" << WSAGetLastError() << endl;
        }
        closesocket(ClientSocket);
	}
    //关闭连接
	shutdown(ListenSocket, 2);
	closesocket(ListenSocket);
	cout << "主动关闭连接";
	WSACleanup();
	return 0;
}