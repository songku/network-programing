///////////////////////////////////////
// P2PServer.cpp文件


#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")	// 链接到WS2_32.lib

class CInitSock		
{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 2)
	{
		// 初始化WS2_32.dll
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
#include <windows.h>


///////////////////////////////////////////////////////////////////////
 
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
#include <stdio.h>


DWORD WINAPI IOThreadProc(LPVOID lpParam);


CInitSock theSock;

CPeerList  g_PeerList;				// 客户列表
CRITICAL_SECTION g_PeerListLock;	// 同步对客户列表的访问
SOCKET g_s;							// UDP套节字

int main()
{
	// WSADATA wsadata; //保存库版本有关信息
	// WORD wVersionRequested = MAKEWORD(2, 1); //获得Winsock库版本
	// WSAStartup(wVersionRequested, &wsadata); //初始化Winsocks dll
	// 创建套节字，绑定到本地端口
	g_s = ::WSASocket(AF_INET, 
			SOCK_DGRAM , IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if(::bind(g_s, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf(" bind() failed %d \n", ::WSAGetLastError());
		return 0;
	}

	///////////////////////////////////////////////////////
	// 下面这段代码用来显示服务器绑定的终端
	char szHost[256];
	::gethostname(szHost, 256);
	hostent *pHost = ::gethostbyname(szHost);
	in_addr addr;
	for(int i = 0; ; i++)
	{
		char *p = pHost->h_addr_list[i];
		if(p == NULL)
			break;
		
		memcpy(&addr.S_un.S_addr, p, pHost->h_length);
		printf(" bind to local address -> %s:%ld \n", ::inet_ntoa(addr), SERVER_PORT);
	}	
	
	// 开启服务	
	printf(" P2P Server starting... \n\n");


	::InitializeCriticalSection(&g_PeerListLock);
	HANDLE hThread = ::CreateThread(NULL, 0, IOThreadProc, NULL, 0, NULL);

	// 定时向客户方发送“询问”消息，删除不响应的用户
	while(TRUE)
	{
		int nRet = ::WaitForSingleObject(hThread, 15*1000);
		if(nRet == WAIT_TIMEOUT)
		{
			CP2PMessage queryMsg;
			queryMsg.nMessageType = USERACTIVEQUERY;
			DWORD dwTick = ::GetTickCount();
			for(int i=0; i<g_PeerList.m_nCurrentSize; i++)
			{
				if(dwTick - g_PeerList.m_pPeer[i].dwLastActiveTime >= 2*15*1000 + 600) 
				{
					printf(" delete a non-active user: %s \n", g_PeerList.m_pPeer[i].szUserName);
					::EnterCriticalSection(&g_PeerListLock);
					g_PeerList.DeleteAPeer(g_PeerList.m_pPeer[i].szUserName);
					::LeaveCriticalSection(&g_PeerListLock);
					// 因为删了当前遍历到的用户，所以i值就不应该加1了
					i--;
				}
				else
				{
					// 注意，地址列表中的最后一个地址是客户的公共地址，询问消息应该发向这个地址
					sockaddr_in peerAddr = { 0 };
					peerAddr.sin_family = AF_INET;
					peerAddr.sin_addr.S_un.S_addr = 
						g_PeerList.m_pPeer[i].addr[g_PeerList.m_pPeer[i].AddrNum - 1].dwIp;
					peerAddr.sin_port = 
						htons(g_PeerList.m_pPeer[i].addr[g_PeerList.m_pPeer[i].AddrNum - 1].nPort);
					
					::sendto(g_s, (char*)&queryMsg, sizeof(queryMsg), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
				}
			}	
		} 
		else
		{
			break;
		}
	}

	printf(" P2P Server shutdown. \n");
	::DeleteCriticalSection(&g_PeerListLock);
	::CloseHandle(hThread);
	::closesocket(g_s);
}


DWORD WINAPI IOThreadProc(LPVOID lpParam)
{
	char buff[MAX_PACKET_SIZE];
	CP2PMessage *pMsg = (CP2PMessage*)buff;
	sockaddr_in remoteAddr;
	int nRecv, nAddrLen = sizeof(remoteAddr);
	while(TRUE)
	{
		nRecv = ::recvfrom(g_s, buff, MAX_PACKET_SIZE, 0, (sockaddr*)&remoteAddr, &nAddrLen);
		if(nRecv == SOCKET_ERROR)
		{
			printf(" recvfrom() failed \n");
			continue;
		}
		if(nRecv < sizeof(CP2PMessage))
			continue;

		// 防止用户发送错误的用户名
		pMsg->peer.szUserName[MAX_USERNAME] = '\0'; 
		switch(pMsg->nMessageType)
		{
		case USERLOGIN:		// 有用户登陆			
			{
				// 设置用户的公共终端信息，记录用户的活动时间
				pMsg->peer.addr[pMsg->peer.AddrNum].dwIp = remoteAddr.sin_addr.S_un.S_addr;
				pMsg->peer.addr[pMsg->peer.AddrNum].nPort = ntohs(remoteAddr.sin_port);
				pMsg->peer.AddrNum ++;
				pMsg->peer.dwLastActiveTime = ::GetTickCount();
				

				// 将用户信息保存到用户列表中
				::EnterCriticalSection(&g_PeerListLock);
				BOOL bOK = g_PeerList.AddAPeer(&pMsg->peer);
				::LeaveCriticalSection(&g_PeerListLock);
				if(bOK)
				{
					// 发送确认消息，将用户的公共地址传递过去
					pMsg->nMessageType = USERLOGACK;
					::sendto(g_s, (char*)pMsg, sizeof(CP2PMessage), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));	
					
					printf(" has a user login : %s (%s:%ld) \n", 
						pMsg->peer.szUserName, ::inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
				}
			}
			break;
		case USERLOGOUT:	// 有用户登出
			{
				::EnterCriticalSection(&g_PeerListLock);
				g_PeerList.DeleteAPeer(pMsg->peer.szUserName);
				::LeaveCriticalSection(&g_PeerListLock);
				
				printf(" has a user logout : %s (%s:%ld) \n", 
					pMsg->peer.szUserName, ::inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
			}
			break;

		case GETUSERLIST:	// 有用户请求发送用户列表
			{
				printf(" sending user list information to %s (%s:%ld)... \n",
					pMsg->peer.szUserName, ::inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
				CP2PMessage peerMsg;
				peerMsg.nMessageType = GETUSERLIST;
				for(int i=0; i<g_PeerList.m_nCurrentSize; i++)
				{
					memcpy(&peerMsg.peer, &g_PeerList.m_pPeer[i], sizeof(PEER_INFO));
					::sendto(g_s, (char*)&peerMsg, sizeof(CP2PMessage), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
				}

				// 发送结束封包
				peerMsg.nMessageType = USERLISTCMP;
				::sendto(g_s, (char*)&peerMsg, sizeof(CP2PMessage), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
			}
			break;
		case P2PCONNECT:	// 有用户请求让另一个用户向它发送打洞消息
			{				
				char *pszUser = (char*)(pMsg + 1);
				printf(" %s wants to connect to %s \n", pMsg->peer.szUserName, pszUser);
			
				::EnterCriticalSection(&g_PeerListLock);
				PEER_INFO *pInfo = g_PeerList.GetAPeer(pszUser);
				::LeaveCriticalSection(&g_PeerListLock);

				if(pInfo != NULL)
				{
					remoteAddr.sin_addr.S_un.S_addr = pInfo->addr[pInfo->AddrNum -1].dwIp;
					remoteAddr.sin_port = htons(pInfo->addr[pInfo->AddrNum -1].nPort);
					
					::sendto(g_s, (char*)pMsg, 
						sizeof(CP2PMessage), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));	
				}
			}
			break;

		case USERACTIVEQUERYACK:	// 用户对“询问”消息的应答	
			{
				printf(" recv active ack message from %s (%s:%ld) \n",
					pMsg->peer.szUserName, ::inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));

				::EnterCriticalSection(&g_PeerListLock);
				PEER_INFO *pInfo = g_PeerList.GetAPeer(pMsg->peer.szUserName);
				if(pInfo != NULL)
				{
					pInfo->dwLastActiveTime = ::GetTickCount();
				}
				::LeaveCriticalSection(&g_PeerListLock);
			}
			break;
		}	
	}
	return 0;
}