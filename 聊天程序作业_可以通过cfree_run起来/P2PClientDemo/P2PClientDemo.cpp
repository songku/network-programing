//////////////////////////////////////////////////////////
// P2PClient.cpp�ļ�



#include <winsock2.h>
#include <stdio.h>
////////////////////////////////////////////////////////
// p2pclient.h�ļ�

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

// һ���ն���Ϣ
struct ADDR_INFO
{
	DWORD dwIp;
	u_short nPort;
};

// һ���ڵ���Ϣ
struct PEER_INFO
{
	char szUserName[MAX_USERNAME];	 // �û���
	ADDR_INFO addr[MAX_ADDR_NUMBER]; // �ɽڵ��˽���ն˺͹����ն���ɵ�����
	u_char AddrNum;					 // addr����Ԫ������
	ADDR_INFO p2pAddr;				 // P2Pͨ��ʱӦʹ�õĵ�ַ���ͻ���ʹ�ã�
	DWORD dwLastActiveTime;			 // ��¼���û��Ļʱ�䣨������ʹ�ã�
};

// ͨ����Ϣ��ʽ
struct CP2PMessage
{
	int nMessageType;	// ��Ϣ����
	PEER_INFO peer;		// �ڵ���Ϣ
};


// �û�ֱ���������֮�䷢�͵���Ϣ
#define USERLOGIN	101		// �û���½������
#define USERLOGOUT	102		// �û��ǳ�������
#define USERLOGACK  103

#define GETUSERLIST	104		// �����û��б�
#define USERLISTCMP	105		// �б������

#define USERACTIVEQUERY	106			// ������ѯ��һ���û��Ƿ���Ȼ���
#define USERACTIVEQUERYACK	107		// ������ѯ��Ӧ��

// ͨ����������ת���û����û�֮�䷢�͵���Ϣ
#define P2PCONNECT	108			// ������һ���û���������
#define P2PCONNECTACK	109		// ����Ӧ�𣬴���Ϣ���ڴ�

// �û�ֱ�����û�֮�䷢�͵���Ϣ
#define P2PMESSAGE		110			// ������Ϣ
#define P2PMESSAGEACK	111			// �յ���Ϣ��Ӧ��


class CPeerList
{
public:
	CPeerList();
	~CPeerList();
	
	// ���б������һ���ڵ�
	BOOL AddAPeer(PEER_INFO *pPeer);
	// ����ָ���û�����Ӧ�Ľڵ�
	PEER_INFO *GetAPeer(char *pszUserName);
	// ���б���ɾ��һ���ڵ�
	void DeleteAPeer(char *pszUserName);
	// ɾ�����нڵ�
	void DeleteAllPeers();

	// ��ͷָ��ͱ�Ĵ�С
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
	// ����ռ�
	if(m_nCurrentSize >= m_nTatolSize) // �Ѿ����ꣿ
	{
		PEER_INFO *pTmp = m_pPeer;
		m_nTatolSize = m_nTatolSize * 2;
		m_pPeer = new PEER_INFO[m_nTatolSize];
		memcpy(m_pPeer, pTmp, m_nCurrentSize);
		delete pTmp;
	}
	// ��ӵ�����
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
	// ��ʼ������ĳ�Ա
	BOOL Init(USHORT usLocalPort = 0);

	// ��½���������ǳ�������
	BOOL Login(char *pszUserName, char *pszServerIp);
	void Logout();

	// ������������û��б������û��б��¼
	BOOL GetUserList();

	// ��һ���û�������Ϣ
	BOOL SendText(char *pszUserName, char *pszText, int nTextLen);

	// ���յ�����Ϣ���麯��
	virtual void OnRecv(char *pszUserName, char *pszData, int nDataLen) { }

	// �û��б�
	CPeerList m_PeerList;

protected:
	void HandleIO(char *pBuf, int nBufLen, sockaddr *addr, int nAddrLen);
	static DWORD WINAPI RecvThreadProc(LPVOID lpParam);

	CRITICAL_SECTION m_PeerListLock;	// ͬ�����û��б�ķ���

	SOCKET m_s;				// ����P2Pͨ�ŵ��׽��־��
	HANDLE m_hThread;		// �߳̾��	
	WSAOVERLAPPED m_ol;		// ���ڵȴ������¼����ص��ṹ

	PEER_INFO m_LocalPeer;	// ���û���Ϣ

