/*
 * @Description: 
 * @Author: dive668
 * @Date: 2021-12-11 17:37:36
 * @LastEditTime: 2021-12-11 17:38:56
 */
//////////////////////////////////////////////////////////
// initsock.h�ļ�

#include <winsock2.h>
#pragma comment(lib, "WS2_32")	// ���ӵ�WS2_32.lib

class CInitSock		
{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 2)
	{
		// ��ʼ��WS2_32.dll
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer, majorVer);
		if(::WSAStartup(sockVersion, &wsaData) != 0)
		{
			exit(0);
		}
	}
	~CInitSock()
	{	
		::WSACleanup();	
	}
};