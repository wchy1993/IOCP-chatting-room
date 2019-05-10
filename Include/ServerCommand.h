

#if !defined(AFX_SERVERCOMMAND_H__C2AA9EFC_E662_4141_AEFC_791A9E36DCBA__INCLUDED_)
#define AFX_SERVERCOMMAND_H__C2AA9EFC_E662_4141_AEFC_791A9E36DCBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XMLCommand.h"
using namespace std;

template<class Type>
class CServerCommand : public CXMLCommand<Type>
{
	//定义call back 函数
	typedef void (Type::*PLOGINFUNC)(USERINFO &user);
	typedef void (Type::*PUSERMESSAGEFUNC)(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
private:
	PLOGINFUNC			m_pLoginFunc;
	PUSERMESSAGEFUNC	m_pUserMessageFunc;
protected:
	MSXML::IXMLDOMDocumentPtr CreateUser(USERINFO &user);
public:
	//处理net packet
	virtual BOOL DoProcessNetPacket(LONG userID, CString &strXML);
	
	//构建XML格式
	CString CreateNewUser(USERINFO &user);
	CString CreateLoginSuccess(USERINFO &user, MAPUSER* lpMapUsers);
	CString CreateUserLogout(USERINFO &user);
	CString CreateUserMessage(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
	
	//设置callback函数
	void SetLoginFunc(PLOGINFUNC pLoginFunc){m_pLoginFunc = pLoginFunc;}
	void SetUserMessageFunc(PUSERMESSAGEFUNC pUserMessageFunc){m_pUserMessageFunc = pUserMessageFunc;}

	CServerCommand();
	~CServerCommand();
};
template<class Type>
CServerCommand<Type>::CServerCommand()
{
	m_pLoginFunc = NULL;
	m_pUserMessageFunc = NULL;
}
template<class Type>
CServerCommand<Type>::~CServerCommand()
{

}

//<NET><NEWUSER ID="用户id" NAME="用户名" IP="IP地址" PORT="port"/></NET>
//当有用户登陆时,服务器将该登陆用户属性发送给所有用户
template<class Type>
CString CServerCommand<Type>::CreateNewUser(USERINFO &user)
{
	return CreateNetPacket(CreateUserInfoTag(_bstr_t(NEWUSER), user, TRUE));
}

//<NET><USERLOGOUT ID="用户id" NAME="用户名"/></NET>
template<class Type>
CString CServerCommand<Type>::CreateUserLogout(USERINFO &user)
{
	return CreateNetPacket(CreateUserInfoTag(_bstr_t(USERLOGOUT), user));	
}

/*
<NET>
<LOGINSUCCESS ID="用户id" NAME="用户名" IP="IP地址" PORT="Port" FPORT="fport">
<USERS>
<USER ID="用户id" NAME="用户名" IP="IP地址" PORT="Port" FPORT="fport"/>
<USER ID="用户id" NAME="用户名" IP="IP地址" PORT="Port" FPORT="fport"/>
...
</USERS>
</LOGINSUCCESS>
</NET>
*/
template<class Type>
CString CServerCommand<Type>::CreateLoginSuccess(USERINFO &user, MAPUSER* lpMapUsers)
{
	MSXML::IXMLDOMDocumentPtr docPtr = CreateUserInfoTag(_bstr_t(LOGINSUCCESS), user, TRUE);

	//设置子节点
	MSXML::IXMLDOMElementPtr usersPtr = docPtr->createElement(_bstr_t(USERS));
	MSXML::IXMLDOMNodePtr usePtr = docPtr->GetdocumentElement()->appendChild(usersPtr);

	POSITION pos = lpMapUsers->GetStartPosition();
	for(;pos!=NULL;)
	{
		LONG id;
		LPUSERINFO pUser;
		lpMapUsers->GetNextAssoc(pos, id, pUser);
		
		//添加用户列表
		usePtr->appendChild(CreateUserInfoTag(_bstr_t(USER), *pUser, TRUE)->documentElement);
	}

	return CreateNetPacket(docPtr);
}
/*
<NET>
<USERMESSAGE ID=发言者用户id NAME=用户名>
<USER ID="接收者id" NAME="用户名">发言内容</USER>
</USERMESSAGE>
</NET>
*/
template<class Type>
CString CServerCommand<Type>::CreateUserMessage(USERINFO &userSrc,USERINFO &userDest, CString strSpeak)
{
	MSXML::IXMLDOMDocumentPtr docPtr = CreateUserInfoTag(_bstr_t(USERMESSAGE), userSrc);
	MSXML::IXMLDOMDocumentPtr userPtr = CreateUserInfoTag(_bstr_t(USER), userDest);
	userPtr->documentElement->text=_bstr_t(strSpeak);
	docPtr->documentElement->appendChild(userPtr->documentElement);
	return CreateNetPacket(docPtr);
}

template<class Type>
BOOL CServerCommand<Type>::DoProcessNetPacket(LONG userID, CString &strXML)
{
	MSXML::IXMLDOMDocumentPtr docPtr;
	
	//生成文档	
	docPtr.CreateInstance(MSXML::CLSID_DOMDocument);
	
	//命令格式出错
	if(!docPtr->loadXML(_bstr_t(strXML)))	return FALSE;
	
	//取得NET后第一个节点数据
	MSXML::IXMLDOMNodePtr nodePtr = docPtr->documentElement->firstChild;
	
	if(nodePtr->GetnodeName() == _bstr_t(USERLOGIN))
	{
		if(m_pLoginFunc == NULL) return FALSE;
		
		LPUSERINFO lpUser = new USERINFO;
		memset(lpUser, 0, sizeof(USERINFO));
		if(!GetUserInfoFromNode(nodePtr, *lpUser, TRUE))
			return FALSE;
		//设置用户id
		lpUser->m_id = userID;
		//call back
		(m_pType->*m_pLoginFunc)(*lpUser);
	}
	else if(nodePtr->GetnodeName() == _bstr_t(USERSPEAK))
	{
		if(m_pUserMessageFunc == NULL) return FALSE;
		
		//发言人信息
		USERINFO userSrc;
		memset(&userSrc, 0, sizeof(userSrc));
		if(!GetUserInfoFromNode(nodePtr, userSrc)) 
			return FALSE;
		
		//接受者信息
		MSXML::IXMLDOMNodePtr nodePtr = docPtr->selectSingleNode(_bstr_t("//"USERSPEAK"//")+_bstr_t(USER));
		USERINFO userDest;
		memset(&userDest, 0, sizeof(userDest));
		if(!GetUserInfoFromNode(nodePtr, userDest)) 
			return FALSE;		

		//call back
		(m_pType->*m_pUserMessageFunc)(userSrc,userDest, (char*)(nodePtr->text));
	}
	return TRUE;
}
#endif // !defined(AFX_SERVERCOMMAND_H__C2AA9EFC_E662_4141_AEFC_791A9E36DCBA__INCLUDED_)
