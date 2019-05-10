

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

//�����������
enum IOType 
{
	OP_IOInitialize,
	OP_IORead,
	OP_IOWrite,
	OP_IOIdle
};

//����completion key
typedef struct _CLIENTCONTEXT
{
    SOCKET				m_Socket;
	CHAR				m_ip[32];
	UINT				m_nPort;
	//�洢��/�������ݵĻ�����
	CBuffer				m_ReadBuffer;
	CBuffer				m_WriteBuffer;
	
	//winsockȡ�����ݵĻ�����
	WSABUF				m_wsaInBuffer;
	CHAR				m_byInBuffer[BUFSIZE];    
	
	//winsock�������ݵĻ�����,���͵����ݾͷ���m_WriteBuffer
	WSABUF				m_wsaOutBuffer;
	//Ϊ�˷�ֹд���ݸ��Ƿ���,�趨���¼�
	HANDLE				m_hWriteComplete;
	
} CLIENTCONTEXT, *LPCLIENTCONTEXT;

typedef struct _OVERLAPPEDPLUS 
{
	WSAOVERLAPPED ol;
	//��������
	IOType opCode;
	
} OVERLAPPEDPLUS, *LPOVERLAPPEDPLUS;


class CIOCPServer  
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();
	
	void Accept();
	BOOL Initialize(DWORD conns, int port);

	//���ڶ˿��Ϲ����߳�
	static DWORD WINAPI CompletionWorkerThread( void * lpParam);


protected:
	//������Ϣ
	virtual void ProcessReceiveData(LPCLIENTCONTEXT lpContext, CBuffer &buffer) = 0;
	//�û��˳�,���������ж�,����TRUE:�Ѿ�������öԳ��¼�,FALSE,û�д�������˳��¼�
	//���ຯ������д
	virtual BOOL ClientExit(LPCLIENTCONTEXT lpContext) = 0 ;
	//д����
	void Send(LPCLIENTCONTEXT lpContext, CString strData);	
	
private:
	//������Ϣoverlapped������,������Ϣ
	BOOL ProcessIOMessage(IOType opCode, LPCLIENTCONTEXT lpContext , DWORD dwIoSize);
	//֪ͨ�ͻ�����ʱ��ʼ���ɹ�
	BOOL OnClientInitializing(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//֪ͨ���ͻ����ݳɹ�
	BOOL OnClientReading(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//֪ͨ�û�д���ݳɹ�
	BOOL OnClientWriting(LPCLIENTCONTEXT lpContext, DWORD dwIoSize);
	//֪ͨ�û��˳��������ж�
	void FreeClientContext(LPCLIENTCONTEXT lpContext);		
	//�ڶ˿��ϲ����߳�
	void CreateWorkerThread();
	//�ر���ɶ˿�
	void CloseCompletionPort();
	//��������overlappedplus
	LPOVERLAPPEDPLUS  AllocateOverlappedPlus(IOType ioType);
	//�������ӽ���Ŀͻ��������Ϣ
	LPCLIENTCONTEXT  AllocateContext();
	//�ͷ�overlappedplus
	void  FreeOverlappedPlus(LPOVERLAPPEDPLUS lpOlp);
private:
	//�����˿����
	HANDLE m_hKillEvent;
	//��ɶ˿ھ��
	HANDLE m_hIocp;
	//�߳���
	DWORD	m_dwThreads;
	//��������socket
	SOCKET m_sListen;
	

	
};

#endif // !defined(AFX_IOCPSERVER_H__1EC64F02_5939_46BF_B121_DEDD88DB2D1B__INCLUDED_)
