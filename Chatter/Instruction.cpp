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

//从缓冲区中得到一条合法指令,
//指令以两个字母开头,并以'\0'结尾
BOOL CInstruction::HasInstrunction(CBuffer &buffer)
{
	m_strInstruction.Empty();
	
	CHAR byte[] = "\0";
	
	//扫描是否有合法指令到达
	int nPos = buffer.Scan(byte, 0);
	//没有合法指令,返回
	if(nPos == 0) return FALSE;
	
	PCHAR pBuffer = new CHAR[nPos];
	memset(pBuffer, 0, nPos);
	
	int nSize = buffer.Read(pBuffer, nPos);
	//开头的字母小于2,非法指令
	if(nSize < 2)
	{
		delete pBuffer;
		return FALSE;
	}
	m_strInstruction = pBuffer;
	delete pBuffer;
	return TRUE;

}

//获得有效数据
LPCSTR CInstruction::GetValidMessage()
{
	return m_strInstruction.Mid(2);
}
//是否是新用户加入(New User)
BOOL CInstruction::IsNewUser()
{
	return (m_strInstruction.Left(2).CompareNoCase("NU")==0);
}
//是否是欢迎消息(Welcome)
BOOL CInstruction::IsWCMessage()
{
	return (m_strInstruction.Left(2).CompareNoCase("WC")==0);
}
//是否是所有用户列表(All Users)
BOOL CInstruction::IsAllUsers()
{
	return (m_strInstruction.Left(2).CompareNoCase("AU")==0);
}
//是否是新信息(New Information)
BOOL CInstruction::IsNewInformation()
{
	return (m_strInstruction.Left(2).CompareNoCase("NI")==0);
}
//是否是用户退出(User Exit)
BOOL CInstruction::IsUserExit()
{
	return (m_strInstruction.Left(2).CompareNoCase("UE")==0);
}
//获得发送给服务器的登陆指令(LOGIN)
LPCSTR CInstruction::GetLoginInstruction(LPCSTR strUser)
{
	return CString("LG")+strUser;
}
//获得发送给服务器的"话" (User say)
LPCSTR CInstruction::GetSayInstruction(LPCSTR strSay)
{
	return CString("US")+strSay;
}
