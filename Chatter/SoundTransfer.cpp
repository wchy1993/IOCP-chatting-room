#include "stdafx.h"
#include "chatter.h"
#include "chattermgr.h"
#include "SoundTransfer.h"
#include "ChatterDlg.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static bool IsRun = true;

// 声音停止
void StopSound()
{
	IsRun = false;
	CChatterDlg::GetInitializePtr()->_RunSoundFlag = -1;
	GetChatterMgr()->_sound.StopRecord();
//	GetChatterMgr()->_sound.StopPlay();
}

//监听文件传送线程
UINT AcceptSoundFunc(LPVOID pParam)
{
	SOCKET s = (SOCKET)pParam;

	listen(s, 5);
	sockaddr_in client;
	int sClientLength = sizeof(client);
	while (TRUE)
	{
		SOCKET sClient = accept(s, (sockaddr*)&client, &sClientLength);

		IsRun = true;
		GetChatterMgr()->_sound.StopRecord();
		//GetChatterMgr()->_sound.StopPlay();

		GetChatterMgr()->_sound.Init(CChatterDlg::GetInitializePtr());
		GetChatterMgr()->_sound.Record();
		AfxBeginThread(ReceiveSoundFunc, (LPVOID)sClient);
	}

}

//连接另外用户,传送文件
UINT ConnectSoundFunc(LPVOID pParam)
{
	IsRun = true;
	LPUSERINFO lpUser = (LPUSERINFO)pParam;
	//取得用户信息

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpUser->m_ip);
	sockAddr.sin_port = htons((u_short)lpUser->m_nSoundPort);

	//连接用户
	if (SOCKET_ERROR == connect(s, (sockaddr*)&sockAddr, sizeof(sockAddr)))
	{
		GetChatterMgr()->DoServerMessage("连接失败,请确认 [" + CString(lpUser->m_name) + "] 有无真实IP!");
		closesocket(s);
		return 0;
	}

	//发送的数据:用户名长度[4个字节]+用户名称

	//获得文件信息
	char buf[1024] = "\0";

	//发送的数据
	int nPointer = 0;
	int nSize = sizeof(int);

	int nTmpLength;
	//用户名长度
	nTmpLength = strlen(GetChatterMgr()->GetUser()->m_name);
	memcpy(buf + nPointer, &nTmpLength, nSize);
	nPointer += nSize;

	//用户名
	sprintf(buf + nPointer, "%s", GetChatterMgr()->GetUser()->m_name);
	nPointer += strlen(GetChatterMgr()->GetUser()->m_name);

	if (SOCKET_ERROR == send(s, buf, nPointer, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("异常原因导致传送文件==> [" + CString(lpUser->m_name) + "]失败!");
		closesocket(s);
		return 0;
	}

	//获得接收或拒绝通知
	if (1 != recv(s, buf, 1, MSG_PARTIAL))
	{
		StopSound();
		GetChatterMgr()->DoServerMessage("[" + CString(lpUser->m_name) + "] 拒绝接收语音!");
		closesocket(s);
		return 0;
	}

	GetChatterMgr()->DoServerMessage("[" + CString(lpUser->m_name) + "] 同意语音通话,正在语音聊天中...");

	DWORD nPos = 0;
	DWORD nTime = GetTickCount();
	DWORD nTimeStart = nTime;
	do
	{

		CSingleLock lock(&GetChatterMgr()->_SoundMutex, TRUE);
		if (GetChatterMgr()->SoundBuf[0] == 0 && GetChatterMgr()->SoundBuf[1] == 0 &&
			GetChatterMgr()->SoundBuf[2] == 0 && GetChatterMgr()->SoundBuf[3] == 0 &&
			GetChatterMgr()->SoundBuf[4] == 0 && GetChatterMgr()->SoundBuf[4] == 0)
		{
			lock.Unlock();
			continue;
		}

		send(s, GetChatterMgr()->SoundBuf, MAX_BUFFER_SIZE, MSG_PARTIAL);
		memset(GetChatterMgr()->SoundBuf, 0, MAX_BUFFER_SIZE);
		lock.Unlock();

	} while (IsRun);


	closesocket(s);

	GetChatterMgr()->DoServerMessage("语音聊天==>" + CString(lpUser->m_name) + "] 结束...");
	return 0;

}
UINT ReceiveSoundFunc(LPVOID pParam)
{
	SOCKET s = (SOCKET)pParam;

	//接收的数据:用户名长度[4个字节]+用户名称

	char buf[MAX_BUFFER_SIZE] = "\0";
	char userName[20] = "\0";
	int nNameLength = 1;

	int nSize = 4;

	//用户名长度
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nNameLength, buf, nSize);

	//获得用户名称
	recv(s, userName, nNameLength, MSG_PARTIAL);

	CString szNotify;
	szNotify = "[" + CString(userName) + "] 要与你语音  ,接收 或 拒绝?";

	GetChatterMgr()->DoServerMessage(szNotify);

	if (AfxMessageBox(szNotify, MB_YESNO) == IDNO)
	{
		StopSound();
		closesocket(s);
		return 0;
	}

	//发送接收通知
	send(s, "T", 1, MSG_PARTIAL);

	GetChatterMgr()->DoServerMessage("语音聊天进行中 ....");

	CChatterDlg::GetInitializePtr()->_RunSoundFlag = 1;
	while (IsRun)
	{
		int nResult = recv(s, buf, MAX_BUFFER_SIZE, MSG_PARTIAL);

		switch (nResult)
		{
			case 0:
				IsRun = FALSE;
				StopSound();
				break;
			case SOCKET_ERROR:
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					PRINTDEBUG(FALSE);
					IsRun = FALSE;
				}
				else
				{
					break;
				}
			default:
			{
				memcpy(GetChatterMgr()->_sound.m_cBufferOut, buf, MAX_BUFFER_SIZE);
			}

		}


	}

	closesocket(s);

	GetChatterMgr()->DoServerMessage("[" + CString(userName) + "] 语音聊天结束...");

	return 0;
}
