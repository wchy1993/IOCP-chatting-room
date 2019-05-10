#include "stdafx.h"
#include "ChatServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatServer::CChatServer()
{
	m_cmd.SetType(this);

	m_cmd.SetLoginFunc(&CChatServer::LoginFunc);
	m_cmd.SetUserMessageFunc(&CChatServer::UserMessageFunc);
	
	InitializeCriticalSection(&m_cs);
}

CChatServer::~CChatServer()
{
	DeleteCriticalSection(&m_cs);
}

BOOL CChatServer::ClientExit(LPCLIENTCONTEXT lpContext)
{
	CoInitialize(NULL); 

	LPUSERINFO lpUser;
	if(!m_mapUsers.Lookup((LONG)lpContext, lpUser))
	{
		//�Ҳ������û�,˵���Ѿ��û��Ѿ��˳���
		CoUninitialize(); 
		return TRUE;
	}
	

	EnterCriticalSection(&m_cs);
	m_mapUsers.RemoveKey((LONG)lpContext);
	LeaveCriticalSection(&m_cs);
	
	//�����˳����û����Ƹ������û�
	CString strCmd = m_cmd.CreateUserLogout(*lpUser);
	printf("%s LEAVE\n", lpUser->m_name);
	delete lpUser;

	
	POSITION pos = m_mapUsers.GetStartPosition();
	for(;pos!=NULL;)
	{
		LPUSERINFO lpUser;
		LONG  pContext;
		m_mapUsers.GetNextAssoc(pos,  pContext, lpUser);
		Send((LPCLIENTCONTEXT)pContext, strCmd);
	}	
	
	CoUninitialize(); 
	return FALSE;

}

void CChatServer::ProcessReceiveData(LPCLIENTCONTEXT lpContext, CBuffer &buffer)
{
	CoInitialize(NULL); 

	try
	{
		//ѭ��ȡ����Чָ��
		CString strXML;
		while(m_cmd.HasNetPacket(buffer, strXML))
		{
			if(!m_cmd.DoProcessNetPacket((LONG)lpContext, strXML))
			{
				printf("Error xml format: %s\n", (LPCSTR)strXML);
			}
		}
	}
	catch(_com_error &e)
	{
		printf("%s", (char *)e.Description());
	}

	CoUninitialize(); 
	
}

void CChatServer::LoginFunc(USERINFO &user)
{
	//�û���½
	//���û���Ϣ���͸��û�,�Լ������������û��б��͸��û�
	sprintf(user.m_ip, ((LPCLIENTCONTEXT)(user.m_id))->m_ip);
	user.m_nPort = ((LPCLIENTCONTEXT)(user.m_id))->m_nPort;
	CString strCmd = m_cmd.CreateLoginSuccess(user, &m_mapUsers);
	
	Send((LPCLIENTCONTEXT)user.m_id, strCmd);

	//�����û������������û�
	strCmd = m_cmd.CreateNewUser(user);

	POSITION pos = m_mapUsers.GetStartPosition();
	for(;pos!=NULL;)
	{
		LPUSERINFO lpUser;
		LONG  lpContext;
		m_mapUsers.GetNextAssoc(pos,  lpContext, lpUser);
		
		if( user.m_id != (DWORD)lpContext)
		{
			Send((LPCLIENTCONTEXT)lpContext, strCmd);
		}
	}

	//��ӵ��б�
	EnterCriticalSection(&m_cs);
	m_mapUsers[user.m_id] = &user;	
	LeaveCriticalSection(&m_cs);
}

//����ĳ�û��ķ��������û�:����˽��
void CChatServer::UserMessageFunc(USERINFO &userSrc,USERINFO &userDest,CString strSpeak)
{
	if(userDest.m_id == 0)
	{
		CString strCmd = m_cmd.CreateUserMessage(userSrc, userDest, strSpeak);
		
		//���͸������û�
		POSITION pos = m_mapUsers.GetStartPosition();
		for(;pos!=NULL;)
		{
			LPUSERINFO lpUser;
			LONG  lpContext;
			m_mapUsers.GetNextAssoc(pos,  lpContext, lpUser);
			
			Send((LPCLIENTCONTEXT)lpContext, strCmd);
		}
	}
	else
	{
		//���͸���һ�û�:˽��
		CString strCmd = m_cmd.CreateUserMessage(userSrc, *m_mapUsers[userDest.m_id], strSpeak);

		Send((LPCLIENTCONTEXT)userDest.m_id, strCmd);
		//������Ƿ��͸��Լ���,��Ҫ���͸��Լ�
		//��,˽���Լ�Ҳ�ܿ���
		if(userDest.m_id != userSrc.m_id)
			Send((LPCLIENTCONTEXT)userSrc.m_id, strCmd);
	}
}
