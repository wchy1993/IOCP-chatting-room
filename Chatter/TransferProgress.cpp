

#include "stdafx.h"
#include "chatter.h"
#include "TransferProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransferProgress

int CTransferProgress::m_nHeight = 15;

CTransferProgress::CTransferProgress()
{
	VERIFY(m_font.CreateFont(
		12,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"ËÎÌו"));                 // lpszFacename
	
	m_pDC = NULL;
}

CTransferProgress::~CTransferProgress()
{
	delete m_pDC;
}


/////////////////////////////////////////////////////////////////////////////
// CTransferProgress message handlers

BOOL CTransferProgress::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd) 
{
	CRect progressRect(rect), labelRect(rect), velocityRect(rect);

	labelRect.right -= 150;
	velocityRect.left +=160;
	velocityRect.right -= 30;
	progressRect.left += 200;
	
	m_progress.Create(dwStyle, progressRect, pParentWnd, 1);
	m_label.Create("", dwStyle, labelRect, pParentWnd, 0xffff);
	m_velocity.Create("", dwStyle, labelRect, pParentWnd, 0xffff);

	m_pDC = new CClientDC(&m_label);
	m_pDC->SelectObject(&m_font);
	
	return TRUE;
}

void CTransferProgress::SetRange(DWORD nMaxRange)
{
	m_nStep = nMaxRange/100;
	m_progress.SetRange32(0, 100);
}
void CTransferProgress::SetPos(DWORD nPos)
{
	nPos = nPos/m_nStep;
	m_progress.SetPos(nPos);
}

void CTransferProgress::SetVelocity(DWORD nVelocity)
{
	m_velocity.SetFont(&m_font);
	CString str;
	str.Format("%d k/s", nVelocity);
	m_velocity.SetWindowText(str);

}
void CTransferProgress::SetLabel(LPCSTR szLabel)
{	
	CRect rect;
	m_label.GetClientRect(&rect);

	rect.bottom -= 1;
//	rect.right -= 40;
	CSize size = m_pDC->GetTextExtent(szLabel);
	
	CString str(szLabel);
	CString s(str);

	if(rect.Width()<size.cx)
	{
		s = str.Left((int)(1.0*rect.Width()/size.cx*str.GetLength()));
		s.SetAt(s.GetLength()-1, '.');
		s.SetAt(s.GetLength()-2, '.');
		s.SetAt(s.GetLength()-3, '.');
	}
	
	m_label.SetFont(&m_font);
	m_label.SetWindowText(s);
}

void CTransferProgress::MovePlace(CRect &rect)
{
	CRect progressRect(rect), labelRect(rect), velocityRect(rect);
	
	labelRect.right = labelRect.left+200;

	progressRect.left = labelRect.right + 10;
	progressRect.right = progressRect.left + rect.Width()/3;

	velocityRect.left = progressRect.right + 10;

	progressRect.top += 5;
	m_progress.MoveWindow(progressRect);
	m_label.MoveWindow(labelRect);
	m_velocity.MoveWindow(velocityRect);
	
}

