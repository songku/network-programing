//////////////////////////////////////////////////////////
// P2PClient.cpp文件



#include <winsock2.h>
#include <stdio.h>
////////////////////////////////////////////////////////
// p2pclient.h文件

#ifndef __P2PCLIENT_H__
#define __P2PCLIENT_H__

#include <windows.h>
#ifndef __COMM_H__
#define __COMM_H__



#define MAX_USERNAME 15
#define MAX_TRY_NUMBER 5
#define MAX_ADDR_NUMBER 5
#define MAX_PACKET_SIZE 1024
#define SERVER_PORT 4567

// 一个终端信息
struct ADDR_INFO
{
	DWORD dwIp;
	u_short nPort;
};

// 一个节点信息
struct PEER_INFO
{
	char szUserName[MAX_USERNAME];	 // 用户名
	ADDR_INFO addr[MAX_ADDR_NUMBER]; // 由节点的私有终端和公共终端组成的数组
	u_char AddrNum;					 // addr数组元素数量
	ADDR_INFO p2pAddr;				 // P2P通信时应使用的地址（客户方使用）
	DWORD dwLastActiveTime;			 // 记录此用户的活动时间（服务器使用）
};

// 通信消息格式
struct CP2PMessage
{
	int nMessageType;	// 消息类型
	PEER_INFO peer;		// 节点信息
};


// 用户直接与服务器之间发送的消息
#define USERLOGIN	101		// 用户登陆服务器
#define USERLOGOUT	102		// 用户登出服务器
#define USERLOGACK  103

#define GETUSERLIST	104		// 请求用户列表
#define USERLISTCMP	105		// 列表传输结束

#define USERACTIVEQUERY	106			// 服务器询问一个用户是否仍然存活
#define USERACTIVEQUERYACK	107		// 服务器询问应答

// 通过服务器中转，用户与用户之间发送的消息
#define P2PCONNECT	108			// 请求与一个用户建立连接
#define P2PCONNECTACK	109		// 连接应答，此消息用于打洞

// 用户直接与用户之间发送的消息
#define P2PMESSAGE		110			// 发送消息
#define P2PMESSAGEACK	111			// 收到消息的应答


class CPeerList
{
public:
	CPeerList();
	~CPeerList();
	
	// 向列表中添加一个节点
	BOOL AddAPeer(PEER_INFO *pPeer);
	// 查找指定用户名对应的节点
	PEER_INFO *GetAPeer(char *pszUserName);
	// 从列表中删除一个节点
	void DeleteAPeer(char *pszUserName);
	// 删除所有节点
	void DeleteAllPeers();

	// 表头指针和表的大小
	PEER_INFO *m_pPeer;	
	int m_nCurrentSize;

protected:
	int m_nTatolSize;	
};


#endif // __COMM_H__

CPeerList::CPeerList()
{
	m_nCurrentSize = 0;
	m_nTatolSize = 100;
	m_pPeer = new PEER_INFO[m_nTatolSize];
}

CPeerList::~CPeerList()
{
	delete[] m_pPeer;
}

BOOL CPeerList::AddAPeer(PEER_INFO *pPeer)
{
	if(GetAPeer(pPeer->szUserName) != NULL)
		return FALSE;
	// 申请空间
	if(m_nCurrentSize >= m_nTatolSize) // 已经用完？
	{
		PEER_INFO *pTmp = m_pPeer;
		m_nTatolSize = m_nTatolSize * 2;
		m_pPeer = new PEER_INFO[m_nTatolSize];
		memcpy(m_pPeer, pTmp, m_nCurrentSize);
		delete pTmp;
	}
	// 添加到表中
	memcpy(&m_pPeer[m_nCurrentSize ++], pPeer, sizeof(PEER_INFO));
	return TRUE;
}

PEER_INFO *CPeerList::GetAPeer(char *pszUserName)
{
	for(int i=0; i<m_nCurrentSize; i++)
	{
		if(stricmp(m_pPeer[i].szUserName, pszUserName) == 0)
		{
			return &m_pPeer[i];
		}
	}
	return NULL;
}

