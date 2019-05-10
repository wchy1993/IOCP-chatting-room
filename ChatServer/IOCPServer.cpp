

#include "stdafx.h"
#include "IOCPServer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPServer::CIOCPServer()
{
	//socket��ʼ��
	WSADATA wsd;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int nResult = WSAStartup(wVersionRequested, &wsd);
	if (nResult == SOCKET_ERROR)
	{
		WSACleanup();
		PRINTDEBUG(WSAGetLastError());
	}
	
	if (LOBYTE(wsd.wVersion) != 2 || HIBYTE(wsd.wVersion) != 2)
	{
		WSACleanup();
	}
	
	m_hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	printf("socket init successful... \n");
	


}

CIOCPServer::~CIOCPServer()
{
	WSACleanup();
}


//��������overlappedplus
LPOVERLAPPEDPLUS  CIOCPServer::AllocateOverlappedPlus(IOType ioType)
{
	OVERLAPPEDPLUS* pOlp = NULL;
	
	pOlp = new OVERLAPPEDPLUS;
	ZeroMemory(pOlp, sizeof(OVERLAPPEDPLUS));
	pOlp->opCode = ioType;
	
	return pOlp;
	
}

//�������ӽ���Ŀͻ��������Ϣ
LPCLIENTCONTEXT  CIOCPServer::AllocateContext()
{
	LPCLIENTCONTEXT lpContext = NULL;
	
	lpContext = new CLIENTCONTEXT;
	ZeroMemory(lpContext, sizeof(CLIENTCONTEXT));
	lpContext->m_wsaInBuffer.buf = lpContext->m_byInBuffer;
	lpContext->m_wsaInBuffer.len = BUFSIZE;

	lpContext->m_hWriteComplete = CreateEvent(NULL, FALSE, TRUE, NULL);

	return lpContext;
	
}

//�ͷ�overlappedplus
void  CIOCPServer::FreeOverlappedPlus(LPOVERLAPPEDPLUS lpOlp)
{
	delete lpOlp;	
}


//������Ϣoverlapped������,������Ϣ,����ֵTRUE:������,FALSE,����
//һ��д�¼��Ͳ�����������FALSE,û�б�Ҫ�ٶ���!
BOOL CIOCPServer::ProcessIOMessage(IOType opCode, LPCLIENTCONTEXT lpContext , DWORD dwIoSize)
{
	BOOL bRet = FALSE;
	
	//����opCodeȷ������
	switch (opCode)
	{
	case OP_IOInitialize:
		bRet = OnClientInitializing(lpContext, dwIoSize);
		break;
	case OP_IORead:
		bRet = OnClientReading(lpContext, dwIoSize);
		break;
	case OP_IOWrite:
		bRet = OnClientWriting(lpContext, dwIoSize);
		break;
	default:
		printf("worker thread:unknown operation...\n");
	}

	return bRet; 
}

//�ر���ɶ˿�
void CIOCPServer::CloseCompletionPort( )
{
	PostQueuedCompletionStatus(m_hIocp, 0, (DWORD) NULL, NULL);
	
	// Close the CompletionPort and stop any more requests
	CloseHandle(m_hIocp);
	
}


void CIOCPServer::CreateWorkerThread()
{
	SYSTEM_INFO sysinfo;
	DWORD dwThreadId;
	
	//��completion port�ϵȴ����߳���Ϊ:CPU*2+2
	GetSystemInfo(&sysinfo);
	m_dwThreads  = sysinfo.dwNumberOfProcessors*2+2;
	
	printf("create worker thread num: %d \n", m_dwThreads);
	for(UINT i=0;i<m_dwThreads;i++)
	{
		HANDLE hThread;
		hThread = CreateThread(NULL, 
			0,
			CompletionWorkerThread, 
			(LPVOID)this, 
			0, 
			&dwThreadId);
		CloseHandle(hThread);
	}
}

