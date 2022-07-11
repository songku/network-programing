/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-07 08:57:37
 * @LastEditTime: 2021-12-07 15:44:35
 */
#include <iostream>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define PORT 1666
#define MAXLINE 4096
using namespace std;

//接收客户端请求
int handleclient(SOCKET serversocket)
{
    sockaddr_in clientAddr;  //客户端地址信息
    int nSize = sizeof(sockaddr_in);
    char recv_data[MAXLINE];  //缓冲区
    int strLen=0; //recvfrom返回值
    int count=0; //记录次数
    do{
        memset(recv_data, 0, MAXLINE); //清空接收缓冲区
        strLen = recvfrom(serversocket, recv_data, MAXLINE, 0, (SOCKADDR *) &clientAddr, &nSize);
        if (strLen >= 0) { //接收到的数据长度有效，记录数据包个数count++
            count += 1;
        } else {
            int error = WSAGetLastError();
            // 其他错误
            if (error != 10060) {
				cout<<"WSAError:"<<WSAGetLastError()<<endl;
            } else { //error==10060，超时，跳出循环统计接收到包的数量
                strLen = 0;
                break;
            }
        }
    } while (strLen >= 0);
    if (count > 0) //输出接收到的数据包数量
	{
		cout<<"共接收到的包的数量为："<<count<<endl;
	}
    return strLen;
}

int main(){
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
    int recv_buff_len;
    int len = sizeof(recv_buff_len);
    //通过getsockopt查看套接字缓冲区的默认大小
    if (getsockopt(serversocket, SOL_SOCKET, SO_RCVBUF, (char *) &recv_buff_len, &len)<0) 
	{
		cout<<"getsockopt error"<<endl;
        return -1;
    }
    cout<<"默认缓冲区大小:"<<recv_buff_len<<endl;
    //更改缓冲区大小
    int setbuff;
    cout<<"输入设置的缓冲区大小：";
    cin>>setbuff;
    recv_buff_len = setbuff;
    if (setsockopt(serversocket, SOL_SOCKET, SO_RCVBUF, (char *) &recv_buff_len, len)<0) 
    {
        cout<<"setsockopt error"<<endl;
        return -1;
    }
    cout<<"更改后的缓冲区大小："<<recv_buff_len<<endl;
    //设置接收包的超时时间
    int recvtimeout;
    cout<<"输入接受包的超时时间（ms）:";
    cin>>recvtimeout;
    int time_out = recvtimeout; 
    //设置SO_RCVTIMEO选项	
    if (setsockopt(serversocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &time_out, sizeof(time_out))<0) 
    {
        cout<<"setsockopt error"<<endl;
        return -1;
    }
    //循环服务器接收客户端请求
    int res;
    while(1)
    {
        res=handleclient(serversocket); //调用handleclient进行处理
        if(res==-1)
        {
            cout<<"客户端连接失败，等待其他客户端连接"<<endl;
        }
    }
    closesocket(serversocket);
    WSACleanup();
    return 0;
}