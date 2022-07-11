/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-01 08:47:21
 * @LastEditTime: 2021-12-04 09:34:31
 */
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

int char2int(char *buffer,int numlen) //将缓冲区字符数组转为字符串后再转为int
{
    string sum;
    int i=0;
    for(i=0;i<numlen;++i)
    {
        sum+=buffer[i];
    }
    return stoi(sum);
}

string cha2str(char *buffer,int buflen) //将缓冲区字符数组转为字符串
{
    string s;
    int i=0;
    for(i=0;i<buflen;++i)
    {
        s+=buffer[i];
    }
    return s;
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
        unsigned int slength;
        char recv_data[BUFFSIZE+1];
        do
        {
            // 清空接收缓存
            memset(recv_data, 0, BUFFSIZE);
            // 接收数据
            res = recv(ClientSocket, recv_data,BUFFSIZE,0); //接收用户发送过来的含有空格的两个数
            if (res > 0)
            {
                cout << recv_data << endl;
                // 处理接受的数据
                int num1=0,num2=0,sum=0;
                string s=cha2str(recv_data,strlen(recv_data)); //将接收缓冲区字符数组转为字符串
                string::size_type n;
                n=s.find(" "); //寻找分隔两个数的空格，作为界限
                num1=stoi(s.substr(0,n)); //从串中解析得到第一个加数
                cout<<"num1:"<<num1<<endl;
                num2=stoi(s.substr(n+1,s.size()-1)); //从串中解析得到第二个加数
                cout<<"num2:"<<num2<<endl;
                sum=num1+num2;
                unsigned int slength; //数据长度
                char echo_buff[BUFFSIZE]; //发送缓冲区
                sprintf(echo_buff,"%d",sum); //数字转为发送缓冲区字符数组
                //发送长度数据
                slength=(unsigned int)strlen(echo_buff);
                res = send(ClientSocket, (char*)&slength,sizeof(unsigned int), 0);
                if (res == SOCKET_ERROR)
                {
                    cout << WSAGetLastError() << "发送错误!" << endl;
                    res = -1;
                }
                else{
                    //cout<<"发送长度数据成功"<<endl;
                }
                //发送实际数据
                res = send(ClientSocket, echo_buff, int(strlen(echo_buff)), 0);
                if (res == SOCKET_ERROR)
                {
                    cout << WSAGetLastError() << "发送错误!" << endl;
                    res = -1;
                }
                else
                {
                    cout <<"发送的两数之和为:"<<echo_buff << endl;
                    cout<<endl;
                }
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