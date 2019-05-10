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
		//找不到该用户,说明已经用户已经退出了
		CoUninitialize(); 
		return TRUE;
	}
	

	EnterCriticalSection(&m_cs);
	m_mapUsers.RemoveKey((LONG)lpContext);
	LeaveCriticalSection(&m_cs);
	
	//发送退出的用户名称给其他用户
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
		//循环取得有效指令
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
	//用户登陆
	//将用户信息发送给用户,以及将服务器上用户列表发送给用户
	sprintf(user.m_ip, ((LPCLIENTCONTEXT)(user.m_id))->m_ip);
	user.m_nPort = ((LPCLIENTCONTEXT)(user.m_id))->m_nPort;
	CString strCmd = m_cmd.CreateLoginSuccess(user, &m_mapUsers);
	
	Send((LPCLIENTCONTEXT)user.m_id, strCmd);

	//将新用户发送所有他用户
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

	//添加到列表
	EnterCriticalSection(&m_cs);
	m_mapUsers[user.m_id] = &user;	
	LeaveCriticalSection(&m_cs);
}

//发送某用户的发言其他用户:区别私聊
void CChatServer::UserMessageFunc(USERINFO &userSrc,USERINFO &userDest,CString strSpeak)
{
	if(userDest.m_id == 0)
	{
		CString strCmd = m_cmd.CreateUserMessage(userSrc, userDest, strSpeak);
		
		//发送给所有用户
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
		//发送给单一用户:私聊
		CString strCmd = m_cmd.CreateUserMessage(userSrc, *m_mapUsers[userDest.m_id], strSpeak);

		Send((LPCLIENTCONTEXT)userDest.m_id, strCmd);
		//如果不是发送给自己的,则还要发送给自己
		//即,私聊自己也能看见
		if(userDest.m_id != userSrc.m_id)
			Send((LPCLIENTCONTEXT)userSrc.m_id, strCmd);
	}
}
