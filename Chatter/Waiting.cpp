#include "stdafx.h"
#include "Chatter.h"
#include "Waiting.h"
#include "connectdlg.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaiting::CWaiting()
{
	m_pDlg = new CConnectDlg;
	m_pDlg->Create(IDD_CONNECTING, NULL);
	m_pDlg->CenterWindow(NULL);
	m_pDlg->ShowWindow(SW_SHOW);
}

CWaiting::~CWaiting()
{
	m_pDlg->ShowWindow(SW_HIDE);
	m_pDlg->DestroyWindow();
	delete m_pDlg;
}

void CWaiting::BeginWaiting()
{
	m_pDlg->ShowWindow(SW_SHOW);
}
void CWaiting::EndWaiting()
{
	m_pDlg->ShowWindow(SW_HIDE);
		
}

void CWaiting::SetPrompt(LPCSTR sPrompt)
{
	m_pDlg->GetDlgItem(IDC_HINT)->SetWindowText(sPrompt);
}

BOOL CWaiting::DoWaiting(HANDLE hEvent, DWORD nDelay)
{
	DWORD sStart = GetTickCount();
	do 
	{
		MSG msg ; 
		
		//����Ϣ�����ж�ȡ����
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{ 
			DispatchMessage(&msg); 
		}		
		
		//����Ƿ����¼�����
		int result = MsgWaitForMultipleObjects(1, 
			&hEvent, 
			TRUE,
			0,
			QS_ALLEVENTS);
		
		if(result == WAIT_OBJECT_0)
		{ 
			//�¼�����,����TRUE
			return TRUE;
		}
		else
		{
			//windows��Ϣ���˻��߳�ʱ��:����ѭ��
			continue;
		} 

	} while(GetTickCount() - sStart < nDelay);
	
	AfxMessageBox("��ʱ");
	//��ʱ
	return FALSE;
}