	DWORD m_dwServerIp;		// ������IP��ַ

	BOOL m_bThreadExit;		// ����ָʾ�����߳��˳�

	BOOL m_bLogin;			// �Ƿ��½
	BOOL m_bUserlistCmp;	// �û��б��Ƿ������
	BOOL m_bMessageACK;		// �Ƿ���յ���ϢӦ��
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

#pragma comment(lib, "WS2_32")	// ���ӵ�WS2_32.lib

CP2PClient::CP2PClient()
{	
	m_bLogin = FALSE;
	m_hThread = NULL;
	m_s = INVALID_SOCKET;

	memset(&m_ol, 0, sizeof(m_ol));
	m_ol.hEvent = ::WSACreateEvent();

	::InitializeCriticalSection(&m_PeerListLock);	
	
	// ��ʼ��WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	::WSAStartup(sockVersion, &wsaData);
}

CP2PClient::~CP2PClient()
{
	Logout();
	
	// ֪ͨ�����߳��˳�
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

	// ��������P2Pͨ�ŵ�UDP�׽��֣����а�
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

	// ��ȡ���ػ�����IP��ַ���õ���ǰ�û���˽���ն�
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

	// �������շ����߳�
	m_bThreadExit = FALSE;
	m_hThread = ::CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL);

	return TRUE;
}

