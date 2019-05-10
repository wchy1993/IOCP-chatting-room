

#if !defined(AFX_CHATSERVER1_H__F6365C1E_C996_4A39_A4BE_505DA39112AC__INCLUDED_)
#define AFX_CHATSERVER1_H__F6365C1E_C996_4A39_A4BE_505DA39112AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "IOCPServer.h"
#include "servercommand.h"

class CChatServer : public CIOCPServer  
{
public:
	CChatServer();
	virtual ~CChatServer();
protected:
	virtual BOOL ClientExit(LPCLIENTCONTEXT lpContext);
	virtual void ProcessReceiveData(LPCLIENTCONTEXT lpContext, CBuffer &buffer);

	//call back
	void UserMessageFunc(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
	void LoginFunc(USERINFO &user);
private:
	MAPUSER m_mapUsers;
	CServerCommand<CChatServer>		m_cmd;	//合法指令
	CRITICAL_SECTION m_cs;					//互斥变量
};

#endif // !defined(AFX_CHATSERVER1_H__F6365C1E_C996_4A39_A4BE_505DA39112AC__INCLUDED_)
