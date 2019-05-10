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

// ����ֹͣ
void StopSound()
{
	IsRun = false;
	CChatterDlg::GetInitializePtr()->_RunSoundFlag = -1;
	GetChatterMgr()->_sound.StopRecord();
//	GetChatterMgr()->_sound.StopPlay();
}

//�����ļ������߳�
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

//���������û�,�����ļ�
UINT ConnectSoundFunc(LPVOID pParam)
{
	IsRun = true;
	LPUSERINFO lpUser = (LPUSERINFO)pParam;
	//ȡ���û���Ϣ

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpUser->m_ip);
	sockAddr.sin_port = htons((u_short)lpUser->m_nSoundPort);

	//�����û�
	if (SOCKET_ERROR == connect(s, (sockaddr*)&sockAddr, sizeof(sockAddr)))
	{
		GetChatterMgr()->DoServerMessage("����ʧ��,��ȷ�� [" + CString(lpUser->m_name) + "] ������ʵIP!");
		closesocket(s);
		return 0;
	}

	//���͵�����:�û�������[4���ֽ�]+�û�����

	//����ļ���Ϣ
	char buf[1024] = "\0";

	//���͵�����
	int nPointer = 0;
	int nSize = sizeof(int);

	int nTmpLength;
	//�û�������
	nTmpLength = strlen(GetChatterMgr()->GetUser()->m_name);
	memcpy(buf + nPointer, &nTmpLength, nSize);
	nPointer += nSize;

	//�û���
	sprintf(buf + nPointer, "%s", GetChatterMgr()->GetUser()->m_name);
	nPointer += strlen(GetChatterMgr()->GetUser()->m_name);

	if (SOCKET_ERROR == send(s, buf, nPointer, MSG_PARTIAL))
	{
		GetChatterMgr()->DoServerMessage("�쳣ԭ���´����ļ�==> [" + CString(lpUser->m_name) + "]ʧ��!");
		closesocket(s);
		return 0;
	}

	//��ý��ջ�ܾ�֪ͨ
	if (1 != recv(s, buf, 1, MSG_PARTIAL))
	{
		StopSound();
		GetChatterMgr()->DoServerMessage("[" + CString(lpUser->m_name) + "] �ܾ���������!");
		closesocket(s);
		return 0;
	}

	GetChatterMgr()->DoServerMessage("[" + CString(lpUser->m_name) + "] ͬ������ͨ��,��������������...");

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

	GetChatterMgr()->DoServerMessage("��������==>" + CString(lpUser->m_name) + "] ����...");
	return 0;

}
UINT ReceiveSoundFunc(LPVOID pParam)
{
	SOCKET s = (SOCKET)pParam;

	//���յ�����:�û�������[4���ֽ�]+�û�����

	char buf[MAX_BUFFER_SIZE] = "\0";
	char userName[20] = "\0";
	int nNameLength = 1;

	int nSize = 4;

	//�û�������
	recv(s, buf, nSize, MSG_PARTIAL);
	memcpy(&nNameLength, buf, nSize);

	//����û�����
	recv(s, userName, nNameLength, MSG_PARTIAL);

	CString szNotify;
	szNotify = "[" + CString(userName) + "] Ҫ��������  ,���� �� �ܾ�?";

	GetChatterMgr()->DoServerMessage(szNotify);

	if (AfxMessageBox(szNotify, MB_YESNO) == IDNO)
	{
		StopSound();
		closesocket(s);
		return 0;
	}

	//���ͽ���֪ͨ
	send(s, "T", 1, MSG_PARTIAL);

	GetChatterMgr()->DoServerMessage("������������� ....");

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

	GetChatterMgr()->DoServerMessage("[" + CString(userName) + "] �����������...");

	return 0;
}