//���ڶ˿��Ϲ����߳�
DWORD WINAPI CIOCPServer::CompletionWorkerThread( void * lpParam)
{
	CIOCPServer *pIocpServer = (CIOCPServer *)lpParam;
	
	DWORD dwNumRead;
	LPCLIENTCONTEXT lpContext;
	LPWSAOVERLAPPED lpOverlapped;
	LPOVERLAPPEDPLUS lpOlp;
	
	while (TRUE)
	{
		BOOL bError = FALSE;
		BOOL bEnterRead = TRUE;
		
		BOOL bResult = GetQueuedCompletionStatus(pIocpServer->m_hIocp, 
			&dwNumRead,
			(LPDWORD)&lpContext,
			&lpOverlapped,
			INFINITE);
		
		//���LPOVERLAPPEDPLUSָ��
		lpOlp = CONTAINING_RECORD(lpOverlapped,	OVERLAPPEDPLUS, ol);
		printf("Event comming %d\n", lpOlp->opCode);
		
		//��timeout����Ĵ���, �����Ϣû�д�GetQueuedCompletionStatus�з���
        if (!bResult && lpOlp == NULL && WAIT_TIMEOUT != WSAGetLastError())
		{
			PRINTDEBUG(WSAGetLastError());
			// ��������
			bError = TRUE;
		}
		//����,���������Ϣ��GetQueuedCompletionStatus�з���
		//����ԭ��֮һ��:�ͻ�ǿ���˳���!
		else if(!bResult && lpOlp != NULL)
		{
			PRINTDEBUG(WSAGetLastError());
			pIocpServer->FreeClientContext(lpContext);
			//ѭ������,��Ӧ�ö���!
			continue;
		}

		//�޴���,�����¼�
		if (!bError) 
		{
			if(bResult && NULL != lpOlp && NULL != lpContext) 
			{
				bEnterRead = pIocpServer->ProcessIOMessage(lpOlp->opCode, lpContext, dwNumRead);
			}
		}
		
		//�޴� && ��
		if(! bError && bEnterRead) 
		{
			LPOVERLAPPEDPLUS lpOlp = pIocpServer->AllocateOverlappedPlus(OP_IORead);
			ULONG			ulFlags = MSG_PARTIAL;
			
			ZeroMemory(lpContext->m_wsaInBuffer.buf, lpContext->m_wsaInBuffer.len);
			UINT nRetVal = WSARecv(lpContext->m_Socket, 
				&lpContext->m_wsaInBuffer,
				1,
				&dwNumRead, 
				&ulFlags,
				&lpOlp->ol, 
				NULL);

			if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
			{
				printf("CLIENT abortive exit\n");
				pIocpServer->FreeClientContext(lpContext);
			}

		}		
		
		pIocpServer->FreeOverlappedPlus(lpOlp);
		
	}
	return 0;
}

BOOL CIOCPServer::Initialize(DWORD conns, int port)
{
	m_sListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(m_sListen == SOCKET_ERROR)
	{
		PRINTDEBUG(WSAGetLastError());
		return FALSE;
	}
	
	printf("create listening socket successful... \n");
	
	//��Ҫ�󶨵ı��ص�ַ
	sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_addr.s_addr = htonl(INADDR_ANY);	
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	
	//��,�������˿ڰ󶨵����ص�ַ
	if(bind(m_sListen, (sockaddr*)&local,sizeof(local))
		== SOCKET_ERROR)
	{
		PRINTDEBUG(WSAGetLastError());
		return FALSE;
	}
	
	printf("bind local socket successful... \n");
	
	//������ɶ˿�
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(m_hIocp == NULL)
	{
		PRINTDEBUG(WSAGetLastError());
		return FALSE;
	}
	
	printf("create completion port successful... \n");

	CreateWorkerThread();
	printf("listening socket... \n");
	//�����߳�
	listen(m_sListen, conns);

	return TRUE;
}

