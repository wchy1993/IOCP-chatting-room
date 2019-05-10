#if !defined(AFX_XMLCOMMAND_H__468D863E_C56E_435B_9D2D_57CE71FC5037__INCLUDED_)
#define AFX_XMLCOMMAND_H__468D863E_C56E_435B_9D2D_57CE71FC5037__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import <msxml.dll> named_guids 
using namespace MSXML;

#include "user.h"
#include "buffer.h"

//element
#define USERLOGIN		"USERLOGIN"
#define USERLOGOUT		"USERLOGOUT"
#define NEWUSER			"NEWUSER"
#define USERSPEAK		"USERSPEAK"
#define USERMESSAGE		"USERMESSAGE"
#define LOGINSUCCESS	"LOGINSUCCESS"
#define USERS			"USERS"
#define USER			"USER"

#define REQUESTSF		"REQUESTSF"
#define OK				"OK"
#define NO				"NO"
//attribute
#define NAME			"NAME"
#define ID				"ID"
#define IP				"IP"
#define PORT			"PORT"
#define FPORT			"FPORT"
#define SPORT			"SPORT"

//net packet
#define NET				"NET"
#define TAGNET1			"<"NET">"
#define TAGNET2			"</"NET">"

template<class Type>
class CXMLCommand  
{
protected:
	Type*	m_pType;
	
protected:
	//构建XML格式
	CString CreateNetPacket(MSXML::IXMLDOMDocumentPtr docPtr);
	//根据tag,产生用户信息
	MSXML::IXMLDOMDocumentPtr CreateUserInfoTag(_bstr_t &tag, USERINFO &user, BOOL bIP = FALSE);
	//从节点中获得用户数据,节点包括userinfo的属性:ID和NAME属性
	BOOL GetUserInfoFromNode(MSXML::IXMLDOMNodePtr nodePtr, USERINFO &user, BOOL bIP = FALSE);
	
public:
	CXMLCommand(){};
	virtual ~CXMLCommand(){};
	void SetType(Type* pType){m_pType = pType;};
	
	//从缓冲区中读取,是否存在完整的xml指令,即取出<NET>...</NET>
	BOOL HasNetPacket(CBuffer &buffer, CString &strXML);
	//获得输入指令
	CString GetCommand(){return CString((char*)m_out);}
	//处理信息,由子类重写
	virtual BOOL DoProcessNetPacket(LONG userID, CString &strXML) = 0;	
	
};

//<tag ID="用户id" NAME="用户名" [IP="IP地址" PORT="port" FPORT="fport"]/>
template<class Type>
MSXML::IXMLDOMDocumentPtr CXMLCommand<Type>::CreateUserInfoTag(_bstr_t &tag, USERINFO &user, BOOL bIP)
{
	MSXML::IXMLDOMDocumentPtr docPtr;
	MSXML::IXMLDOMElementPtr elementPtr;	
	
	//生成文档	
	docPtr.CreateInstance(MSXML::CLSID_DOMDocument);
	
	//根据tag生成根元素
	elementPtr = docPtr->createElement(tag);
	docPtr->documentElement = elementPtr;
	
	//设置用户属性
	elementPtr->setAttribute(_bstr_t(ID), _variant_t(user.m_id));
	elementPtr->setAttribute(_bstr_t(NAME),_variant_t(user.m_name));
// 	elementPtr->setAttribute(_bstr_t(TYPE), _variant_t(user.m_type));
// 	elementPtr->setAttribute(_bstr_t(FILENAME), _variant_t(user.m_FileName));
	if(bIP)
	{
		elementPtr->setAttribute(_bstr_t(IP),_variant_t(user.m_ip));
		elementPtr->setAttribute(_bstr_t(PORT),_variant_t(user.m_nPort));
		elementPtr->setAttribute(_bstr_t(FPORT),_variant_t(user.m_nFilePort));
		elementPtr->setAttribute(_bstr_t(SPORT), _variant_t(user.m_nSoundPort));
// 		elementPtr->setAttribute(_bstr_t(TYPE), _variant_t(user.m_type));
// 		elementPtr->setAttribute(_bstr_t(FILENAME), _variant_t(user.m_FileName));
	}
	
	return docPtr;
}

