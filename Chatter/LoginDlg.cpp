
#include "stdafx.h"
#include "Chatter.h"
#include "LoginDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_nPort = 8888;
	m_strUser = _T("Jerry");
	m_strServer = _T("127.0.0.1");
	//}}AFX_DATA_INIT
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Text(pDX, IDC_PORT, m_nPort);
	DDX_Text(pDX, IDC_USER, m_strUser);
	DDX_Text(pDX, IDC_SERVER, m_strServer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

void CLoginDlg::OnOK() 
{
	UpdateData();
	m_strUser.TrimLeft();
	m_strUser.TrimRight();
	UpdateData(FALSE);
	if(m_strUser.GetLength() == 0)
	{
		AfxMessageBox("用户名不能为空!", MB_OK | MB_ICONINFORMATION );
		return ;
	}
/*
	CString str = m_strUser.SpanExcluding(INVALID_USER_NAME);	
	if(str.Compare(m_strUser)!=0)
	{
		AfxMessageBox(CString("用户名中间不能包含空格")+INVALID_USER_NAME, MB_OK | MB_ICONINFORMATION );
		return ;		
	}
*/
	CDialog::OnOK();
}
