
#if !defined(AFX_TRANSFERPROGRESS_H__4C0FDD36_5E57_4749_9F22_545B420DD36C__INCLUDED_)
#define AFX_TRANSFERPROGRESS_H__4C0FDD36_5E57_4749_9F22_545B420DD36C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransferProgress.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTransferProgress window

class CTransferProgress
{
// Construction
public:
	CTransferProgress();

	CProgressCtrl	m_progress;
	CStatic			m_label;
	CStatic			m_velocity;

	DWORD			m_nStep;
	CFont			m_font;
	CDC				*m_pDC;


	static int m_nHeight;
// Attributes
public:

// Operations
public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd);
	void SetRange(DWORD nMaxRange);
	void SetPos(DWORD nStep);
	void SetLabel(LPCSTR szLabel);
	void SetVelocity(DWORD nVelocity);
// Overrides

// Implementation
public:
	void MovePlace(CRect& rect);
	virtual ~CTransferProgress();

protected:

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSFERPROGRESS_H__4C0FDD36_5E57_4749_9F22_545B420DD36C__INCLUDED_)
