
#if !defined(AFX_CHATTER_H__721A56B0_8F40_4BCB_B0FF_49E9AECCBAD3__INCLUDED_)
#define AFX_CHATTER_H__721A56B0_8F40_4BCB_B0FF_49E9AECCBAD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CChatterApp:
// See Chatter.cpp for the implementation of this class
//

class CChatterMgr;

CChatterMgr *GetChatterMgr();
#define PRINTDEBUG(a) PrintError(#a,__FILE__,__LINE__,GetLastError())

int PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum);


class CChatterApp : public CWinApp
{
public:
	CChatterApp();
	void Debug();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CChatterApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATTER_H__721A56B0_8F40_4BCB_B0FF_49E9AECCBAD3__INCLUDED_)
