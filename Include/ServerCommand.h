

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
	//����call back ����
	typedef void (Type::*PLOGINFUNC)(USERINFO &user);
	typedef void (Type::*PUSERMESSAGEFUNC)(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
private:
	PLOGINFUNC			m_pLoginFunc;
	PUSERMESSAGEFUNC	m_pUserMessageFunc;
protected:
	MSXML::IXMLDOMDocumentPtr CreateUser(USERINFO &user);
public:
	//����net packet
	virtual BOOL DoProcessNetPacket(LONG userID, CString &strXML);
	
	//����XML��ʽ
	CString CreateNewUser(USERINFO &user);
	CString CreateLoginSuccess(USERINFO &user, MAPUSER* lpMapUsers);
	CString CreateUserLogout(USERINFO &user);
	CString CreateUserMessage(USERINFO &userSrc,USERINFO &userDest, CString strSpeak);
	
	//����callback����
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

//<NET><NEWUSER ID="�û�id" NAME="�û���" IP="IP��ַ" PORT="port"/></NET>
//�����û���½ʱ,���������õ�½�û����Է��͸������û�
template<class Type>
CString CServerCommand<Type>::CreateNewUser(USERINFO &user)
{
	return CreateNetPacket(CreateUserInfoTag(_bstr_t(NEWUSER), user, TRUE));
}

//<NET><USERLOGOUT ID="�û�id" NAME="�û���"/></NET>
template<class Type>
CString CServerCommand<Type>::CreateUserLogout(USERINFO &user)
{
	return CreateNetPacket(CreateUserInfoTag(_bstr_t(USERLOGOUT), user));	
}

/*
<NET>
<LOGINSUCCESS ID="�û�id" NAME="�û���" IP="IP��ַ" PORT="Port" FPORT="fport">
<USERS>
<USER ID="�û�id" NAME="�û���" IP="IP��ַ" PORT="Port" FPORT="fport"/>
<USER ID="�û�id" NAME="�û���" IP="IP��ַ" PORT="Port" FPORT="fport"/>
...
</USERS>
</LOGINSUCCESS>
</NET>
*/
template<class Type>
CString CServerCommand<Type>::CreateLoginSuccess(USERINFO &user, MAPUSER* lpMapUsers)
{
	MSXML::IXMLDOMDocumentPtr docPtr = CreateUserInfoTag(_bstr_t(LOGINSUCCESS), user, TRUE);

	//�����ӽڵ�
	MSXML::IXMLDOMElementPtr usersPtr = docPtr->createElement(_bstr_t(USERS));
	MSXML::IXMLDOMNodePtr usePtr = docPtr->GetdocumentElement()->appendChild(usersPtr);

	POSITION pos = lpMapUsers->GetStartPosition();
	for(;pos!=NULL;)
	{
		LONG id;
		LPUSERINFO pUser;
		lpMapUsers->GetNextAssoc(pos, id, pUser);
		
		//����û��б�
		usePtr->appendChild(CreateUserInfoTag(_bstr_t(USER), *pUser, TRUE)->documentElement);
	}

	return CreateNetPacket(docPtr);
}
/*
<NET>
<USERMESSAGE ID=�������û�id NAME=�û���>
<USER ID="������id" NAME="�û���">��������</USER>
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
	
	//�����ĵ�	
	docPtr.CreateInstance(MSXML::CLSID_DOMDocument);
	
	//�����ʽ����
	if(!docPtr->loadXML(_bstr_t(strXML)))	return FALSE;
	
	//ȡ��NET���һ���ڵ�����
	MSXML::IXMLDOMNodePtr nodePtr = docPtr->documentElement->firstChild;
	
	if(nodePtr->GetnodeName() == _bstr_t(USERLOGIN))
	{
		if(m_pLoginFunc == NULL) return FALSE;
		
		LPUSERINFO lpUser = new USERINFO;
		memset(lpUser, 0, sizeof(USERINFO));
		if(!GetUserInfoFromNode(nodePtr, *lpUser, TRUE))
			return FALSE;
		//�����û�id
		lpUser->m_id = userID;
		//call back
		(m_pType->*m_pLoginFunc)(*lpUser);
	}
	else if(nodePtr->GetnodeName() == _bstr_t(USERSPEAK))
	{
		if(m_pUserMessageFunc == NULL) return FALSE;
		
		//��������Ϣ
		USERINFO userSrc;
		memset(&userSrc, 0, sizeof(userSrc));
		if(!GetUserInfoFromNode(nodePtr, userSrc)) 
			return FALSE;
		
		//��������Ϣ
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
