// Instruction.cpp: implementation of the CInstruction class.
// Download by http://www.NewXing.com
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "chatter.h"
#include "Instruction.h"
#include "buffer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInstruction::CInstruction()
{

}

CInstruction::~CInstruction()
{

}

//�ӻ������еõ�һ���Ϸ�ָ��,
//ָ����������ĸ��ͷ,����'\0'��β
BOOL CInstruction::HasInstrunction(CBuffer &buffer)
{
	m_strInstruction.Empty();
	
	CHAR byte[] = "\0";
	
	//ɨ���Ƿ��кϷ�ָ���
	int nPos = buffer.Scan(byte, 0);
	//û�кϷ�ָ��,����
	if(nPos == 0) return FALSE;
	
	PCHAR pBuffer = new CHAR[nPos];
	memset(pBuffer, 0, nPos);
	
	int nSize = buffer.Read(pBuffer, nPos);
	//��ͷ����ĸС��2,�Ƿ�ָ��
	if(nSize < 2)
	{
		delete pBuffer;
		return FALSE;
	}
	m_strInstruction = pBuffer;
	delete pBuffer;
	return TRUE;

}

//�����Ч����
LPCSTR CInstruction::GetValidMessage()
{
	return m_strInstruction.Mid(2);
}
//�Ƿ������û�����(New User)
BOOL CInstruction::IsNewUser()
{
	return (m_strInstruction.Left(2).CompareNoCase("NU")==0);
}
//�Ƿ��ǻ�ӭ��Ϣ(Welcome)
BOOL CInstruction::IsWCMessage()
{
	return (m_strInstruction.Left(2).CompareNoCase("WC")==0);
}
//�Ƿ��������û��б�(All Users)
BOOL CInstruction::IsAllUsers()
{
	return (m_strInstruction.Left(2).CompareNoCase("AU")==0);
}
//�Ƿ�������Ϣ(New Information)
BOOL CInstruction::IsNewInformation()
{
	return (m_strInstruction.Left(2).CompareNoCase("NI")==0);
}
//�Ƿ����û��˳�(User Exit)
BOOL CInstruction::IsUserExit()
{
	return (m_strInstruction.Left(2).CompareNoCase("UE")==0);
}
//��÷��͸��������ĵ�½ָ��(LOGIN)
LPCSTR CInstruction::GetLoginInstruction(LPCSTR strUser)
{
	return CString("LG")+strUser;
}
//��÷��͸���������"��" (User say)
LPCSTR CInstruction::GetSayInstruction(LPCSTR strSay)
{
	return CString("US")+strSay;
}
