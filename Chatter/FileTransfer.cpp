
#include "stdafx.h"
#include "chatter.h"
#include "chattermgr.h"
#include "FileTransfer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//�����ļ������߳�
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

//���������û�,�����ļ�
UINT ConnectFunc( LPVOID pParam )
{
	//ȡ���û���Ϣ
	LPFILETRANSFER lpF = (LPFILETRANSFER)pParam;
	LPUSERINFO lpUser = lpF->m_lpUser;

	SOCKET s = socket(AF_INET, SOCK_STREAM,IPPROTO_IP);
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpUser->m_ip);
	sockAddr.sin_port = htons((u_short)lpUser->m_nFilePort);

	//���ļ�
	CFile file;
	CFileException e;
	if(!file.Open(lpF->m_fileName, CFile::modeRead | CFile::typeBinary, &e))
	{
		char szError[1024];
		e.GetErrorMessage(szError, 1024);	
		GetChatterMgr()->DoServerMessage("�ļ���ʧ��:"+CString(szError));
		
		closesocket(s);
		delete lpF;
		return 0;
	}

	//�����û�
	if(SOCKET_ERROR  == connect(s, (sockaddr*)&sockAddr, sizeof(sockAddr)))
	{
		GetChatterMgr()->DoServerMessage("����ʧ��,��ȷ�� ["+CString(lpUser->m_name)+"] ������ʵIP!");
		closesocket(s);
		delete lpF;
		return 0;
	}
		
	DWORD nFileLength = file.GetLength();

	//���͵�����:�ļ�����[4���ֽ�]+�ļ�������[4���ֽ�]+�ļ���+�û�������[4���ֽ�]+�û�����
	
	//����ļ���Ϣ
	char buf[1024]="\0";

	//���͵�����
	int nPointer = 0;
	int nSize = sizeof(int);

	//�ļ�����
	memcpy(buf+nPointer, &nFileLength, nSize);
	nPointer += nSize;
	
	//�ļ�������
	int nTmpLength = file.GetFileName().GetLength(); 
	memcpy(buf+nPointer, &nTmpLength, nSize);
	nPointer += nSize;

	//�ļ���
	sprintf(buf+nPointer, "%s", file.GetFileName());
	nPointer += file.GetFileName().GetLength();

	//�û�������
	nTmpLength = strlen(GetChatterMgr()->GetUser()->m_name); 
	memcpy(buf+nPointer, &nTmpLength, nSize);
	nPointer +=	nSize;

	//�û���
	sprintf(buf+nPointer, "%s", GetChatterMgr()->GetUser()->m_name);
	nPointer += strlen(GetChatterMgr()->GetUser()->m_name);
	
	if(SOCKET_ERROR  == send(s, buf, nPointer, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("�쳣ԭ���´����ļ�==> ["+CString(lpUser->m_name)+"]ʧ��!");
		closesocket(s);
		delete lpF;
		return 0;		
	}

	//��ý��ջ�ܾ�֪ͨ
	if( 1 != recv(s, buf, 1, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("["+CString(lpUser->m_name)+"] �ܾ������ļ�!");
		closesocket(s);
		delete lpF;
		return 0;		
	}

	//��ӽ�����
	CString szNotify;
	szNotify = "�����ļ� ["+CString(file.GetFileName())+"] ";
	DWORD nIndex = GetChatterMgr()->AddTP(szNotify, nFileLength);
	
	GetChatterMgr()->DoServerMessage("["+CString(lpUser->m_name)+"] ͬ������ļ�,���ڴ����ļ�...");

	DWORD nPos = 0;
	DWORD nTime = GetTickCount();
	DWORD nTimeStart = nTime;
	do 
	{
		UINT nBytesRead = file.Read(buf, sizeof(buf));	
		if(SOCKET_ERROR  == send(s, buf, nBytesRead, MSG_PARTIAL))
		{
			GetChatterMgr()->DoServerMessage("�쳣ԭ���´����ļ�==> ["+CString(lpUser->m_name)+"]ʧ��!");
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

	GetChatterMgr()->DoServerMessage("�����ļ�==> ["+CString(lpUser->m_name)+"] �ɹ�");
	
	GetChatterMgr()->DeleteTP(nIndex);
	return 0;
	
}
UINT ReceiveFunc( LPVOID pParam )
{
	SOCKET s = (SOCKET)pParam;

	//���յ�����:�ļ�����[4���ֽ�]+�ļ�������[4���ֽ�]+�ļ���+�û�������[4���ֽ�]+�û�����
	
	char buf[1024]="\0";
	char fileName[256]="\0";
	char userName[20]="\0";
	int nLength = 1;
	int nFileLength = 1;
	int nNameLength = 1;

	//�ļ�����
	int nSize = sizeof(int);
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nFileLength, buf, nSize);
	
	//�ļ�������
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nLength, buf, nSize);

	//�ļ�����
	recv(s, fileName, nLength, MSG_PARTIAL);

	//�û�������
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nNameLength, buf, nSize);

	//����û�����
	recv(s, userName, nNameLength, MSG_PARTIAL);

	CString szNotify;
	szNotify = "["+CString(userName)+"] �����ļ� ["+CString(fileName)+"] ,���� �� �ܾ�?";
	
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

	//���ͽ���֪ͨ
	send(s, "T", 1, MSG_PARTIAL);

	GetChatterMgr()->DoServerMessage("["+CString(fileName)+"] ������....");	
	
	CFile file;
	file.Open(fdlg.GetPathName(), CFile::modeReadWrite|CFile::modeCreate|CFile::shareDenyWrite);

	//��ӽ�����
	szNotify="�����ļ� ["+file.GetFileName()+"]";
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

	GetChatterMgr()->DoServerMessage("["+CString(userName)+"] �����ļ��ɹ� "+fdlg.GetPathName());
	GetChatterMgr()->DeleteTP(nIndex);
	
	return 0;
}
