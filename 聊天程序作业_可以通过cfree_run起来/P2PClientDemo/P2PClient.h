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