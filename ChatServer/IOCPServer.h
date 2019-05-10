

#if !defined(AFX_IOCPSERVER_H__1EC64F02_5939_46BF_B121_DEDD88DB2D1B__INCLUDED_)
#define AFX_IOCPSERVER_H__1EC64F02_5939_46BF_B121_DEDD88DB2D1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define BUFSIZE 1024
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include "buffer.h"

#define PRINTDEBUG(a) PrintError(#a,__FILE__,__LINE__,GetLastError())

__inline int PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
{
	LPSTR lpBuffer;
	char errbuf[256];
	DWORD numread;
	
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errnum,
		LANG_NEUTRAL,
		(LPTSTR)&lpBuffer,
		0,
		NULL );
	
	wsprintf(errbuf, "\nThe following call failed at line %d in %s:\n\n"
		"    %s\n\nReason: %s\n", lineno, filename, linedesc, lpBuffer);
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), errbuf, strlen(errbuf), &numread, FALSE );
	
	return errnum;
}

//定义操作类型
enum IOType 
{
	OP_IOInitialize,
	OP_IORead,
	OP_IOWrite,
	OP_IOIdle
};

//定义completion key
typedef struct _CLIENTCONTEXT
{
    SOCKET				m_Socket;
	CHAR				m_ip[32];
	UINT				m_nPort;
	//存储接/发送数据的缓冲区
	CBuffer				m_ReadBuffer;
	CBuffer				m_WriteBuffer;
	
	//winsock取得数据的缓冲区
	WSABUF				m_wsaInBuffer;
	CHAR				m_byInBuffer[BUFSIZE];    
	
	//winsock发送数据的缓冲区,发送得数据就放在m_WriteBuffer
	WSABUF				m_wsaOutBuffer;
	//为了防止写数据覆盖发生,设定此事件
	HANDLE				m_hWriteComplete;
	
} CLIENTCONTEXT, *LPCLIENTCONTEXT;

typedef struct _OVERLAPPEDPLUS 
{
	WSAOVERLAPPED ol;
	//操作类型
	IOType opCode;
	
} OVERLAPPEDPLUS, *LPOVERLAPPEDPLUS;


class CIOCPServer  
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();
	
	void Accept();
	BOOL Initialize(DWORD conns, int port);

	//绑定在端口上工作线程
	static DWORD WINAPI CompletionWorkerThread( void * lpParam);


protected:
	//处理消息
	virtual void ProcessReceiveData(LPCLIENTCONTEXT lpContext, CBuffer &buffer) = 0;
	//用户退出,或者连接中断,返回TRUE:已经处理过该对出事件,FALSE,没有处理过该退出事件
	//子类函数必须写
	virtual BOOL ClientExit(LPCLIENTCONTEXT lpContext) = 0 ;
	//写数据
	void Send(LPCLIENTCONTEXT lpContext, CString strData);	
	
private:
	//根据消息overlapped的类型,处理消息
	BOOL ProcessIOMessage(IOType opCode, LPCLIENTCONTEXT lpContext , DWORD dwIoSize);
	//通知客户连接时初始化成功
	BOOL OnClientInitializing(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//通知读客户数据成功
	BOOL OnClientReading(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//通知用户写数据成功
	BOOL OnClientWriting(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//通知用户退出或连接中断
	void FreeClientContext(LPCLIENTCONTEXT lpContext);		
	//在端口上产生线程
	void CreateWorkerThread();
	//关闭完成端口
	void CloseCompletionPort();
	//分配连接overlappedplus
	LPOVERLAPPEDPLUS  AllocateOverlappedPlus(IOType ioType);
	//分配连接进入的客户的相关信息
	LPCLIENTCONTEXT  AllocateContext();
	//释放overlappedplus
	void  FreeOverlappedPlus(LPOVERLAPPEDPLUS lpOlp);
private:
	//结束端口完成
	HANDLE m_hKillEvent;
	//完成端口句柄
	HANDLE m_hIocp;
	//线程数
	DWORD	m_dwThreads;
	//产生监听socket
	SOCKET m_sListen;
	

	
};

#endif // !defined(AFX_IOCPSERVER_H__1EC64F02_5939_46BF_B121_DEDD88DB2D1B__INCLUDED_)
