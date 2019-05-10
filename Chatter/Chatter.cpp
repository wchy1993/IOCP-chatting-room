#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include "Chatter.h"
#include "ChatterDlg.h"
#include "chattermgr.h"
#include "logindlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatterApp

BEGIN_MESSAGE_MAP(CChatterApp, CWinApp)
	//{{AFX_MSG_MAP(CChatterApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatterApp construction

CChatterApp::CChatterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CChatterApp object

CChatterApp theApp;
CChatterMgr *GetChatterMgr()
{
	return CChatterMgr::GetChatterMgr();
}

#define PRINTDEBUG(a) PrintError(#a,__FILE__,__LINE__,GetLastError())

int PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
{
	LPSTR lpBuffer;
	char errbuf[256];
	
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errnum,
		LANG_NEUTRAL,
		(LPTSTR)&lpBuffer,
		0,
		NULL );
	
	wsprintf(errbuf, "\nThe following call failed at line %d in %s:\n\n"
		"    %s\n\nReason: %s\n", lineno, filename, linedesc, lpBuffer);
	AfxMessageBox(errbuf);
	
	return errnum;
}

/////////////////////////////////////////////////////////////////////////////
// CChatterApp initialization

BOOL CChatterApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
//	Debug();
	
	CChatterDlg dlg;
	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CChatterApp::Debug()
{
	AllocConsole();	// you only get 1 console.
	
	// lovely hack to get the standard io (printf, getc, etc) to the new console. Pretty much does what the
	// C lib does for us, but when we want it, and inside of a Window'd app.
	// The ugly look of this is due to the error checking (bad return values. Remove the if xxx checks if you like it that way.
	
	DWORD astds[3]={STD_OUTPUT_HANDLE,STD_ERROR_HANDLE,STD_INPUT_HANDLE};
	FILE *atrgs[3]={stdout,stderr,stdin};
	for( int i=0; i<3; i++ ) 
	{
		long hand=(long)GetStdHandle(astds[i]);
		if( hand!=(long)INVALID_HANDLE_VALUE ) 
		{
			int osf=_open_osfhandle(hand,_O_TEXT);
			if( osf!=-1 ) 
			{
				FILE *fp=_fdopen(osf,(astds[i]==STD_INPUT_HANDLE) ? "r" : "w");
				if( fp!=NULL ) 
				{
					*(atrgs[i])=*fp;
					setvbuf(fp,NULL,_IONBF,0);
				}
			}
		}
	}
	
}