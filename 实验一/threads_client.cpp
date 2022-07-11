/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 22:20:05
 * @LastEditTime: 2021-11-30 22:20:05
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
        if(iResult<fixedlen) //解决误判的数据大小，提前退出
        {
            //cout<<"接收到的字节数"<<iResult<<endl;
            return iResult;    
        }
        //cout<<"接收到的字节数"<<iResult<<endl;
        //接收缓存指针后移
        recvbuf+=iResult;
        //更新cnt
        cnt-=iResult;
    }
    return fixedlen;
}

//变长消息接收
//以结束标记分割变长消息
//用数据包中长度字段显式指明字节流的数据长度，相当于两次定长接收
int recvvl(SOCKET s,char *recvbuf,unsigned  int recvbuflen)
{
    int iResult; //存储单次recv操作的返回值
    unsigned int reclen; //用于存储报文头部存储的长度信息
    //获取接收报文长度信息
    iResult=recvn(s,(char *)&reclen,sizeof(unsigned int));
    if(iResult!=sizeof(unsigned int))
    {
        //如果长度字段在接收时没有返回一个长度数据就返回(连接关闭)或-1(发生错误)
        if(iResult==-1)
        {
            cout<<"接收错误，错误码为:"<<GetLastError()<<endl;
            return -1;
        }
        else{
            cout<<"连接关闭"<<endl;
            return 0;
        }
    }
    //转换网络字节序到主机字节序
    reclen=ntohl(reclen);
    if(reclen>recvbuflen)
    {
        //如果recvbuf没有足够的长度存储变长消息，则接收该消息并丢弃，返回错误
        while(reclen>0){
            iResult=recvn(s,recvbuf,recvbuflen);
            if(iResult!=recvbuflen)
            {
                //如果边长消息在接收时没有返回足够的数据就返回(连接关闭)或-1(发生错误)
                if(iResult==-1)
                {
                    cout<<"接收时发生错误，错误码为:"<<WSAGetLastError()<<endl;
                    return -1;
                }
                else if(iResult==0)
                {
                    cout<<"连接关闭"<<endl;
                    return 0;
                }
                else
                {
                    return 0;
                }
            }
            reclen-=recvbuflen;
            //处理最后一段数据
            if(reclen<recvbuflen)
                recvbuflen=reclen;
        }
        cout<<"可变长消息超出预分配的接收缓存"<<endl;
        return -1;
    }
    //接收可变长消息
    iResult=recvn(s,recvbuf,reclen);
    if(iResult!=reclen)
    {
        //如果边长消息在接收时没有返回足够的数据就返回(连接关闭)或-1(发生错误)
        if(iResult==-1)
        {
            cout<<"接收时发生错误，错误码为:"<<WSAGetLastError()<<endl;
            return -1;
        }
        else
        {
            cout<<"连接关闭"<<endl;
            return 0;
        }
    }
    return iResult;
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
    unsigned int length=0;
    char send_data[SendBuffer], recv_data[RecvBuffer];
    memset(send_data, 0, SendBuffer);
    memset(recv_data, 0, RecvBuffer);
    //stdin数据到send_data缓冲区
    while (fgets (send_data, SendBuffer, stdin)!=NULL)
    {
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
        // 发送长度数据
        length = (unsigned int)strlen(send_data); //注意这里强制类型转换，和server端recvn对应
        length=htonl(length);//注意这里主机字节序到网络字节序转换
        res = send(ConnectSocket,(char*)&length,sizeof(unsigned int), 0);
        if (res == SOCKET_ERROR)
        {
            cout << "发送数据失败，错误码为:" <<WSAGetLastError() << endl;
            return -1;
        }
        //发送具体数据
        res = send(ConnectSocket, send_data, (int)strlen(send_data), 0);
        if (res == SOCKET_ERROR)
        {
            cout << "发送数据失败，错误码为:" <<WSAGetLastError() << endl;
            return -1;
        }
        // 接收数据 -- 阻塞模式接收
        int rec_val;
        rec_val=recvvl(ConnectSocket,recv_data,RecvBuffer);
        if(rec_val==SOCKET_ERROR)
        {
            cout << WSAGetLastError() << "接收失败!" << endl;
            rec_val=-1;
        }
        cout<<recv_data<<endl;
        // 清空缓存
        memset(recv_data, 0, RecvBuffer);
        memset(send_data, 0, SendBuffer);
    }
	//关闭连接
	shutdown(ConnectSocket,2);
	closesocket(ConnectSocket);
	//释放dll所使用的资源
    WSACleanup();
	return 0;
}