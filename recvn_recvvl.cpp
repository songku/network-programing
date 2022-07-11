#define MAXLINE 80
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#include <cstring>
#include <winsock2.h>
using namespace std;

//定长数据接收
int recvn(SOCKET s,char *recvbuf,unsigned int fixedlen)
{
    int iResult; //存储单次recv操作的返回值
    int cnt;
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
        //下面这个if判断是自己改进的
        if(iResult<fixed_length)
        {
            cout<<"最后一次接收到的字节数"<<iResult<<endl;
            //接收缓存指针后移
            recvbuf+=iResult;
            return iResult;
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
//以结束标记分割边长消息
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
