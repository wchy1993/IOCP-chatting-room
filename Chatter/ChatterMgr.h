

#if !defined(AFX_CHATTERMGR_H__AC18B233_6CD4_43CC_891D_AFE5933834BB__INCLUDED_)
#define AFX_CHATTERMGR_H__AC18B233_6CD4_43CC_891D_AFE5933834BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "buffer.h"
#include "clientcommand.h"
#include "chatterdlg.h"
#include <afxmt.h>
#include "Sound.h"

class CClientSocket;

class CChatterMgr  
{
protected:
	static CChatterMgr *s_pChatter;
public:
	static CChatterMgr *GetChatterMgr();

private:
	CChatterMgr();
	 ~CChatterMgr();

public:
	void ServerClosed()
	{
		DoServerMessage("服务器未知原因关闭!");	
		ClientClosed();
		DoServerMessage("本地连接关闭!");
	}
	void ClientClosed()
	{
		m_pSocket->ShutDown(2);
	}	
	
	void DoUserMessage(LPCSTR strMsg)
	{
		m_pChatterDlg->AddUserSay(strMsg);		
	}
	
	void DoServerMessage(LPCSTR strMsg)
	{
		m_pChatterDlg->AddServerMessage(strMsg);
	}
	
	void SetConnected()
	{
		m_bConnected = TRUE;
	}
	BOOL IsConnected()
	{
		return m_bConnected;
	}

	void SetInterface(CChatterDlg *pDlg)
	{
		m_pChatterDlg = pDlg;
	}

	void DeleteTP(DWORD nIndex);
	DWORD AddTP(LPCSTR strName, DWORD nMaxRange);
	void SetTPVelocity(DWORD nIndex, DWORD nVelocity);
	void SetTPStep(DWORD nIndex, DWORD nPos);

	void ProcessReceiveData(LPCSTR lpData);
	BOOL Login();	
	void OnConnect(int nErrorCode);
	BOOL DoConnect(LPCTSTR lpszHandle, LPCTSTR lpszAddress, UINT nPort);
	void DoLogin();
	void DoSay(USERINFO &user, LPCSTR lpszSay);
	void SendFile(LONG id,CString strFile);
	void SendSound(LONG id);// 语音
	void SendMsg(CString strCmd);	
	LPUSERINFO GetUser(){return &m_user;}
protected:
	//call back函数
	void LoginSuccessFunc(USERINFO &user, MAPUSER*lpMapUser);
	void UserMessageFunc(USERINFO &userSrc,USERINFO &userDest, CString strMessage);
	void NewUserFunc(USERINFO &user);
	void LogoutFunc(USERINFO &user);
	
private:
	CChatterDlg *m_pChatterDlg;		
	CAsyncSocket *m_pSocket;


	CClientCommand<CChatterMgr>		m_cmd;	//合法指令
	CBuffer m_buffer;						//缓存

	HANDLE m_hLoginEvent;
	HANDLE m_hConnectEvent;
	BOOL m_bConnected;			
	
	USERINFO	m_user;
	MAPUSER*	m_lpMapUsers;

	CCriticalSection	m_cs;

	DWORD	m_nIndex;			//progressbar的索引
public:
	CSound _sound;
	char SoundBuf[MAX_BUFFER_SIZE];
	CMutex _SoundMutex;
};

#endif // !defined(AFX_CHATTERMGR_H__AC18B233_6CD4_43CC_891D_AFE5933834BB__INCLUDED_)
