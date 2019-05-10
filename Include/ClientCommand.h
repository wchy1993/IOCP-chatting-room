

#if !defined(AFX_CLIENTCOMMAND_H__874C0FF7_3574_442D_AE84_075B56E420E7__INCLUDED_)
#define AFX_CLIENTCOMMAND_H__874C0FF7_3574_442D_AE84_075B56E420E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLCommand.h"
#include <string>
#include <iostream>

template<class Type>
class CClientCommand : public CXMLCommand<Type>  
{
	typedef void (Type::*PLOGOUTFUNC)(USERINFO &user);
	typedef void (Type::*PNEWUSERFUNC)(USERINFO &user);
	typedef void (Type::*PUSERMESSAGEFUNC)(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
	typedef void (Type::*PLOGINSUCCESSFUNC)(USERINFO &user, MAPUSER *lpMapUser );
	
private:
	PLOGOUTFUNC			m_pLogoutFunc;
	PNEWUSERFUNC		m_pNewUserFunc;
	PUSERMESSAGEFUNC	m_pUserMessageFunc;
	PLOGINSUCCESSFUNC	m_pLoginSuccessFunc;	
public:
	//处理信息
	virtual BOOL DoProcessNetPacket(LONG userID, CString &strXML);
	//构建XML格式
	CString CreateUserSpeak(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
	CString CreateUserLogin(USERINFO &user);
	CString	CreateRequsetSF(USERINFO &user);	

	//设置callback函数
	void SetNewUserFunc(PNEWUSERFUNC pNewUserFunc){m_pNewUserFunc = pNewUserFunc;}
	void SetUserMessageFunc(PUSERMESSAGEFUNC pUserMessageFunc){m_pUserMessageFunc = pUserMessageFunc;}
	void SetLogoutFunc(PLOGOUTFUNC pLogoutFunc){m_pLogoutFunc = pLogoutFunc;}
	void SetLoginSuccessFunc(PLOGINSUCCESSFUNC pLoginSuccessFunc){m_pLoginSuccessFunc = pLoginSuccessFunc;}
	
	CClientCommand()
	{
		m_pLogoutFunc = NULL;
		m_pNewUserFunc = NULL;
		m_pUserMessageFunc = NULL;
		m_pLoginSuccessFunc = NULL;	
	}
	virtual ~CClientCommand(){}

};

//<REQUESTSF ID="发送者ID" NAME="发送者名称"/>
template<class Type>
CString	CClientCommand<Type>::CreateRequsetSF(USERINFO &user)
{
	MSXML::IXMLDOMDocumentPtr docPtr = CreateUserInfoTag(_bstr_t(REQUESTSF), user);
	return CreateNetPacket(docPtr);
}
/*
<NET>
<USERSPEAK ID="发言者id" NAME="用户名">
<USER ID="接收者id" NAME="用户名">发内容</USER>
</USERSPEAK>
</NET>
接收者id=0表示发给所有用户
*/
template<class Type>
CString CClientCommand<Type>::CreateUserSpeak(USERINFO &userSrc,USERINFO &userDest, CString strSpeak)
{
	MSXML::IXMLDOMDocumentPtr docPtr = CreateUserInfoTag(_bstr_t(USERSPEAK), userSrc);
	MSXML::IXMLDOMDocumentPtr userPtr = CreateUserInfoTag(_bstr_t(USER), userDest);

	userPtr->documentElement->text = _bstr_t(strSpeak);

	docPtr->documentElement->appendChild(userPtr->documentElement);
	return CreateNetPacket(docPtr);
}

//<NET><USERLOGIN  ID="0" NAME="用户名"/></NET>
template<class Type>
CString CClientCommand<Type>::CreateUserLogin(USERINFO &user)
{	
	user.m_id = 0;
	return CreateNetPacket(CreateUserInfoTag(_bstr_t(USERLOGIN), user, TRUE));

}

//处理信息
template<class Type>
BOOL CClientCommand<Type>::DoProcessNetPacket(LONG userID, CString &strXML)
{
	MSXML::IXMLDOMDocumentPtr docPtr;
	
	//生成文档	
	docPtr.CreateInstance(MSXML::CLSID_DOMDocument);
	
	//格式不对
	if(!docPtr->loadXML(_bstr_t(strXML)))	return FALSE;
	
	MSXML::IXMLDOMNodePtr nodePtr = docPtr->documentElement->firstChild;
	
	if(nodePtr->GetnodeName() == _bstr_t(USERLOGOUT))
	{
		if(m_pLogoutFunc == NULL) return FALSE;
		
		USERINFO user;
		memset(&user, 0, sizeof(user));
		if(!GetUserInfoFromNode(nodePtr, user))
			return FALSE;

		//call back
		(m_pType->*m_pLogoutFunc)(user);
	}
	else if(nodePtr->GetnodeName() == _bstr_t(NEWUSER))
	{
		if(m_pNewUserFunc == NULL) return FALSE;
		
		LPUSERINFO lpUser = new USERINFO;
		memset(lpUser, 0, sizeof(USERINFO));
		if(!GetUserInfoFromNode(nodePtr, *lpUser, TRUE))
			return FALSE;

		//call back
		(m_pType->*m_pNewUserFunc)(*lpUser);
	}
	else if(nodePtr->GetnodeName() == _bstr_t(USERMESSAGE))
	{
		if(m_pUserMessageFunc == NULL) return FALSE;
		
		//发言人信息
		USERINFO userSrc;
		memset(&userSrc, 0, sizeof(userSrc));
		if(!GetUserInfoFromNode(nodePtr, userSrc)) 
			return FALSE;

		//收信息人信息
		MSXML::IXMLDOMNodePtr nodePtr1 = docPtr->selectSingleNode(_bstr_t("//"USERMESSAGE"//"USER));
		USERINFO userDest;
		memset(&userDest, 0, sizeof(userDest));
		if(!GetUserInfoFromNode(nodePtr1, userDest)) 
			return FALSE;		
		
		//call back
		(m_pType->*m_pUserMessageFunc)(userSrc, userDest, (char*)(nodePtr1->text));
	}
	else if(nodePtr->GetnodeName() == _bstr_t(LOGINSUCCESS))
	{
		if(m_pLoginSuccessFunc == NULL) return FALSE;
		
		//从服务器返回登陆用户的信息
		LPUSERINFO lpUser = new USERINFO;
		memset(lpUser, 0, sizeof(USERINFO));
		if(!GetUserInfoFromNode(nodePtr, *lpUser, TRUE)) 
			return FALSE;
		
		//其他用户的信息,在客户端自己释放
		MAPUSER *lpMapUsers = new MAPUSER;
		MSXML::IXMLDOMNodeListPtr nodeListPtr = docPtr->selectNodes(_bstr_t("//"LOGINSUCCESS"//"USERS"//"USER));
		for(int i=0;i<nodeListPtr->Getlength();i++)
		{
			MSXML::IXMLDOMNodePtr nodePtr = nodeListPtr->Getitem(i);
			
			LPUSERINFO pUser = new USERINFO; 
			memset(pUser, 0, sizeof(USERINFO));
			
			if(!GetUserInfoFromNode(nodePtr, *pUser, TRUE))
			{
				delete pUser;
				return FALSE;
			}
			
			(*lpMapUsers)[pUser->m_id] = pUser;
		}
		
		//call back
		(m_pType->*m_pLoginSuccessFunc)(*lpUser, lpMapUsers);		
		
	}
	
	return TRUE;
}




#endif // !defined(AFX_CLIENTCOMMAND_H__874C0FF7_3574_442D_AE84_075B56E420E7__INCLUDED_)
