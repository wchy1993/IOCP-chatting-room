#if !defined(AFX_USER_H__2078F1CB_DD41_412C_B88D_F1E613054380__INCLUDED_)
#define AFX_USER_H__2078F1CB_DD41_412C_B88D_F1E613054380__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

typedef struct _USERINFO
{
	LONG		m_id;
	CHAR		m_name[20];	
	CHAR		m_ip[32];
	LONG		m_nPort;
	LONG		m_nFilePort;
	LONG		m_nSoundPort;
} *LPUSERINFO, USERINFO;

typedef struct _FILE_TRANSFER
{
	LPUSERINFO	m_lpUser;
	CHAR		m_fileName[256];
}FILETRANSFER, *LPFILETRANSFER;

typedef CMap<LONG, LONG, LPUSERINFO, LPUSERINFO> MAPUSER;

#endif // !defined(AFX_USER_H__2078F1CB_DD41_412C_B88D_F1E613054380__INCLUDED_)