void CPeerList::DeleteAPeer(char *pszUserName)
{
	for(int i=0; i<m_nCurrentSize; i++)
	{
		if(stricmp(m_pPeer[i].szUserName, pszUserName) == 0)
		{
			memcpy(&m_pPeer[i], &m_pPeer[i+1], (m_nCurrentSize - i - 1)*sizeof(PEER_INFO));
			m_nCurrentSize --;
			break;
		}
	}
}

void CPeerList::DeleteAllPeers()
{
	m_nCurrentSize = 0;
}

class CP2PClient
{
public:
	CP2PClient();
	~CP2PClient();
	// 初始化对象的成员
	BOOL Init(USHORT usLocalPort = 0);

	// 登陆服务器，登出服务器
	BOOL Login(char *pszUserName, char *pszServerIp);
	void Logout();

	// 向服务器请求用户列表，更新用户列表记录
	BOOL GetUserList();

	// 向一个用户发送消息
	BOOL SendText(char *pszUserName, char *pszText, int nTextLen);

	// 接收到来消息的虚函数
	virtual void OnRecv(char *pszUserName, char *pszData, int nDataLen) { }

	// 用户列表
	CPeerList m_PeerList;

protected:
	void HandleIO(char *pBuf, int nBufLen, sockaddr *addr, int nAddrLen);
	static DWORD WINAPI RecvThreadProc(LPVOID lpParam);

	CRITICAL_SECTION m_PeerListLock;	// 同步对用户列表的访问

	SOCKET m_s;				// 用于P2P通信的套节字句柄
	HANDLE m_hThread;		// 线程句柄	
	WSAOVERLAPPED m_ol;		// 用于等待网络事件的重叠结构

	PEER_INFO m_LocalPeer;	// 本用户信息

	DWORD m_dwServerIp;		// 服务器IP地址

	BOOL m_bThreadExit;		// 用于指示接收线程退出

	BOOL m_bLogin;			// 是否登陆
	BOOL m_bUserlistCmp;	// 用户列表是否传输结束
	BOOL m_bMessageACK;		// 是否接收到消息应答
};


#endif // __P2PCLIENT_H__

class CMyP2P : public CP2PClient
{
public:

	void OnRecv(char *pszUserName, char *pszData, int nDataLen)
	{
		pszData[nDataLen] = '\0';
		printf(" Recv a Message from %s :  %s \n", pszUserName, pszData);
	}
};

#pragma comment(lib, "WS2_32")	// 链接到WS2_32.lib

CP2PClient::CP2PClient()
{	
	m_bLogin = FALSE;
	m_hThread = NULL;
	m_s = INVALID_SOCKET;

	memset(&m_ol, 0, sizeof(m_ol));
	m_ol.hEvent = ::WSACreateEvent();

	::InitializeCriticalSection(&m_PeerListLock);	
	
	// 初始化WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	::WSAStartup(sockVersion, &wsaData);
}

CP2PClient::~CP2PClient()
{
	Logout();
	
	// 通知接收线程退出
	if(m_hThread != NULL)
	{
		m_bThreadExit = TRUE;
		::WSASetEvent(m_ol.hEvent);
		::WaitForSingleObject(m_hThread, 300);
		::CloseHandle(m_hThread);
	}
	
	if(m_s != INVALID_SOCKET)
		::closesocket(m_s);

	::WSACloseEvent(m_ol.hEvent);

	::DeleteCriticalSection(&m_PeerListLock);
	::WSACleanup();	
}

