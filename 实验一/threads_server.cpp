/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-11-30 22:20:31
 * @LastEditTime: 2021-12-04 08:52:39
 */
#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define fixed_length 10
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
        cout<<"对等方"<<inet_ntoa(addr.sin_addr)<<":"<<ntohs(addr.sin_port)<<endl;
    }
    else
    {
        cout<<"获得对等方socket信息失败,错误号为:"<<GetLastError()<<endl;
    }
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
            return fixedlen-cnt;
        }
        cout<<"接收到的字节数"<<iResult<<endl;
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
                else
                {
                    cout<<"连接关闭"<<endl;
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

struct client_file
{
    SOCKET clientSocket; //文件内部包含了一个SOCKET 用于和客户端进行通信
    sockaddr_in clientAddr; //用于保存客户端的socket地址
    int id; //文件块的序号
};

DWORD WINAPI AsyncServerEcho(const LPVOID arg)
{
    client_file *temp = (client_file*)arg; //保存传入的连接套接字参数
    cout<<"线程"<<temp->id<<"正在服务:";
    getname(temp->clientSocket);
    int res; //保存函数返回值
    unsigned int slength; //数据长度
    char recv_data[BUFFSIZE+1]; //接收缓冲区
    do
    {
        // 清空接收缓存
        memset(recv_data, 0, BUFFSIZE);
        // 接收数据
        cout<<"线程"<<temp->id<<"阻塞中，待接收"<<endl;
        res = recvvl(temp->clientSocket, recv_data,BUFFSIZE); //变长方式接收数据
        if (res > 0)
        {
            cout << recv_data << endl;
            cout<<"线程"<<temp->id<<"接收完毕"<<endl;
            // 回射接收的数据
            char echo_buff[BUFFSIZE];
            memset(echo_buff,0,BUFFSIZE);
            strcpy_s(echo_buff,"ECHO:"); //拷贝ECHO:到echo_buff
            strcat(echo_buff,recv_data); //拼接echo_buff和recv_data
            //发送长度数据
            slength=(unsigned int)strlen(echo_buff); //数据长度
            res = send(temp->clientSocket, (char*)&slength,sizeof(unsigned int), 0); //send()长度数据
            if (res == SOCKET_ERROR)
            {
                cout << WSAGetLastError() << "发送错误!" << endl;
                res = -1;
            }
            else{
                //cout<<"发送长度数据成功"<<endl;
            }
            //发送实际数据
            res = send(temp->clientSocket, echo_buff, int(strlen(echo_buff)), 0); //send()实际数据
            if (res == SOCKET_ERROR)
            {
                cout << WSAGetLastError() << "发送错误!" << endl;
                res = -1;
            }
            else
            {
                cout<<"线程"<<temp->id<<"发送"<<endl;
                cout << "ECHO:" << recv_data;
                //cout<<"发送实际数据成功"<<endl;
                cout<<"线程"<<temp->id<<"发送完毕"<<endl;
            }
        }
        else
        {
            if (res == 0)
            {
                cout <<"线程"<<temp->id<<"服务的客户端关闭!" << endl;
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
    if (shutdown(temp->clientSocket, SD_SEND) < 0)
    {
        cout << "shutdown函数调用错误，错误码为:" << WSAGetLastError() << endl;
    }
    closesocket(temp->clientSocket); //释放套接字
    return -1;
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
    //获取句柄
    HANDLE hThread[LISTENQ];
    //采用并发服务器，为连接创建线程
    for (int i=0;;++i)
    {
        client_file *temp=new client_file; //用以保存连接套接字
        sockaddr_in clientAddr;
        int nSize=sizeof(SOCKADDR);
        //accept后两个参数可以为NULL
        ClientSocket = accept(ListenSocket,(SOCKADDR*)&clientAddr,&nSize);
        if (ClientSocket < 0)
        {
            cout << "接收失败!" << endl;
            cout << "错误码为:" << WSAGetLastError() << endl;
        }
        temp->clientSocket=ClientSocket; //temp的套接字指向刚建立连接的套接字
        temp->clientAddr=clientAddr;
        temp->id=i+1; //指明temp的套接字正在被哪个线程服务
        hThread[i]= CreateThread(NULL, 0, &AsyncServerEcho, temp, 0, NULL);  
    }
    //关闭连接
	shutdown(ListenSocket, 2);
	closesocket(ListenSocket);
	cout << "主动关闭连接";
	WSACleanup();
	return 0;
}