//在网络上传输时,先打包<NET></NET>
template<class Type>
CString CXMLCommand<Type>::CreateNetPacket(MSXML::IXMLDOMDocumentPtr docPtr)
{
	MSXML::IXMLDOMElementPtr elementPtr = docPtr->createElement(_bstr_t(NET));
	elementPtr->appendChild(docPtr->documentElement);
	
	docPtr->documentElement = elementPtr;
	
	return (char*)(docPtr->xml);
}

template<class Type>
BOOL CXMLCommand<Type>::HasNetPacket(CBuffer &buffer, CString &strXML)
{
	int nPos1 = buffer.Scan(TAGNET1, 0);
	if(nPos1 == -1) return FALSE;
	//删除无效数据(不再NET包内部)
	buffer.Delete(nPos1);
	
	int nPos2 = buffer.Scan(TAGNET2, 0);
	
	
	if(nPos2 == -1) return FALSE;
	
	//分配足够的空间,多分配一个结尾符
	int nSize = nPos2 + strlen(TAGNET2)+1;
	PCHAR pBuf = new CHAR[nSize];
	memset(pBuf, 0, nSize);
	
	//从缓冲区中读入数据,并删除缓冲区内容
	buffer.Read(pBuf, nSize-1);
	
	strXML = pBuf;
	
	//释放空间
	free(pBuf);
	return TRUE;
	
}

//FALSE:指令格式不对
template<class Type>
BOOL CXMLCommand<Type>::GetUserInfoFromNode(MSXML::IXMLDOMNodePtr nodePtr, USERINFO &user, BOOL bIP)
{
	MSXML::IXMLDOMNamedNodeMapPtr nodeMapPtr = nodePtr->Getattributes();
	MSXML::IXMLDOMNodePtr nodePtr1 = nodeMapPtr->getNamedItem(_bstr_t(ID));
	if(nodePtr1 == NULL) return FALSE;
	MSXML::IXMLDOMNodePtr nodePtr2 = nodeMapPtr->getNamedItem(_bstr_t(NAME));
	if(nodePtr2 == NULL) return FALSE;
// 	MSXML::IXMLDOMNodePtr nodePtr3 = nodeMapPtr->getNamedItem(_bstr_t(TYPE));
// 	if (nodePtr3 == NULL)
// 		return FALSE;
// 	user.m_type = atol((char*)nodePtr3->text);
// 	MSXML::IXMLDOMNodePtr nodePtr4 = nodeMapPtr->getNamedItem(_bstr_t(FILENAME));
// 	if (nodePtr4 == NULL)
// 		return FALSE;
// 	strcpy(user.m_FileName, (char*)nodePtr4->text);

	user.m_id = atol((char*)nodePtr1->text);
	sprintf(user.m_name, (char *)nodePtr2->text);
	
	if(bIP)
	{
		MSXML::IXMLDOMNodePtr nodePtr3 = nodeMapPtr->getNamedItem(_bstr_t(IP));
		if(nodePtr3 == NULL) 
			return FALSE;	
		sprintf(user.m_ip, (char *)nodePtr3->text);

		MSXML::IXMLDOMNodePtr nodePtr4 = nodeMapPtr->getNamedItem(_bstr_t(PORT));
		if(nodePtr4 == NULL) 
			return FALSE;	
		user.m_nPort = atol((char*)nodePtr4->text);

		MSXML::IXMLDOMNodePtr nodePtr5 = nodeMapPtr->getNamedItem(_bstr_t(FPORT));
		if(nodePtr5 == NULL) 
			return FALSE;	
		user.m_nFilePort = atol((char*)nodePtr5->text);


		MSXML::IXMLDOMNodePtr nodePtr6 = nodeMapPtr->getNamedItem(_bstr_t(SPORT));
		if (nodePtr6 == NULL)
			return FALSE;
		user.m_nSoundPort = atol((char*)nodePtr6->text);
// 
// 		MSXML::IXMLDOMNodePtr nodePtr7 = nodeMapPtr->getNamedItem(_bstr_t(FILENAME));
// 		if (nodePtr7 == NULL)
// 			return FALSE;
// 		strcpy(user.m_FileName, (char*)nodePtr7->text);
	}




	
	return TRUE;
}


#endif // !defined(AFX_XMLCOMMAND_H__468D863E_C56E_435B_9D2D_57CE71FC5037__INCLUDED_)