BOOL CP2PClient::Init(USHORT usLocalPort)
{
	if(m_s != INVALID_SOCKET)
		return FALSE;

	// 创建用于P2P通信的UDP套节字，进行绑定
	m_s = ::WSASocket(AF_INET, 
			SOCK_DGRAM , IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);

	sockaddr_in localAddr = { 0 };
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(usLocalPort);
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(m_s, (LPSOCKADDR)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
	{
		::closesocket(m_s);
		m_s = INVALID_SOCKET;
		return FALSE;
	}
	if(usLocalPort == 0)
	{
		int nLen = sizeof(localAddr);
		::getsockname(m_s, (sockaddr*)&localAddr, &nLen);
		usLocalPort = ntohs(localAddr.sin_port);
	}

	// 获取本地机器的IP地址，得到当前用户的私有终端
	char szHost[256];
	::gethostname(szHost, 256);
	hostent *pHost = ::gethostbyname(szHost);

	memset(&m_LocalPeer, 0, sizeof(m_LocalPeer));
	for(int i=0; i<MAX_ADDR_NUMBER - 1; i++)
	{
		char *p = pHost->h_addr_list[i];
		if(p == NULL)
			break;

		memcpy(&m_LocalPeer.addr[i].dwIp, &p, pHost->h_length);
		m_LocalPeer.addr[i].nPort = usLocalPort;
		m_LocalPeer.AddrNum ++;
	}

	// 创建接收服务线程
	m_bThreadExit = FALSE;
	m_hThread = ::CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);

	return TRUE;
}

