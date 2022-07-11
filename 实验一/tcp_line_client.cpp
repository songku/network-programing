/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 08:17:47
 * @LastEditTime: 2021-12-03 21:10:36
 */
#include <iostream>
#include <time.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define SendBuffer 80   
#define RecvBuffer 80
#define PORT 1666
BOOL	bConning; //连接服务器状态
using namespace std;

BOOL	RecvLine(SOCKET s, char* buf)
{
	BOOL	retVal = TRUE;			//返回值
	BOOL	bLineEnd = FALSE;		//行结束
	int		nReadLen = 0;			//读入字节数
	int		nDataLen = 0;			//数据长度
	while (!bLineEnd && bConning)	//与客户端连接 没有换行
	{
		nReadLen = recv(s, buf + nDataLen, 1, 0);//每次接收一个字节		
		//错误处理
		if (SOCKET_ERROR == nReadLen)
		{			
			retVal= FALSE;	//读数据失败
			//cout<<"SOCKET_ERROR"<<endl;
			break;			//跳出循环						
		}		
		if (0 == nReadLen)//客户端关闭
		{
			retVal = FALSE;	//读数据失败
			//cout<<"readlen=0"<<endl;
			break ;			//跳出循环			
		}
		//读入数据
		if ('\n' == *(buf + nDataLen))	//换行符
		{
			//cout<<"read:换行"<<endl;
			bLineEnd = TRUE;			//接收数据结束
		}else{
			//cout<<"readlen增加"<<nReadLen<<endl;
			nDataLen += nReadLen;		//增加数据长度
		}	
	}
	return retVal;
}

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
    else{
        bConning=TRUE;
    }
	//发送消息
	int res = 0; //保存函数返回结果
	char send_data[SendBuffer], recv_data[RecvBuffer];
	memset(send_data, 0, SendBuffer); //初始化发送缓冲区
	memset(recv_data, 0, RecvBuffer); //初始化接收缓冲区
	while (fgets (send_data, SendBuffer, stdin)!=NULL) //从stdin接收数据，放入send_data
	{
		if (*send_data == 'Q' || *send_data == 'q') //客户端主动输入Q或q终止发送
		{
			cout << "输入结束!" << endl;
			// 关闭发送
			res = shutdown(ConnectSocket, SD_SEND); //关闭连接
			if (res == SOCKET_ERROR) //关闭失败
			{
				cout <<"关闭连接错误，错误码为:"<<WSAGetLastError()<<endl;
			}
			return 0;
		}
		// 发送数据
		res = send(ConnectSocket, send_data, (int)strlen(send_data), 0);
		if (res == SOCKET_ERROR) //返回值错误
		{
			cout << "发送数据失败，错误码为:" <<WSAGetLastError() << endl;
			return -1;
		}
		// 接收数据 -- 阻塞模式接收
		if(!RecvLine(ConnectSocket, recv_data)) //接收一行数据
		{
			cout << "接收失败，错误码为:"<< WSAGetLastError()<< endl;
			res = -1;
		}
		fputs(recv_data,stdout); //将recv_data中的数据送入stdout
		cout<<endl;
		// 清空缓存
		memset(recv_data, 0, RecvBuffer); //接收缓存清0
		memset(send_data, 0, SendBuffer); //发送缓存清0
	}
	//关闭连接
	shutdown(ConnectSocket,2);
	closesocket(ConnectSocket);
	//释放dll所使用的资源
    WSACleanup();
	return 0;
}