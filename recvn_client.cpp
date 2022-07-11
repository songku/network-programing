/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 11:15:00
 * @LastEditTime: 2021-11-30 20:06:05
 */
#include <iostream>
#include <time.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define SendBuffer 80
#define RecvBuffer 80
#define fixed_length 10
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
			break;			//跳出循环						
		}		
		if (0 == nReadLen)//客户端关闭
		{
			retVal = FALSE;	//读数据失败
			break ;			//跳出循环			
		}
		//读入数据
		if ('\n' == *(buf + nDataLen))	//换行符
		{
			bLineEnd = TRUE;			//接收数据结束
		}else{
			nDataLen += nReadLen;		//增加数据长度
		}	
	}
	return retVal;
}

//定长数据接收
int recvn(SOCKET s,char *recvbuf,unsigned int fixedlen)
{
    int iResult; //存储单次recv操作的返回值
    int cnt; //统计相对于固定长度，还有多少没有接收
    cnt=fixedlen;
    while(cnt>0)
    {
        iResult=recv(s,recvbuf,cnt,0);
        if(iResult<0)
        {
            //数据接收出现错误，返回失败
            cout<<"接收错误，错误码为:"<<GetLastError()<<endl;
            return -1;
        }
        if(iResult==0)
        {
            //对方关闭连接，返回已接收到的小于fixedlen的字节数
            cout<<"连接关闭"<<endl;
            return fixedlen-cnt; //返回接收到的字节
        }
        cout<<"接收到的字节数"<<iResult<<endl;
        //接收缓存指针后移
        recvbuf+=iResult;
        //更新cnt
        cnt-=iResult;
    }
    return fixedlen;
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
    int res = 0;
    char send_data[SendBuffer], recv_data[RecvBuffer];
    memset(send_data, 0, SendBuffer);
    memset(recv_data, 0, RecvBuffer);
    cout<<"输入五个字节的数据(注:包含回车，例如:1234):";
    while (fgets (send_data, SendBuffer, stdin)!=NULL)
    {
        if(strlen(send_data)>5)
        {
            cout<<"输入字节数据过长，请重新输入:";
            continue;
        }
        if (*send_data == 'Q' || *send_data == 'q')
        {
            cout << "输入结束!" << endl;
            // 关闭发送
            res = shutdown(ConnectSocket, SD_SEND);
            if (res == SOCKET_ERROR)
            {
                cout <<"关闭连接错误，错误码为:"<<WSAGetLastError()<<endl;
            }
            return 0;
        }
        // 发送数据
        res = send(ConnectSocket, send_data, (int)strlen(send_data), 0);
        if (res == SOCKET_ERROR)
        {
            cout << "发送数据失败，错误码为:" <<WSAGetLastError() << endl;
            return -1;
        }

        // 接收数据 -- 阻塞模式接收
        int rec_val;
        rec_val=recvn(ConnectSocket, recv_data,fixed_length);
        if(rec_val>0)
        {
            fputs(recv_data,stdout);
        }
		cout<<endl;
        // 清空缓存
        memset(recv_data, 0, RecvBuffer);
        memset(send_data, 0, SendBuffer);
        cout<<"输入五个字节的数据(注:包含回车)";
    }
	//关闭连接
	shutdown(ConnectSocket,2);
	closesocket(ConnectSocket);
	//释放dll所使用的资源
    WSACleanup();
	return 0;
}