BOOL CP2PClient::Login(char *pszUserName, char *pszServerIp)
{
	if(m_bLogin || strlen(pszUserName) > MAX_USERNAME - 1)
		return FALSE;

	// 保存参数
	m_dwServerIp = ::inet_addr(pszServerIp);
	strncpy(m_LocalPeer.szUserName, pszUserName, strlen(pszUserName));


	// 服务器名称
	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
	serverAddr.sin_port = htons(SERVER_PORT);

	// 向服务发送本用户信息
	CP2PMessage logMsg;
	logMsg.nMessageType = USERLOGIN;
	memcpy(&logMsg.peer, &m_LocalPeer, sizeof(PEER_INFO)); 

	for(int i=0; i<MAX_TRY_NUMBER; i++)
	{
		::sendto(m_s, (char*)&logMsg, sizeof(logMsg), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		for(int j=0; j<10; j++)
		{
			if(m_bLogin)
				return TRUE;
			::Sleep(300);
		}
	}
	return FALSE;
}

void CP2PClient::Logout()
{
	if(m_bLogin)
	{
		// 告诉服务器，我们要离开了		
		CP2PMessage logMsg;
		logMsg.nMessageType = USERLOGOUT;
		memcpy(&logMsg.peer, &m_LocalPeer, sizeof(PEER_INFO)); 

		sockaddr_in serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
		serverAddr.sin_port = htons(SERVER_PORT);

		::sendto(m_s, (char*)&logMsg, sizeof(logMsg), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		m_bLogin = FALSE;
	}
}

BOOL CP2PClient::SendText(char *pszUserName, char *pszText, int nTextLen)
{
	if(!m_bLogin || strlen(pszUserName) > MAX_USERNAME - 1 
					|| nTextLen > MAX_PACKET_SIZE - sizeof(CP2PMessage))
		return FALSE;

	// 构建封包
	char sendBuf[MAX_PACKET_SIZE];
	CP2PMessage *pMsg = (CP2PMessage*)sendBuf;
	pMsg->nMessageType = P2PMESSAGE;
	memcpy(&pMsg->peer, &m_LocalPeer, sizeof(m_LocalPeer));
	memcpy((pMsg + 1), pszText, nTextLen);

	m_bMessageACK = FALSE;
	for(int i=0; i<MAX_TRY_NUMBER; i++)
	{
		PEER_INFO *pInfo = m_PeerList.GetAPeer(pszUserName);
		if(pInfo == NULL)
			return FALSE;
		
		// 如果对方P2P地址不为0，就试图以它为目的地址发送数据，
		// 如果发送失败，则认为此P2P地址无效
		if(pInfo->p2pAddr.dwIp != 0) 
		{	
			sockaddr_in peerAddr = { 0 };
			peerAddr.sin_family = AF_INET;
			peerAddr.sin_addr.S_un.S_addr = pInfo->p2pAddr.dwIp;
			peerAddr.sin_port = htons(pInfo->p2pAddr.nPort);
			
			::sendto(m_s, sendBuf, 
				nTextLen + sizeof(CP2PMessage), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
			
			for(int j=0; j<10; j++)
			{
				if( m_bMessageACK)
					return TRUE;
				::Sleep(300);
			}
		}

		// 请求打洞，并且重新设置P2P地址
		pInfo->p2pAddr.dwIp = 0;	

		// 构建封包
		char tmpBuf[sizeof(CP2PMessage) + MAX_USERNAME];
		CP2PMessage *p = (CP2PMessage *)tmpBuf;
		p->nMessageType = P2PCONNECT;
		memcpy(&p->peer, &m_LocalPeer, sizeof(m_LocalPeer));
		memcpy((char*)(p + 1), pszUserName, strlen(pszUserName) + 1);

		// 首先直接发向目标，
		sockaddr_in peerAddr = { 0 };
		peerAddr.sin_family = AF_INET;
		for(int j=0; j<pInfo->AddrNum; j++)
		{
			peerAddr.sin_addr.S_un.S_addr = pInfo->addr[j].dwIp;
			peerAddr.sin_port = htons(pInfo->addr[j].nPort);
			::sendto(m_s, tmpBuf, sizeof(CP2PMessage), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
		}
		
		// 然后通过服务器转发，请求对方向自己打洞
		sockaddr_in serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
		serverAddr.sin_port = htons(SERVER_PORT);
		::sendto(m_s, tmpBuf, 
			sizeof(CP2PMessage) + MAX_USERNAME, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

		// 等待对方的P2PCONNECTACK消息
		for(int j=0; j<10; j++)
		{
			if(pInfo->p2pAddr.dwIp != 0)
				break;
			::Sleep(300);
		}	
	}
	return 0;
}

BOOL CP2PClient::GetUserList()
{	
	// 服务器地址
	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
	serverAddr.sin_port = htons(SERVER_PORT);
	// 构建封包
	CP2PMessage msgList;
	msgList.nMessageType = GETUSERLIST;	
	memcpy(&msgList.peer, &m_LocalPeer, sizeof(m_LocalPeer));
	// 删除所有节点
	::EnterCriticalSection(&m_PeerListLock);
	m_PeerList.DeleteAllPeers();
	::LeaveCriticalSection(&m_PeerListLock);	
	
	// 发送GETUSERLIST请求，等待列表发送完成
	m_bUserlistCmp = FALSE;	
	int nUserCount = 0;
	for(int i=0; i<MAX_TRY_NUMBER; i++)
	{
		::sendto(m_s, (char*)&msgList, 
			sizeof(msgList), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
		do
		{	
			nUserCount = m_PeerList.m_nCurrentSize;
			for(int j=0; j<10; j++)
			{
				if(m_bUserlistCmp)
					return TRUE;
				::Sleep(300);
			}
		}while(m_PeerList.m_nCurrentSize > nUserCount);
	}
	return FALSE;
}

DWORD WINAPI CP2PClient::RecvThreadProc(LPVOID lpParam)
{	
	CP2PClient *pThis = (CP2PClient *)lpParam;	
	char buff[MAX_PACKET_SIZE];
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	WSABUF wsaBuf;
	wsaBuf.buf = buff;
	wsaBuf.len = MAX_PACKET_SIZE;

	// 接收处理到来的消息
	while(TRUE)
	{
		DWORD dwRecv, dwFlags = 0;
		int nRet = ::WSARecvFrom(pThis->m_s, &wsaBuf, 
				1, &dwRecv, &dwFlags, (sockaddr*)&remoteAddr, &nAddrLen, &pThis->m_ol, NULL);
		if(nRet == SOCKET_ERROR && ::WSAGetLastError() == WSA_IO_PENDING)
		{
			::WSAGetOverlappedResult(pThis->m_s, &pThis->m_ol, &dwRecv, TRUE, &dwFlags);
		}
		// 首先查看是否要退出
		if(pThis->m_bThreadExit)
			break;
		// 调用HandleIO函数来处理这个消息
		pThis->HandleIO(buff, dwRecv, (sockaddr *)&remoteAddr, nAddrLen);
	}
	return 0;
}

void CP2PClient::HandleIO(char *pBuf, int nBufLen, sockaddr *addr, int nAddrLen)
{
	CP2PMessage *pMsg = (CP2PMessage*)pBuf;
	if(nBufLen < sizeof(CP2PMessage))
		return;
	
	switch(pMsg->nMessageType)
	{
	case USERLOGACK:		// 接收到服务器发来的登陆确认
		{
			memcpy(&m_LocalPeer, &pMsg->peer, sizeof(PEER_INFO));
			m_bLogin = TRUE;
		}
		break;
	case P2PMESSAGE:		// 有一个节点向我们发送消息
		{
			int nDataLen = nBufLen - sizeof(CP2PMessage);
			if(nDataLen > 0)
			{
				// 发送确认消息
				CP2PMessage ackMsg;
				ackMsg.nMessageType = P2PMESSAGEACK;
				memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));
				::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, addr, nAddrLen);		
				
				OnRecv(pMsg->peer.szUserName, (char*)(pMsg + 1), nDataLen);
			}
		}
		break;
	case P2PMESSAGEACK:		// 收到消息的应答
		{
			m_bMessageACK = TRUE;
		}
		break;

	case P2PCONNECT:		// 一个节点请求建立P2P连接（打洞），可能是服务器发来的，也可能是其它节点发来的
		{
			CP2PMessage ackMsg;
			ackMsg.nMessageType = P2PCONNECTACK;
			memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));

			if(((sockaddr_in*)addr)->sin_addr.S_un.S_addr != m_dwServerIp)	// 节点发来的消息
			{
				::EnterCriticalSection(&m_PeerListLock);
				PEER_INFO *pInfo = m_PeerList.GetAPeer(pMsg->peer.szUserName);
				if(pInfo != NULL)
				{
					if(pInfo->p2pAddr.dwIp == 0)
					{
						pInfo->p2pAddr.dwIp = ((sockaddr_in*)addr)->sin_addr.S_un.S_addr;
						pInfo->p2pAddr.nPort = ntohs(((sockaddr_in*)addr)->sin_port);

						printf(" Set P2P address for %s -> %s:%ld \n", pInfo->szUserName, 
							::inet_ntoa(((sockaddr_in*)addr)->sin_addr), ntohs(((sockaddr_in*)addr)->sin_port));
					}
				}
				::LeaveCriticalSection(&m_PeerListLock);

				::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, addr, nAddrLen);
			}
			else	// 服务器转发的消息
			{
				// 向节点的所有终端发送打洞消息
				sockaddr_in peerAddr = { 0 };
				peerAddr.sin_family = AF_INET;
				for(int i=0; i<pMsg->peer.AddrNum; i++)
				{
					peerAddr.sin_addr.S_un.S_addr = pMsg->peer.addr[i].dwIp;
					peerAddr.sin_port = htons(pMsg->peer.addr[i].nPort);
					::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
				}
			}
		}
		break;

	case P2PCONNECTACK:		// 接收到节点的打洞消息，在这里设置它的P2P通信地址
		{
			::EnterCriticalSection(&m_PeerListLock);
			PEER_INFO *pInfo = m_PeerList.GetAPeer(pMsg->peer.szUserName);
			if(pInfo != NULL)
			{		
				if(pInfo->p2pAddr.dwIp == 0)
				{
					pInfo->p2pAddr.dwIp = ((sockaddr_in*)addr)->sin_addr.S_un.S_addr;
					pInfo->p2pAddr.nPort = ntohs(((sockaddr_in*)addr)->sin_port);

					printf(" Set P2P address for %s -> %s:%ld \n", pInfo->szUserName, 
						::inet_ntoa(((sockaddr_in*)addr)->sin_addr), ntohs(((sockaddr_in*)addr)->sin_port));
				}
		
			}
			::LeaveCriticalSection(&m_PeerListLock);	
			
		}
		break;

	case USERACTIVEQUERY:	// 服务器询问是否存活
		{
			CP2PMessage ackMsg;
			ackMsg.nMessageType = USERACTIVEQUERYACK;
			memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));
			::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, addr, nAddrLen);
		}
		break;
	case GETUSERLIST:		// 服务器发送的用户列表
		{	
			// 首先清除此用户的P2P地址，再将用户信息保存到本地用户列表中
			pMsg->peer.p2pAddr.dwIp = 0;
			::EnterCriticalSection(&m_PeerListLock);
			m_PeerList.AddAPeer(&pMsg->peer);
			::LeaveCriticalSection(&m_PeerListLock);
		}
		break;
	case USERLISTCMP:		// 用户列表传输结束
		{
			m_bUserlistCmp = TRUE;
		}
		break;
	}
}

