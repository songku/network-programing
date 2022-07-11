/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-06 18:32:43
 * @LastEditTime: 2021-12-06 20:16:08
 */
#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define fixed_length 10
#define BUFFER_SIZE 80
#define PORT 1666
#define LISTENQ 80
using namespace std;

int dynamic_port=1667; //动态变化
struct client_file
{
    SOCKET clientSocket; //文件内部包含了一个SOCKET 用于和客户端进行通信
    sockaddr_in clientAddr; //用于保存客户端的socket地址
    int id; //文件块的序号
    char echo_buffer[BUFFER_SIZE]={0};
};

DWORD WINAPI AsyncServerEcho(const LPVOID arg)
{
    client_file *temp = (client_file*)arg;
    cout<<"线程"<<temp->id<<"正在服务"<<endl;
    SOCKET thread_serversocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in thread_servAddr;
    memset(&thread_servAddr, 0, sizeof(thread_servAddr));  //每个字节都用0填充
    thread_servAddr.sin_family = PF_INET;  //使用IPv4地址
    thread_servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
    thread_servAddr.sin_port = htons(dynamic_port++);  //新端口
    bind(thread_serversocket, (SOCKADDR*)&thread_servAddr, sizeof(SOCKADDR)); //创建新套接字绑定新端口
    //伪装成1666端口，发回客户端数据
    sendto(thread_serversocket,temp->echo_buffer,BUFFER_SIZE,0,(sockaddr*)&temp->clientAddr,sizeof(temp->clientAddr)); 
    int res;
    char recv_data[BUFFER_SIZE]; //接收缓冲
    sockaddr_in thread_clientAddr;  //客户端地址信息
    int nSize = sizeof(sockaddr_in);
    //后续的回射
    for(;;)
    {
        // 清空接收缓存
        memset(recv_data, 0, BUFFER_SIZE);
        int strLen = recvfrom(thread_serversocket, recv_data, BUFFER_SIZE, 0, (sockaddr*)&thread_clientAddr, &nSize);
        if(strLen==SOCKET_ERROR)
        {
            cout << WSAGetLastError() << "接收错误!" << endl;
            res = -1;
        }
        else if(strLen==0)
        {
            cout<<"连接关闭"<<endl;
        }
        // 回射接收的数据
        char echo_buff[BUFFER_SIZE];
        memset(echo_buff,0,BUFFER_SIZE);
        strcpy_s(echo_buff,"ECHO:");
        strcat(echo_buff,recv_data);
        res = sendto(thread_serversocket, echo_buff, int(strlen(echo_buff)), 0,(sockaddr*)&temp->clientAddr,sizeof(temp->clientAddr));
        if (res == SOCKET_ERROR)
        {
            cout << WSAGetLastError() << "发送错误!" << endl;
            res = -1;
        }
        else if(res==0)
        {
            cout <<"线程"<<temp->id<<"服务的客户端关闭!" << endl;
            break;
        }
        else
        {
            cout<<"线程"<<temp->id<<"发送"<<endl;
            cout << "ECHO:" << recv_data;
            //cout<<"发送实际数据成功"<<endl;
            cout<<"线程"<<temp->id<<"发送完毕"<<endl;
        }
    } while (res > 0);
    //主动关闭连接
    if (shutdown(temp->clientSocket, SD_SEND) < 0)
    {
        cout << "shutdown函数调用错误，错误码为:" << WSAGetLastError() << endl;
    }
    closesocket(temp->clientSocket);
    return -1;
}

int main(int argc, char** argv)
{
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
    SOCKADDR clientAddr;  //客户端地址信息
    int nSize = sizeof(SOCKADDR);
    //获取句柄
    char buffer[BUFFER_SIZE];  //缓冲区
    char echo_buffer[BUFFER_SIZE]; //带echo:的发送缓冲区
    HANDLE hThread[LISTENQ];
	//采用并发服务器，为连接创建线程
	for (int i=0;i<LISTENQ;++i)
	{
        sockaddr_in clientAddr;
        int nSize=sizeof(SOCKADDR);
		int res;
		res = recvfrom(serversocket,buffer,BUFFER_SIZE,0,(SOCKADDR*)&clientAddr,&nSize);
        if (res==SOCKET_ERROR)
		{
			cout << "接收失败!" << endl;
			cout << "错误码为:" << WSAGetLastError() << endl;
		}
        else if(res==0)
        {
            cout<<"连接中断"<<endl;
        }
        strcpy_s(echo_buffer,"echo:");
        strcat(echo_buffer,buffer);
        client_file *temp=new client_file;
        temp->clientSocket=serversocket;
        temp->clientAddr=clientAddr;
        temp->id=i+1;
        memset(temp->echo_buffer,0,BUFFER_SIZE);
        strcpy(temp->echo_buffer,echo_buffer);
        hThread[i]= CreateThread(NULL, 0, &AsyncServerEcho, temp, 0, NULL);  
	}
    //关闭连接
	cout << "主动关闭连接";
	WSACleanup();
	return 0;
}