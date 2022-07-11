//最简单的方法，缓冲区拼接
//buf保存着客户端发送的数据
//echo_buf保存着带了echo:的客户端数据
ZeroMemory(echo_buf, BUF_SZIE);
strcpy_s(echo_buf,"echo:");
strcat(echo_buf,buf);


//古老的方法，移位后，数组赋值
#define MAXLINE 80
#include <iostream>
#include <cstring>
using namespace std;
void echo_change(char *buffer,int len)
{
    int i;
    for(i=len;i>=0;--i)
    {
        buffer[i+5]=buffer[i];
    }
    buffer[0]='E';
    buffer[1]='C';
    buffer[2]='H';
    buffer[3]='O';
    buffer[4]=':';
}
int main()
{
    char buffer[MAXLINE]="hello";
    echo_change(buffer,int(strlen(buffer)));
    cout<<buffer;
}