int main()
{	
	CMyP2P client;
	if(!client.Init(0))
	{
		printf(" CP2PClient::Init() failed \n");
		return 0;
	}

	// 获取服务器IP地址和用户名
	char szServerIp[20];
	char szUserName[MAX_USERNAME];
	printf(" Please input server ip: ");
	gets(szServerIp);
	printf(" Please input your name: ");
	gets(szUserName);
	// 登陆服务器
	if(!client.Login(szUserName, szServerIp))
	{
		printf(" CP2PClient::Login() failed \n");
		return 0;
	}
	// 第一次登陆，首先更新用户列表
	client.GetUserList();
	// 将当前状态和本程序的用法输出给用户
	printf(" %s has successfully logined server \n", szUserName);
	printf("\n Commands are: \"getu\", \"send\", \"exit\" \n");
	// 循环处理用户命令
	char szCommandLine[256];
	while(TRUE)
	{
		gets(szCommandLine);
		if(strlen(szCommandLine) < 4)
			continue;

		// 解析出命令
		char szCommand[10];
		strncpy(szCommand, szCommandLine, 4);
		szCommand[4] = '\0';
		if(stricmp(szCommand, "getu") == 0)
		{
			// 获取用户列表
			if(client.GetUserList())
			{
				printf(" Have %d users logined server: \n", client.m_PeerList.m_nCurrentSize);
				for(int i=0; i<client.m_PeerList.m_nCurrentSize; i++)
				{
					PEER_INFO *pInfo = &client.m_PeerList.m_pPeer[i];
					printf(" Username: %s(%s:%ld) \n", pInfo->szUserName, 
						::inet_ntoa(*((in_addr*)&pInfo->addr[pInfo->AddrNum -1].dwIp)), pInfo->addr[pInfo->AddrNum - 1].nPort);
				}
			}
			else
			{
				printf(" Get User List Failure !\n");
			}
		}

		else if(stricmp(szCommand, "send") == 0)
		{
			// 解析出对方用户名
			char szPeer[MAX_USERNAME];
			int i;
			for(i=5;;i++)
			{
				if(szCommandLine[i] != ' ')
					szPeer[i-5] = szCommandLine[i];
				else
				{
					szPeer[i-5] = '\0';
					break;
				}	
			}
			
			// 解析出要发送的消息
			char szMsg[56];
			strcpy(szMsg, &szCommandLine[i+1]);

			// 发送消息
			if(client.SendText(szPeer, szMsg, strlen(szMsg)))
				printf(" Send OK! \n");
			else
				printf(" Send Failure! \n");
			
		}
		else if(stricmp(szCommand, "exit") == 0)
		{
			break;
		}
	}
}
