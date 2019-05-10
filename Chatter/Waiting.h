#if !defined(AFX_WAITING_H__63579D91_E8C9_4A20_A1BF_C88A6B2D1763__INCLUDED_)
#define AFX_WAITING_H__63579D91_E8C9_4A20_A1BF_C88A6B2D1763__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConnectDlg;

class CWaiting  
{
private:
	CConnectDlg *m_pDlg;
public:
	CWaiting();
	virtual ~CWaiting();

	void BeginWaiting();
	void EndWaiting();
	void SetPrompt(LPCSTR sPrompt);
	BOOL DoWaiting(HANDLE hEvent, DWORD nDelay);

}; 

#endif // !defined(AFX_WAITING_H__63579D91_E8C9_4A20_A1BF_C88A6B2D1763__INCLUDED_)
