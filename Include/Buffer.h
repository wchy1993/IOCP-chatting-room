

#if !defined(AFX_BUFFER_H__24DD7377_C307_4C0B_87B4_E23481B7AA5F__INCLUDED_)
#define AFX_BUFFER_H__24DD7377_C307_4C0B_87B4_E23481B7AA5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBuffer  
{
protected:
	PCHAR	m_pBase;	//����Ļ�ַ
	PCHAR	m_pPtr;		//ָ��û�б������ݵĻ����ַ
	UINT	m_nSize;	//����ĳ���
	// Methods
protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);
	UINT DeAllocateBuffer(UINT nRequestedSize);
	UINT GetMemSize();	
public:
	int Scan(const CHAR scan, UINT nPos);
	void ClearBuffer();
	
	UINT Delete(UINT nSize);
	UINT Read(PCHAR pData, UINT nSize);
	BOOL Write(LPCSTR pData, UINT nSize);
	BOOL Write(CString& strData);
	UINT GetBufferLen();
	int Scan(PCHAR pScan,UINT nPos);
	BOOL Insert(PCHAR pData, UINT nSize);
	BOOL Insert(CString& strData);
	
	void Copy(CBuffer& buffer);	
	
	PCHAR GetBuffer(UINT nPos=0);
	
	CBuffer();
	~CBuffer();
	
	void FileWrite(const CString& strFileName);
};

#endif // !defined(AFX_BUFFER_H__24DD7377_C307_4C0B_87B4_E23481B7AA5F__INCLUDED_)
