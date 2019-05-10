
#include "stdafx.h"
#include "Chatter.h"
#include "ClientSocket.h"
#include "chatterMgr.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket()
{
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

void CClientSocket::OnConnect(int nErrorCode) 
{
	GetChatterMgr()->OnConnect(nErrorCode);
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode) 
{
	if(nErrorCode == 0)
	{
		char buf[1024]="\0";
		
		if(SOCKET_ERROR == Receive(buf, 1024))
		{
			GetChatterMgr()->ServerClosed();
		}
		else
		{
			GetChatterMgr()->ProcessReceiveData(buf);
		}
	}
		
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSocket::OnSend(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CAsyncSocket::OnSend(nErrorCode);
}

void CClientSocket::OnClose(int nErrorCode) 
{
	if(nErrorCode != 0)
	{
		GetChatterMgr()->ServerClosed();
	}
	
	CAsyncSocket::OnClose(nErrorCode);
}
