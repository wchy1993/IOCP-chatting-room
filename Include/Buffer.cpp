#include "stdafx.h"
#include "Buffer.h"
#include <math.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffer::CBuffer()
{
	// Initial size
	m_nSize = 0;

	m_pPtr = m_pBase = NULL;
}

CBuffer::~CBuffer()
{
	if (m_pBase)
		VirtualFree(m_pBase,0,MEM_RELEASE);
}
	
//д���ݵ�������
BOOL CBuffer::Write(LPCSTR pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());

	CopyMemory(m_pPtr,pData,nSize);

	// Advance Pointer
	m_pPtr+=nSize;

	return nSize;
}

BOOL CBuffer::Insert(PCHAR pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());

	MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);
	CopyMemory(m_pBase,pData,nSize);

	// Advance Pointer
	m_pPtr+=nSize;

	return nSize;
}

//�ӻ����ж�������,��ɾ�����������ݵĻ���ռ�
UINT CBuffer::Read(PCHAR pData, UINT nSize)
{
	if (nSize > GetMemSize())	return 0;

	// all that we have 
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

		
	if (nSize)
	{
		// Copy over required amount and its not up to us
		// to terminate the buffer - got that!!!
		CopyMemory(pData,m_pBase,nSize);
		
		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());

	return nSize;
}

//���ػ���Ŀռ��С
UINT CBuffer::GetMemSize() 
{
	return m_nSize;
}
//���ػ��������ݵĿռ��С
UINT CBuffer::GetBufferLen() 
{
	if (m_pBase == NULL)
		return 0;

	int nSize = 
		m_pPtr - m_pBase;
	return nSize;
}

//�ٷ��仺�浽������С,���С�ڻ���������С,�򲻲���
UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetMemSize())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	// New Copy Data Over
	PCHAR pNewBuffer = (PCHAR)(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	if (m_pBase)
		VirtualFree(m_pBase,0,MEM_RELEASE);


	// Hand over the pointer
	m_pBase = pNewBuffer;

	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;

	m_nSize = nNewSize;

	return m_nSize;
}

//�ͷŻ���ռ䵽������С,���������СС�ڻ�������ݿռ�,�򲻲���.
UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	if (nNewSize < GetMemSize())
		return 0;

	// New Copy Data Over
	PCHAR pNewBuffer = (PCHAR)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	VirtualFree(m_pBase,0,MEM_RELEASE);

	// Hand over the pointer
	m_pBase = pNewBuffer;

	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;

	m_nSize = nNewSize;

	return m_nSize;
}

//��ջ���
void CBuffer::ClearBuffer()
{
	// Force the buffer to be empty
	m_pPtr = m_pBase;

	DeAllocateBuffer(1024);
}

//д����,�����ݿ����ʼд
BOOL CBuffer::Write(CString& strData)
{
	int nSize = strData.GetLength();
	return Write((PCHAR) strData.GetBuffer(nSize), nSize);
}

//�ӻ���ͷ����������
BOOL CBuffer::Insert(CString& strData)
{
	int nSize = strData.GetLength();
	return Insert((PCHAR) strData.GetBuffer(nSize), nSize);
}

//��������
void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();
	int nSize = buffer.GetBufferLen();
	ClearBuffer();
	ReAllocateBuffer(nReSize);

	m_pPtr = m_pBase + nSize;

	CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
}

//���ظ���λ�õĻ���
PCHAR CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase+nPos;
}


//�ѻ���д���ļ�
void CBuffer::FileWrite(const CString& strFileName)
{
	CFile file;

	if (file.Open(strFileName, CFile::modeWrite | CFile::modeCreate))
	{
		file.Write(m_pBase,GetBufferLen());
		file.Close();
	}
}

//�ӻ�����ɾ������
UINT CBuffer::Delete(UINT nSize)
{
	if (nSize > GetMemSize())
		return 0;

	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

	if (nSize)
	{
		//�ƶ��ڴ��
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());

	return nSize;
}


//�ҵ������е����ֽ�,����λ��,û���ҵ��򷵻�-1
int CBuffer::Scan(PCHAR pScan,UINT nPos)
{
	if (nPos > GetBufferLen() )
		return -1;
	
	PCHAR pStr = (PCHAR) strstr((char*)(m_pBase+nPos),(char*)pScan);
	
	int nOffset = -1;
	
	if (pStr)
		nOffset = pStr - m_pBase;
	
	return nOffset;
}

int CBuffer::Scan(const CHAR scan, UINT nPos)
{
	if (nPos > GetBufferLen() )
		return -1;

	for(UINT i=0;i<GetBufferLen()-nPos;i++)
	{
		if(m_pBase[i+nPos] == scan)
			return i+nPos;
	}

	return -1;	
}
