

#if !defined(AFX_CHATTERDLG_H__3A5204A9_4B59_47CC_B395_13D17A59C47F__INCLUDED_)
#define AFX_CHATTERDLG_H__3A5204A9_4B59_47CC_B395_13D17A59C47F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CChatterDlg dialog

#include "user.h"
#include "listctrlex.h"
#include "TransferProgress.h"
#include "Resource.h"


#define WM_ADDPROGRESS	WM_USER+100
#define WM_DELPROGRESS	WM_USER+101
#define WM_SETLABEL		WM_USER+102
#define WM_SETPOS		WM_USER+103
#define WM_SETVELOCITY	WM_USER+104


class CChatterDlg : public CDialog
{
	CMap<DWORD, DWORD, CTransferProgress*, CTransferProgress*>	m_mapProgress;
	CRect m_rect;
	static CChatterDlg* _this;
public:
	static CChatterDlg* GetInitializePtr()
	{
		return _this;
	}

// Construction
public:
	void LayTransferProgress();
	void AddServerMessage(LPCSTR strMsg);
	void InitSayEdit();
	void AddUserSay(LPCSTR strSay);
	void InitUserList();
	void DeleteUser(USERINFO &user);
	void AddUser(LPUSERINFO lpUser, BOOL bComboxAdd=TRUE);

	int m_nAllIndex;
	CChatterDlg(CWnd* pParent = NULL);	// standard constructor
	~CChatterDlg();
// Dialog Data
	//{{AFX_DATA(CChatterDlg)
	enum { IDD = IDD_CHATTER_DIALOG };
	CComboBox	m_cbUsers;
	CEdit	m_edit;
	CListCtrlEx	m_lsUsers;
	CString	m_strSay;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CChatterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSend();
	afx_msg void OnExit();
	afx_msg void OnRclickListusers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSendfile();
	//}}AFX_MSG
	afx_msg LRESULT OnAddProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDelProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetLabel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetPos(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetVelocity(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT WriteBufferFull(WPARAM  lp, LPARAM wp);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSendsound();

	int _RunSoundFlag;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATTERDLG_H__3A5204A9_4B59_47CC_B395_13D17A59C47F__INCLUDED_)
