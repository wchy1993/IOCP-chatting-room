

#include "stdafx.h"
#include "Chatter.h"
#include "ChatterDlg.h"
#include "chattermgr.h"
#include "SoundTransfer.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatterDlg dialog

CChatterDlg* CChatterDlg::_this = NULL;

CChatterDlg::CChatterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatterDlg::IDD, pParent)
	, _RunSoundFlag(-1)
{
	//{{AFX_DATA_INIT(CChatterDlg)
	m_strSay = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

CChatterDlg::~CChatterDlg()
{
}

void CChatterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatterDlg)
	DDX_Control(pDX, IDC_COMBOUSERS, m_cbUsers);
	DDX_Control(pDX, IDC_SAYS, m_edit);
	DDX_Control(pDX, IDC_LISTUSERS, m_lsUsers);
	DDX_Text(pDX, IDC_SAY, m_strSay);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChatterDlg, CDialog)
	//{{AFX_MSG_MAP(CChatterDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_SEND, OnSend)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_NOTIFY(NM_RCLICK, IDC_LISTUSERS, OnRclickListusers)
	ON_COMMAND(ID_SENDFILE, OnSendfile)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ADDPROGRESS, OnAddProgress)
	ON_MESSAGE(WM_DELPROGRESS, OnDelProgress)
	ON_MESSAGE(WM_SETLABEL, OnSetLabel)
	ON_MESSAGE(WM_SETPOS, OnSetPos)
	ON_MESSAGE(WM_SETVELOCITY, OnSetVelocity)
	ON_COMMAND(ID_SENDSOUND, &CChatterDlg::OnSendsound)
	ON_MESSAGE(MM_WIM_DATA, &CChatterDlg::WriteBufferFull)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatterDlg message handlers

BOOL CChatterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitUserList();
	InitSayEdit();

	GetChatterMgr()->SetInterface(this);

	//登陆服务器
	if(!GetChatterMgr()->Login())
	{
		ExitProcess(0);
	}

	//获得IDC_SAYS的大小
	GetDlgItem(IDC_SAYS)->GetWindowRect(&m_rect);
	ScreenToClient(&m_rect);
	
	_this = this;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChatterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChatterDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChatterDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CChatterDlg::InitUserList()
{
	m_nAllIndex = m_cbUsers.AddString("所有人");
	m_cbUsers.SetItemData(m_nAllIndex, 0);
	m_cbUsers.SetCurSel(m_nAllIndex);
		
	DWORD dwStyle=GetWindowLong(m_lsUsers.GetSafeHwnd(),GWL_STYLE); 	
	if((dwStyle&LVS_TYPEMASK)!=LVS_REPORT) 
	{
		SetWindowLong(m_lsUsers.GetSafeHwnd(),GWL_STYLE,(dwStyle&~LVS_TYPEMASK)|LVS_REPORT);
	}
	m_lsUsers.InsertColumn(0,"用户列表",LVCFMT_LEFT,100,0); 
	m_lsUsers.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

}

void CChatterDlg::OnSend() 
{
	UpdateData();

	USERINFO user;
	memset(&user, 0, sizeof(user));
	
	user.m_id = m_cbUsers.GetItemData(m_cbUsers.GetCurSel());

	if(user.m_id != 0)
		m_cbUsers.GetLBText(user.m_id, user.m_name);
	else
		strcpy(user.m_name, "所有人");

	GetChatterMgr()->DoSay(user, m_strSay);	

	m_strSay= "";

	UpdateData(FALSE);
}


void CChatterDlg::AddUser(LPUSERINFO lpUser, BOOL bComboxAdd)
{
	LV_ITEM item; 
	memset(&item,0,sizeof(item)); 
	item.mask = LVIF_TEXT | LVIF_STATE;
	item.stateMask = 0xFFFF;		// get all state flags
	
	int nRow = m_lsUsers.InsertItem(&item);
	m_lsUsers.SetItemText(nRow, 0, lpUser->m_name);	
	m_lsUsers.SetItemData(nRow, lpUser->m_id);

	if(!bComboxAdd) return;
		
	m_cbUsers.SetItemData(m_cbUsers.AddString(lpUser->m_name), lpUser->m_id);
}

void CChatterDlg::DeleteUser(USERINFO &user)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM ;
	find.lParam = user.m_id;
	int index;
	if((index = m_lsUsers.FindItem(&find, -1)) != -1)
	{
		m_lsUsers.DeleteItem(index);
	}

	for(index=0;index<m_cbUsers.GetCount();index++)
	{
		if(m_cbUsers.GetItemData(index) == (DWORD)user.m_id)
		{
			if(index == m_cbUsers.GetCurSel())
			{
				m_cbUsers.SetCurSel(m_nAllIndex);	
			}
			m_cbUsers.DeleteString(index);
			
			break;
		}
	}

}


void CChatterDlg::AddUserSay(LPCSTR strSay)
{
	CString str;
	m_edit.GetWindowText(str);
	str +="  ";
	str += strSay;
	str += "\r\n";
	m_edit.SetWindowText(str);
	
	m_edit.LineScroll(m_edit.GetLineCount()-1);
}

