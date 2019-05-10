
#include "stdafx.h"
#include "chatter.h"
#include "ChatterMgr.h"
#include "logindlg.h"
#include "waiting.h"
#include "clientsocket.h"
#include "filetransfer.h"
#include <string>
#include "SoundTransfer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatterMgr *CChatterMgr::s_pChatter = NULL;
CChatterMgr *CChatterMgr::GetChatterMgr()
{
	if(s_pChatter == NULL)
	{
		s_pChatter = new CChatterMgr;
	}
	return s_pChatter;
}


CChatterMgr::CChatterMgr()
{
	CoInitialize(NULL); 
	
	memset(&m_user, 0, sizeof(m_user));

	m_nIndex = 0;
	m_pSocket = new CClientSocket;

	m_bConnected = FALSE;
	m_hConnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hLoginEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_pChatterDlg = NULL;

	m_cmd.SetLoginSuccessFunc(&CChatterMgr::LoginSuccessFunc);
	m_cmd.SetLogoutFunc(&CChatterMgr::LogoutFunc);
	m_cmd.SetNewUserFunc(&CChatterMgr::NewUserFunc);
	m_cmd.SetUserMessageFunc(&CChatterMgr::UserMessageFunc);
	
	m_lpMapUsers = NULL;
	m_cmd.SetType(this);
	
}

CChatterMgr::~CChatterMgr()
{
	CoUninitialize(); 

	m_pSocket->ShutDown(2);	
	m_pSocket->Close();
	delete m_pSocket;

	m_pChatterDlg = NULL;
	CloseHandle(m_hConnectEvent);
	CloseHandle(m_hLoginEvent);

	POSITION pos = m_lpMapUsers->GetStartPosition();
	for(;pos!=NULL;)
	{
		LONG id;
		LPUSERINFO lpUser;
		m_lpMapUsers->GetNextAssoc(pos, id, lpUser);
		m_lpMapUsers->RemoveKey(id);
		delete lpUser;
	}
}

BOOL CChatterMgr::Login()
{
	//��ʼ��socket
	if(!m_pSocket->Create())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED, MB_OK|MB_ICONSTOP);
		delete m_pSocket;
		m_pSocket = NULL;
		return FALSE;
	}
	
	//��½
	CLoginDlg dlg;
	
	while(TRUE)
	{

		if (IDOK != dlg.DoModal())
		{
			return FALSE;
		}
		
		if (!DoConnect(dlg.m_strUser, dlg.m_strServer, dlg.m_nPort))
		{
			m_pSocket->ShutDown(2);

			//����ʧ��
			if (AfxMessageBox(IDS_RETRYCONNECT,MB_YESNO) == IDNO)
			{
				return FALSE;
			}
		}
		else
			break;
	}	
	
	return TRUE;	
}

BOOL CChatterMgr::DoConnect(LPCTSTR lpszUser, LPCTSTR lpszServer, UINT nPort)
{
	//��ʾ������ʾ�Ի���
	CWaiting waiting;
	
	sprintf(m_user.m_name,lpszUser);
	memset(m_user.m_ip,0, sizeof(m_user.m_ip));
	m_user.m_nPort = 0;

	//���ӷ�����
	if(!m_pSocket->Connect(lpszServer, nPort))
	{
		if(WSAEWOULDBLOCK != WSAGetLastError())
		{
			PRINTDEBUG(FALSE);
			m_pSocket->Close();
			return FALSE;
		}
	}
	
	//�ȴ������¼�����
	if(!waiting.DoWaiting(m_hConnectEvent, INFINITE))
		return FALSE;
	
	//�Ƿ����ӳɹ�,��:����FALSE
	if(!IsConnected())	return FALSE;	
	
	//��½
	DoLogin();
	
	waiting.SetPrompt("��֤�û�...");
	
	
	//�ȴ�ע��,��ʱ�趨20��
	if(!waiting.DoWaiting(m_hLoginEvent, 5000))
		return FALSE;
	
	CString str;
	m_pChatterDlg->GetWindowText(str);
	m_pChatterDlg->SetWindowText(str+" [ "+CString(m_user.m_name)+" ] ");

	return TRUE;
}
void CChatterMgr::OnConnect(int nErrorCode)
{
	if(nErrorCode == 0)
	{
		SetConnected();
	}
	SetEvent(m_hConnectEvent);
}

//��½
void CChatterMgr::DoLogin()
{
	//��ʼ�������ļ�����socket
	SOCKET s = socket(AF_INET, SOCK_STREAM,IPPROTO_IP);
	
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	int nSize = sizeof(sockAddr);
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((u_short)0);

	bind(s, (sockaddr*)&sockAddr, nSize);

	getsockname(s, (sockaddr*)&sockAddr, &nSize);

	m_user.m_nFilePort = ntohs(sockAddr.sin_port);
	
	//���������߳�
	AfxBeginThread(AcceptFunc, (LPVOID)s);

	{
		//��ʼ����������socket
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSize = sizeof(sockAddr);
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons((u_short)0);

		bind(s, (sockaddr*)&sockAddr, nSize);

		getsockname(s, (sockaddr*)&sockAddr, &nSize);

		m_user.m_nSoundPort = ntohs(sockAddr.sin_port);

		//�������������߳�
		AfxBeginThread(AcceptSoundFunc, (LPVOID)s);


	}


	CString strCmd = m_cmd.CreateUserLogin(m_user);
	SendMsg(strCmd);	
}

