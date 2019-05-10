
#include "stdafx.h"
#include "chatter.h"
#include "chattermgr.h"
#include "FileTransfer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//监听文件传送线程
UINT AcceptFunc( LPVOID pParam )
{
	SOCKET s = (SOCKET)pParam;
	
	listen(s, 5);
	sockaddr_in client;
	int sClientLength = sizeof(client);
	while(true)
	{
		SOCKET sClient = accept(s,  (sockaddr*)&client, &sClientLength);
		
		AfxBeginThread(ReceiveFunc, (LPVOID)sClient);
	}
	
}

//连接另外用户,传送文件
UINT ConnectFunc( LPVOID pParam )
{
	//取得用户信息
	LPFILETRANSFER lpF = (LPFILETRANSFER)pParam;
	LPUSERINFO lpUser = lpF->m_lpUser;

	SOCKET s = socket(AF_INET, SOCK_STREAM,IPPROTO_IP);
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpUser->m_ip);
	sockAddr.sin_port = htons((u_short)lpUser->m_nFilePort);

	//打开文件
	CFile file;
	CFileException e;
	if(!file.Open(lpF->m_fileName, CFile::modeRead | CFile::typeBinary, &e))
	{
		char szError[1024];
		e.GetErrorMessage(szError, 1024);	
		GetChatterMgr()->DoServerMessage("文件打开失败:"+CString(szError));
		
		closesocket(s);
		delete lpF;
		return 0;
	}

	//连接用户
	if(SOCKET_ERROR  == connect(s, (sockaddr*)&sockAddr, sizeof(sockAddr)))
	{
		GetChatterMgr()->DoServerMessage("连接失败,请确认 ["+CString(lpUser->m_name)+"] 有无真实IP!");
		closesocket(s);
		delete lpF;
		return 0;
	}
		
	DWORD nFileLength = file.GetLength();

	//发送的数据:文件长度[4个字节]+文件名长度[4个字节]+文件名+用户名长度[4个字节]+用户名称
	
	//获得文件信息
	char buf[1024]="\0";

	//发送的数据
	int nPointer = 0;
	int nSize = sizeof(int);

	//文件长度
	memcpy(buf+nPointer, &nFileLength, nSize);
	nPointer += nSize;
	
	//文件名长度
	int nTmpLength = file.GetFileName().GetLength(); 
	memcpy(buf+nPointer, &nTmpLength, nSize);
	nPointer += nSize;

	//文件名
	sprintf(buf+nPointer, "%s", file.GetFileName());
	nPointer += file.GetFileName().GetLength();

	//用户名长度
	nTmpLength = strlen(GetChatterMgr()->GetUser()->m_name); 
	memcpy(buf+nPointer, &nTmpLength, nSize);
	nPointer +=	nSize;

	//用户名
	sprintf(buf+nPointer, "%s", GetChatterMgr()->GetUser()->m_name);
	nPointer += strlen(GetChatterMgr()->GetUser()->m_name);
	
	if(SOCKET_ERROR  == send(s, buf, nPointer, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("异常原因导致传送文件==> ["+CString(lpUser->m_name)+"]失败!");
		closesocket(s);
		delete lpF;
		return 0;		
	}

	//获得接收或拒绝通知
	if( 1 != recv(s, buf, 1, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("["+CString(lpUser->m_name)+"] 拒绝接收文件!");
		closesocket(s);
		delete lpF;
		return 0;		
	}

	//添加进度条
	CString szNotify;
	szNotify = "发送文件 ["+CString(file.GetFileName())+"] ";
	DWORD nIndex = GetChatterMgr()->AddTP(szNotify, nFileLength);
	
	GetChatterMgr()->DoServerMessage("["+CString(lpUser->m_name)+"] 同意接收文件,正在传输文件...");

	DWORD nPos = 0;
	DWORD nTime = GetTickCount();
	DWORD nTimeStart = nTime;
	do 
	{
		UINT nBytesRead = file.Read(buf, sizeof(buf));	
		if(SOCKET_ERROR  == send(s, buf, nBytesRead, MSG_PARTIAL))
		{
			GetChatterMgr()->DoServerMessage("异常原因导致传送文件==> ["+CString(lpUser->m_name)+"]失败!");
			closesocket(s);
			delete lpF;
			return 0;		
		}
		nPos+=nBytesRead;
		DWORD nTimeEnd = GetTickCount();
		if(nTimeEnd - nTimeStart > 1000)
		{
			GetChatterMgr()->SetTPStep(nIndex, nPos);

			DWORD nVelocity = nPos/(nTimeEnd-nTime)*1000/1024;
			GetChatterMgr()->SetTPVelocity(nIndex, nVelocity);

			nTimeStart = nTimeEnd;
		}
		if(nBytesRead < sizeof(buf))
			break;

//		Sleep(2);
	} while(TRUE);
	

	file.Close();
	delete lpF;

	closesocket(s);

	GetChatterMgr()->DoServerMessage("传送文件==> ["+CString(lpUser->m_name)+"] 成功");
	
	GetChatterMgr()->DeleteTP(nIndex);
	return 0;
	
}
UINT ReceiveFunc( LPVOID pParam )
{
	SOCKET s = (SOCKET)pParam;

	//接收的数据:文件长度[4个字节]+文件名长度[4个字节]+文件名+用户名长度[4个字节]+用户名称
	
	char buf[1024]="\0";
	char fileName[256]="\0";
	char userName[20]="\0";
	int nLength = 1;
	int nFileLength = 1;
	int nNameLength = 1;

	//文件长度
	int nSize = sizeof(int);
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nFileLength, buf, nSize);
	
	//文件名长度
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nLength, buf, nSize);

	//文件名称
	recv(s, fileName, nLength, MSG_PARTIAL);

	//用户名长度
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nNameLength, buf, nSize);

	//获得用户名称
	recv(s, userName, nNameLength, MSG_PARTIAL);

	CString szNotify;
	szNotify = "["+CString(userName)+"] 传送文件 ["+CString(fileName)+"] ,接收 或 拒绝?";
	
	GetChatterMgr()->DoServerMessage(szNotify);	

	if(AfxMessageBox(szNotify, MB_YESNO) == IDNO)
	{
		closesocket(s);		
		return 0 ;
	}

	CFileDialog fdlg(FALSE,NULL, fileName);
	if(fdlg.DoModal() == IDCANCEL)
	{
		closesocket(s);		
		return 0;
	}		

	//发送接收通知
	send(s, "T", 1, MSG_PARTIAL);

	GetChatterMgr()->DoServerMessage("["+CString(fileName)+"] 传输中....");	
	
	CFile file;
	file.Open(fdlg.GetPathName(), CFile::modeReadWrite|CFile::modeCreate|CFile::shareDenyWrite);

	//添加进度条
	szNotify="接收文件 ["+file.GetFileName()+"]";
	DWORD nIndex = GetChatterMgr()->AddTP(szNotify, nFileLength);
	
	BOOL bRead = TRUE;
	DWORD nPos = 0;
	DWORD nTime = GetTickCount();
	DWORD nTimeStart = nTime;

	while(bRead)
	{
		int nResult = recv(s, buf, 1024, MSG_PARTIAL);
		
		switch(nResult)
		{
		case 0:
			bRead = FALSE;
			break;
		case SOCKET_ERROR:
			if (GetLastError() != WSAEWOULDBLOCK) 
			{
				PRINTDEBUG(FALSE);
				bRead = FALSE;
			}
			else
			{
				break;
			}
		default:
			file.Write(buf, nResult);
		}

		nPos += nResult;
		DWORD nTimeEnd = GetTickCount();
		if(nTimeEnd - nTimeStart > 1000)
		{
			GetChatterMgr()->SetTPStep(nIndex, nPos);

			DWORD nVelocity = nPos/(nTimeEnd-nTime)*1000/1024;
			GetChatterMgr()->SetTPVelocity(nIndex, nVelocity);
			
			nTimeStart = nTimeEnd;
		}
	}
	file.Close();

	closesocket(s);

	GetChatterMgr()->DoServerMessage("["+CString(userName)+"] 传送文件成功 "+fdlg.GetPathName());
	GetChatterMgr()->DeleteTP(nIndex);
	
	return 0;
}