BOOL CP2PClient::Login(char *pszUserName, char *pszServerIp)
{
	if(m_bLogin || strlen(pszUserName) > MAX_USERNAME - 1)
		return FALSE;

	// �������
	m_dwServerIp = ::inet_addr(pszServerIp);
	strncpy(m_LocalPeer.szUserName, pszUserName, strlen(pszUserName));


	// ����������
	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
	serverAddr.sin_port = htons(SERVER_PORT);

	// ������ͱ��û���Ϣ
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
		// ���߷�����������Ҫ�뿪��		
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

	// �������
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
		
		// ����Է�P2P��ַ��Ϊ0������ͼ����ΪĿ�ĵ�ַ�������ݣ�
		// �������ʧ�ܣ�����Ϊ��P2P��ַ��Ч
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

		// ����򶴣�������������P2P��ַ
		pInfo->p2pAddr.dwIp = 0;	

		// �������
		char tmpBuf[sizeof(CP2PMessage) + MAX_USERNAME];
		CP2PMessage *p = (CP2PMessage *)tmpBuf;
		p->nMessageType = P2PCONNECT;
		memcpy(&p->peer, &m_LocalPeer, sizeof(m_LocalPeer));
		memcpy((char*)(p + 1), pszUserName, strlen(pszUserName) + 1);

		// ����ֱ�ӷ���Ŀ�꣬
		sockaddr_in peerAddr = { 0 };
		peerAddr.sin_family = AF_INET;
		for(int j=0; j<pInfo->AddrNum; j++)
		{
			peerAddr.sin_addr.S_un.S_addr = pInfo->addr[j].dwIp;
			peerAddr.sin_port = htons(pInfo->addr[j].nPort);
			::sendto(m_s, tmpBuf, sizeof(CP2PMessage), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
		}
		
		// Ȼ��ͨ��������ת��������Է����Լ���
		sockaddr_in serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
		serverAddr.sin_port = htons(SERVER_PORT);
		::sendto(m_s, tmpBuf, 
			sizeof(CP2PMessage) + MAX_USERNAME, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

		// �ȴ��Է���P2PCONNECTACK��Ϣ
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
	// ��������ַ
	sockaddr_in serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = m_dwServerIp; 
	serverAddr.sin_port = htons(SERVER_PORT);
	// �������
	CP2PMessage msgList;
	msgList.nMessageType = GETUSERLIST;	
	memcpy(&msgList.peer, &m_LocalPeer, sizeof(m_LocalPeer));
	// ɾ�����нڵ�
	::EnterCriticalSection(&m_PeerListLock);
	m_PeerList.DeleteAllPeers();
	::LeaveCriticalSection(&m_PeerListLock);	
	
	// ����GETUSERLIST���󣬵ȴ��б������
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

	// ���մ���������Ϣ
	while(TRUE)
	{
		DWORD dwRecv, dwFlags = 0;
		int nRet = ::WSARecvFrom(pThis->m_s, &wsaBuf, 
				1, &dwRecv, &dwFlags, (sockaddr*)&remoteAddr, &nAddrLen, &pThis->m_ol, NULL);
		if(nRet == SOCKET_ERROR && ::WSAGetLastError() == WSA_IO_PENDING)
		{
			::WSAGetOverlappedResult(pThis->m_s, &pThis->m_ol, &dwRecv, TRUE, &dwFlags);
		}
		// ���Ȳ鿴�Ƿ�Ҫ�˳�
		if(pThis->m_bThreadExit)
			break;
		// ����HandleIO���������������Ϣ
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
	case USERLOGACK:		// ���յ������������ĵ�½ȷ��
		{
			memcpy(&m_LocalPeer, &pMsg->peer, sizeof(PEER_INFO));
			m_bLogin = TRUE;
		}
		break;
	case P2PMESSAGE:		// ��һ���ڵ������Ƿ�����Ϣ
		{
			int nDataLen = nBufLen - sizeof(CP2PMessage);
			if(nDataLen > 0)
			{
				// ����ȷ����Ϣ
				CP2PMessage ackMsg;
				ackMsg.nMessageType = P2PMESSAGEACK;
				memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));
				::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, addr, nAddrLen);		
				
				OnRecv(pMsg->peer.szUserName, (char*)(pMsg + 1), nDataLen);
			}
		}
		break;
	case P2PMESSAGEACK:		// �յ���Ϣ��Ӧ��
		{
			m_bMessageACK = TRUE;
		}
		break;

	case P2PCONNECT:		// һ���ڵ�������P2P���ӣ��򶴣��������Ƿ����������ģ�Ҳ�����������ڵ㷢����
		{
			CP2PMessage ackMsg;
			ackMsg.nMessageType = P2PCONNECTACK;
			memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));

			if(((sockaddr_in*)addr)->sin_addr.S_un.S_addr != m_dwServerIp)	// �ڵ㷢������Ϣ
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
			else	// ������ת������Ϣ
			{
				// ��ڵ�������ն˷��ʹ���Ϣ
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

	case P2PCONNECTACK:		// ���յ��ڵ�Ĵ���Ϣ����������������P2Pͨ�ŵ�ַ
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

	case USERACTIVEQUERY:	// ������ѯ���Ƿ���
		{
			CP2PMessage ackMsg;
			ackMsg.nMessageType = USERACTIVEQUERYACK;
			memcpy(&ackMsg.peer, &m_LocalPeer, sizeof(PEER_INFO));
			::sendto(m_s, (char*)&ackMsg, sizeof(ackMsg), 0, addr, nAddrLen);
		}
		break;
	case GETUSERLIST:		// ���������͵��û��б�
		{	
			// ����������û���P2P��ַ���ٽ��û���Ϣ���浽�����û��б���
			pMsg->peer.p2pAddr.dwIp = 0;
			::EnterCriticalSection(&m_PeerListLock);
			m_PeerList.AddAPeer(&pMsg->peer);
			::LeaveCriticalSection(&m_PeerListLock);
		}
		break;
	case USERLISTCMP:		// �û��б������
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

	// ��ȡ������IP��ַ���û���
	char szServerIp[20];
	char szUserName[MAX_USERNAME];
	printf(" Please input server ip: ");
	gets(szServerIp);
	printf(" Please input your name: ");
	gets(szUserName);
	// ��½������
	if(!client.Login(szUserName, szServerIp))
	{
		printf(" CP2PClient::Login() failed \n");
		return 0;
	}
	// ��һ�ε�½�����ȸ����û��б�
	client.GetUserList();
	// ����ǰ״̬�ͱ�������÷�������û�
	printf(" %s has successfully logined server \n", szUserName);
	printf("\n Commands are: \"getu\", \"send\", \"exit\" \n");
	// ѭ�������û�����
	char szCommandLine[256];
	while(TRUE)
	{
		gets(szCommandLine);
		if(strlen(szCommandLine) < 4)
			continue;

		// ����������
		char szCommand[10];
		strncpy(szCommand, szCommandLine, 4);
		szCommand[4] = '\0';
		if(stricmp(szCommand, "getu") == 0)
		{
			// ��ȡ�û��б�
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
			// �������Է��û���
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
			
			// ������Ҫ���͵���Ϣ
			char szMsg[56];
			strcpy(szMsg, &szCommandLine[i+1]);

			// ������Ϣ
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