//����
void CChatterMgr::DoSay(USERINFO &user, LPCSTR lpszSay) 
{	
	//m_user.m_type = user.m_type;
	CString strCmd = m_cmd.CreateUserSpeak(m_user, user, lpszSay);	
	SendMsg(strCmd);	
}


void CChatterMgr::SendMsg(CString strCmd)
{
	m_pSocket->Send(strCmd, strCmd.GetLength());
}

//������յ�������
void CChatterMgr::ProcessReceiveData(LPCSTR lpData)
{
	//�½��յ�����д�뻺��
	m_buffer.Write(lpData, strlen(lpData));
	printf("recv data:%s\n", lpData);
	//��ȡָ��
	try
	{
		CString strXML;
		while(m_cmd.HasNetPacket(m_buffer, strXML))
		{
			m_cmd.DoProcessNetPacket(m_user.m_id, strXML);
		}
	}
	catch (_com_error &e)
	{
		AfxMessageBox((char*)e.Description());
	}
}

//������call back ����
void CChatterMgr::LogoutFunc(USERINFO &user)
{
	LPUSERINFO lpUser;
	if(m_lpMapUsers->Lookup(user.m_id, lpUser))
	{
		m_pChatterDlg->DeleteUser(*lpUser);
		delete lpUser;
	}
	DoServerMessage("["+CString(user.m_name)+"] �뿪������");
	
}

void CChatterMgr::NewUserFunc(USERINFO &user)
{
	(*m_lpMapUsers)[user.m_id] = &user;	
	m_pChatterDlg->AddUser(&user);

	char port[10];
	sprintf(port, "%d", user.m_nPort);
	DoServerMessage("��ӭ ["+CString(user.m_name)+"] �� ["+ user.m_ip +":"+port+"] ����������");
}

void CChatterMgr::UserMessageFunc(USERINFO &userSrc,USERINFO &userDest, CString strMessage)
{
	CString strSrc(userSrc.m_name);
	CString strDest(userDest.m_name);
	CString str = "[" + strSrc + "]" + " �� [" + strDest + "] ˵: " + strMessage;
	DoUserMessage(str);
}

void CChatterMgr::LoginSuccessFunc(USERINFO &user, MAPUSER *lpMapUser)
{
	SetEvent(m_hLoginEvent);

	m_user.m_id = user.m_id;
	m_user.m_nPort = user.m_nPort;
	sprintf(m_user.m_ip, user.m_ip);
	m_lpMapUsers = lpMapUser;

	//����Լ�
	m_pChatterDlg->AddUser(&m_user, FALSE);	

	//�û��б�	
	POSITION pos = m_lpMapUsers->GetStartPosition();
	for(;pos!=NULL;)
	{
		LONG id;
		LPUSERINFO lpUser;
		m_lpMapUsers->GetNextAssoc(pos, id, lpUser);
		//���������
		m_pChatterDlg->AddUser(lpUser);	
	}
	
	char port[10];
	sprintf(port, "%d", user.m_nPort);
	DoServerMessage("��ӭ ["+CString(user.m_name)+"] �� ["+ user.m_ip +":"+port+"] ����������");
}

void CChatterMgr::SendFile(LONG id, CString strFile)
{
	//���ѡ�е��û�
	LPUSERINFO lpUser;
	if(!m_lpMapUsers->Lookup(id, lpUser))
	{
		lpUser = &m_user;
	}
	LPFILETRANSFER lpF = new FILETRANSFER;
	lpF->m_lpUser = lpUser;
	strcpy(lpF->m_fileName, strFile);

	AfxBeginThread(ConnectFunc, (LPVOID)lpF);

}

void CChatterMgr::SendSound(LONG id)
{
	//���ѡ�е��û�
	LPUSERINFO lpUser;
	if (!m_lpMapUsers->Lookup(id, lpUser))
	{
		lpUser = &m_user;
	}
	AfxBeginThread(ConnectSoundFunc, lpUser);


}

DWORD CChatterMgr::AddTP(LPCSTR strName, DWORD nMaxRange)
{
	CSingleLock lock(&m_cs);
	lock.Lock();

	DWORD nIndex = m_nIndex;
	SendMessage(m_pChatterDlg->GetSafeHwnd(), WM_ADDPROGRESS, nIndex, nMaxRange);
	SendMessage(m_pChatterDlg->GetSafeHwnd(), WM_SETLABEL, nIndex, (WPARAM)strName);
	
	m_nIndex++;

	lock.Unlock();
	
	return nIndex;

}

//���ò���
void CChatterMgr::SetTPStep(DWORD nIndex, DWORD nPos)
{
	SendMessage(m_pChatterDlg->GetSafeHwnd(), WM_SETPOS, nIndex, nPos);	
}
//�����ٶ���ʾ
void CChatterMgr::SetTPVelocity(DWORD nIndex, DWORD nVelocity)
{
	SendMessage(m_pChatterDlg->GetSafeHwnd(), WM_SETVELOCITY, nIndex, nVelocity);	
}

void CChatterMgr::DeleteTP(DWORD nIndex)
{
	SendMessage(m_pChatterDlg->GetSafeHwnd(), WM_DELPROGRESS, nIndex, 0);
}