void CChatterDlg::OnExit() 
{
	PostQuitMessage(WM_CLOSE);
}

void CChatterDlg::InitSayEdit()
{

}

void CChatterDlg::AddServerMessage(LPCSTR strMsg)
{
	AddUserSay("** "+CString(strMsg)+" **");
}

void CChatterDlg::OnRclickListusers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//获得当前鼠标点
	CPoint pt, screenPt;
	DWORD dwPt = GetCursorPos(&pt);
	screenPt = pt;
	m_lsUsers.ScreenToClient(&pt);

	//是否点击了item
	m_lsUsers.HitTest(pt);
	UINT uFlags;
	int nItem = m_lsUsers.HitTest(pt, &uFlags);
	if (uFlags & LVHT_ONITEMLABEL)
	{
		CMenu menu;
		menu.LoadMenu(IDR_USER);
		CMenu *pSubMenu = menu.GetSubMenu(0);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, screenPt.x, screenPt.y, this);

	}
	
	*pResult = 0;
}


void CChatterDlg::OnSendfile() 
{
	int index = m_lsUsers. GetNextItem(-1, LVNI_SELECTED);
	if(index == -1)
	{
		AfxMessageBox("请选中接受文件的用户!");
		return;
	}

	CFileDialog fdlg(TRUE);
	if(fdlg.DoModal() == IDCANCEL)
		return;

	CString strFile = fdlg.GetPathName();
	AddServerMessage("正在等待用户响应 [ " + strFile + " ] 的传输...");
	
	DWORD userID = m_lsUsers.GetItemData(index);
	GetChatterMgr()->SendFile(userID, strFile);
	
}

// 语音聊天
void CChatterDlg::OnSendsound()
{


	// 正在
	if (_RunSoundFlag != -1)
	{
		StopSound();
		GetChatterMgr()->_sound.StopRecord();
		_RunSoundFlag = -1;
		return;
	}

	GetChatterMgr()->_sound.Init(this);
	_RunSoundFlag = m_lsUsers.GetNextItem(-1, LVNI_SELECTED);
	if (_RunSoundFlag == -1)
	{
		AfxMessageBox("请选中要语音聊天的用户!");
		return;
	}

	AddServerMessage("正在等待用户接受语音聊天的响应 ...");

	DWORD userID = m_lsUsers.GetItemData(_RunSoundFlag);
	GetChatterMgr()->SendSound(userID);
}


LRESULT CChatterDlg::OnAddProgress(WPARAM wParam, LPARAM lParam)
{

	//添加进度条
	CTransferProgress* pProgress = new CTransferProgress;
	pProgress->Create(WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this);
	pProgress->SetRange(lParam);

	m_mapProgress[wParam] = pProgress;

	//重新放置进度条
	LayTransferProgress();

	return 0;
	
}
LRESULT CChatterDlg::OnDelProgress(WPARAM wParam, LPARAM lParam)
{
	//删除进度条	
	delete m_mapProgress[wParam];
	m_mapProgress.RemoveKey(wParam);

	//重新放置进度条
	LayTransferProgress();
	return 0;
}

LRESULT CChatterDlg::OnSetLabel(WPARAM wParam, LPARAM lParam)
{
	m_mapProgress[wParam]->SetLabel((char*)lParam);
	return 0;
}
LRESULT CChatterDlg::OnSetPos(WPARAM wParam, LPARAM lParam)
{
	m_mapProgress[wParam]->SetPos(lParam);
	return 0;
}
LRESULT CChatterDlg::OnSetVelocity(WPARAM wParam, LPARAM lParam)
{
	m_mapProgress[wParam]->SetVelocity(lParam);
	return 0;
}

//放置进度条
void CChatterDlg::LayTransferProgress()
{
	CRect rect(m_rect);

	POSITION pos = m_mapProgress.GetStartPosition();	
	for(;pos!=NULL;)
	{
		DWORD index;
		CTransferProgress *pCtrl;
		m_mapProgress.GetNextAssoc(pos, index, pCtrl);

		rect.bottom -= CTransferProgress::m_nHeight;
		
		CRect ctrlRect(rect);
		ctrlRect.bottom = rect.bottom + CTransferProgress::m_nHeight;
		ctrlRect.top = ctrlRect.bottom - CTransferProgress::m_nHeight;

		pCtrl->MovePlace(ctrlRect);
	}

	GetDlgItem(IDC_SAYS)->MoveWindow(&rect);
}

LRESULT CChatterDlg::WriteBufferFull(WPARAM  lp, LPARAM wp)
{
	GetChatterMgr()->_sound.Play();//发出本地声音
	CSingleLock lock(&GetChatterMgr()->_SoundMutex, TRUE);

	memcpy(GetChatterMgr()->SoundBuf, GetChatterMgr()->_sound.m_cBufferIn, MAX_BUFFER_SIZE);

	
	GetChatterMgr()->_sound.FreeRecordBuffer();
	GetChatterMgr()->_sound.FreePlayBuffer();
	lock.Unlock();
	return 0;
}