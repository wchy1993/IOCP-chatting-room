; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CChatterDlg
LastTemplate=generic CWnd
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "chatter.h"
LastPage=0

ClassCount=13
Class1=CChatterApp
Class2=CAboutDlg
Class3=CChatterDlg
Class4=CClientSocket
Class5=CConnectDlg
Class6=CLoginDlg

ResourceCount=6
Resource1=IDD_CHATTER_DIALOG
Resource2=IDD_ABOUTBOX
Resource3=IDD_CONNECTING
Resource4=IDD_LOGIN
Class7=CJEdit
Resource5=IDR_MAINMENU
Class8=CListBoxEx
Class9=CListCtrlEx
Class10=CFileSocket
Class11=CSendFileThread
Class12=CReceiveSocket
Class13=CTransferProgress
Resource6=IDR_USER

[CLS:CChatterApp]
Type=0
BaseClass=CWinApp
HeaderFile=Chatter.h
ImplementationFile=Chatter.cpp
LastObject=CChatterApp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=ChatterDlg.cpp
ImplementationFile=ChatterDlg.cpp
LastObject=CAboutDlg

[CLS:CChatterDlg]
Type=0
BaseClass=CDialog
HeaderFile=ChatterDlg.h
ImplementationFile=ChatterDlg.cpp
Filter=W
VirtualFilter=dWC
LastObject=CChatterDlg

[CLS:CClientSocket]
Type=0
BaseClass=CAsyncSocket
HeaderFile=ClientSocket.h
ImplementationFile=ClientSocket.cpp

[CLS:CConnectDlg]
Type=0
BaseClass=CDialog
HeaderFile=ConnectDlg.h
ImplementationFile=ConnectDlg.cpp
LastObject=CConnectDlg

[CLS:CLoginDlg]
Type=0
BaseClass=CDialog
HeaderFile=LoginDlg.h
ImplementationFile=LoginDlg.cpp
LastObject=IDOK
Filter=D
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_CHATTER_DIALOG]
Type=1
Class=CChatterDlg
ControlCount=7
Control1=ID_SEND,button,1342242817
Control2=IDC_LISTUSERS,SysListView32,1350631424
Control3=IDC_SAYS,edit,1352734724
Control4=IDC_SAY,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_COMBOUSERS,combobox,1344339971
Control7=IDC_STATIC,static,1342308352

[DLG:IDD_CONNECTING]
Type=1
Class=CConnectDlg
ControlCount=1
Control1=IDC_HINT,static,1342308352

[DLG:IDD_LOGIN]
Type=1
Class=CLoginDlg
ControlCount=8
Control1=IDC_STATIC,static,1342308352
Control2=IDC_USER,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_SERVER,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_PORT,edit,1350631424
Control7=IDOK,button,1342242817
Control8=IDCANCEL,button,1342242816

[MNU:IDR_MAINMENU]
Type=1
Class=?
Command1=ID_SENDFILE
Command2=ID_EXIT
CommandCount=2

[CLS:CJEdit]
Type=0
HeaderFile=JEdit.h
ImplementationFile=JEdit.cpp
BaseClass=CEdit
Filter=W
LastObject=CJEdit
VirtualFilter=WC

[MNU:IDR_USER]
Type=1
Class=?
Command1=ID_SENDFILE
CommandCount=1

[CLS:CListBoxEx]
Type=0
HeaderFile=ListBoxEx.h
ImplementationFile=ListBoxEx.cpp
BaseClass=CListBox
Filter=W
LastObject=CListBoxEx

[CLS:CListCtrlEx]
Type=0
HeaderFile=ListCtrlEx.h
ImplementationFile=ListCtrlEx.cpp
BaseClass=CListCtrl
Filter=W
VirtualFilter=FWC
LastObject=CListCtrlEx

[CLS:CFileSocket]
Type=0
HeaderFile=FileSocket.h
ImplementationFile=FileSocket.cpp
BaseClass=CAsyncSocket
Filter=N
LastObject=CFileSocket
VirtualFilter=q

[CLS:CSendFileThread]
Type=0
HeaderFile=SendFileThread.h
ImplementationFile=SendFileThread.cpp
BaseClass=CWinThread
Filter=N
LastObject=CSendFileThread
VirtualFilter=TC

[CLS:CReceiveSocket]
Type=0
HeaderFile=ReceiveSocket.h
ImplementationFile=ReceiveSocket.cpp
BaseClass=CAsyncSocket
Filter=N
VirtualFilter=q
LastObject=CReceiveSocket