void CIOCPServer::Accept()
{
	sockaddr_in client;
	int sClientLength = sizeof(client);
	while (WAIT_TIMEOUT == WaitForSingleObject(m_hKillEvent, 0))
	{
		printf("waiting for client connecting... \n");
		
		//�ȴ��ͻ�����
		SOCKET clientSocket = accept(m_sListen, (sockaddr*)&client, &sClientLength);
		if(clientSocket == SOCKET_ERROR)
		{
			PRINTDEBUG(WSAGetLastError());	
			continue;
		}
		
		printf("a new client comming... \n");
		
		//����completion key
		LPCLIENTCONTEXT lpContext = AllocateContext();
		lpContext->m_Socket = clientSocket;		
		sprintf(lpContext->m_ip,inet_ntoa(client.sin_addr));
		lpContext->m_nPort = client.sin_port;
		
		
		printf("create completion port key successful... \n");
		
		//completion port ��socket��������
		if(!CreateIoCompletionPort((HANDLE)clientSocket, 
			m_hIocp, 
			(DWORD)lpContext, 
			0))
		{
			PRINTDEBUG(WSAGetLastError());
			closesocket( lpContext->m_Socket );
			delete lpContext;
			continue;
		}
		
		//��ʼ���ͻ�����
		OVERLAPPEDPLUS	*pOlp = AllocateOverlappedPlus(OP_IOInitialize);
		
		BOOL bSuccess = PostQueuedCompletionStatus(
			m_hIocp, 
			0,
			(DWORD) lpContext,
			&pOlp->ol);
		
		if ( (!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
		{            
			PRINTDEBUG(WSAGetLastError());
			closesocket( lpContext->m_Socket );
			delete lpContext;
			continue;
		}
		
		printf("associate completion port and new client successful... \n");
		
	}
	CloseCompletionPort();
	CloseHandle(m_hKillEvent);
}


//�ͻ�����ʱ��ʼ��
BOOL CIOCPServer::OnClientInitializing(LPCLIENTCONTEXT lpContext, DWORD dwIoSize)
{
//	memset(lpContext->m_ip, 0, sizeof(lpContext->m_ip));
	printf("socket init from :%s:%d\n", lpContext->m_ip, lpContext->m_nPort);
	
	return TRUE;
}

//���ͻ�����,receive,������յ����ݳ���Ϊ0,���ʾ,�ͻ������ӹر�
BOOL CIOCPServer::OnClientReading(LPCLIENTCONTEXT lpContext, DWORD dwIoSize)
{
	if(dwIoSize == 0)
	{
		FreeClientContext(lpContext);
		return FALSE;
	}
	printf("recv: %s\n", lpContext->m_wsaInBuffer.buf);
	printf("recv len: %d\n", dwIoSize);	

	
	lpContext->m_ReadBuffer.Write(lpContext->m_byInBuffer,dwIoSize);
	
	ProcessReceiveData(lpContext, lpContext->m_ReadBuffer);
	return TRUE;
}

//д�¼����,����CBuffer�е����ݲ�һ����������!
BOOL CIOCPServer::OnClientWriting(LPCLIENTCONTEXT lpContext, DWORD dwIoSize)
{
	ULONG ulFlags = MSG_PARTIAL;
	
	//ɾ���Ѿ������˵�����
	lpContext->m_WriteBuffer.Delete(dwIoSize);

	if (lpContext->m_WriteBuffer.GetBufferLen() == 0)
	{
		//���ݶ�������!

		//�������		
		lpContext->m_WriteBuffer.ClearBuffer();
		// д�¼������,���������´�д��
		SetEvent(lpContext->m_hWriteComplete);

		printf("WRITE event completed \n");
	
	}
	else
	{
		LPOVERLAPPEDPLUS pOverlap = AllocateOverlappedPlus(OP_IOWrite);
		
		lpContext->m_wsaOutBuffer.buf = lpContext->m_WriteBuffer.GetBuffer();
		lpContext->m_wsaOutBuffer.len = lpContext->m_WriteBuffer.GetBufferLen();
	
		printf("data to sent: %s , length:%d\n", 
			lpContext->m_wsaOutBuffer.buf,
			lpContext->m_wsaOutBuffer.len);

		int nRetVal = WSASend(lpContext->m_Socket, 
			&lpContext->m_wsaOutBuffer,
			1,
			&lpContext->m_wsaOutBuffer.len, 
			ulFlags,
			&pOverlap->ol, 
			NULL);
		
		if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING ) 
		{
			FreeClientContext(lpContext);
		}
		
	}
	
	return FALSE;//���ȴ�����!

}

void CIOCPServer::FreeClientContext(LPCLIENTCONTEXT lpContext)
{
	if(ClientExit(lpContext))
	{
		//���û����˳��¼��Ѿ������,�Լ����ؼ���
		return ;
	}

	CancelIo((HANDLE) lpContext->m_Socket);
	
	closesocket( lpContext->m_Socket );
	lpContext->m_Socket = INVALID_SOCKET;
	
	delete lpContext;
}

//������Ϣ
void CIOCPServer::Send(LPCLIENTCONTEXT lpContext, CString strData)
{
	//����Ҫ���͵���������뷢�ͻ�����	
	lpContext->m_WriteBuffer.Write(strData);

	printf("Waiting for WRITE event\n");
	WaitForSingleObject(lpContext->m_hWriteComplete, INFINITE);

	//׼����������
	lpContext->m_wsaOutBuffer.buf = lpContext->m_WriteBuffer.GetBuffer();
	lpContext->m_wsaOutBuffer.len = lpContext->m_WriteBuffer.GetBufferLen();
	
	LPOVERLAPPEDPLUS lpOverlap = AllocateOverlappedPlus(OP_IOWrite);
	PostQueuedCompletionStatus(m_hIocp, 0, (DWORD) lpContext, &lpOverlap->ol);

	
}

