//////////////////////////////////////////////////////////////////////
// PckClass.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �й���ĳ�ʼ����
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

//#include "zlib.h"

#include "PckClass.h"

#pragma warning ( disable : 4996 )

void CPckClass::CPckClassInit()
{
	m_ReadCompleted = FALSE;

	m_lpPckIndexTable = NULL;
	m_firstFile = NULL;

	m_PckAllInfo.PckTail.dwFileCount = 0;

	memset(&m_RootNode, 0, sizeof(PCK_PATH_NODE));

	*m_PckAllInfo.PckIndexAddr.szAdditionalInfo = 0;
	*m_PckAllInfo.szFilename = 0;

	DWORD	dwCurrentPID = GetCurrentProcessId();

	sprintf_s(m_szEventAllWriteFinish, 16, TEXT_EVENT_WRITE_PCK_DATA_FINISH, dwCurrentPID);
	sprintf_s(m_szEventAllCompressFinish, 16, TEXT_EVENT_COMPRESS_PCK_DATA_FINISH, dwCurrentPID);
	sprintf_s(m_szEventMaxMemory, 16, TEXT_EVENT_PCK_MAX_MEMORY, dwCurrentPID);

	sprintf_s(m_szMapNameRead, 16, TEXT_MAP_NAME_READ, dwCurrentPID);
	sprintf_s(m_szMapNameWrite, 16, TEXT_MAP_NAME_WRITE, dwCurrentPID);

	m_lpThisPckKey = lpPckParams->lpPckVersion->getInitialKey();
}

CPckClass::CPckClass(LPPCK_RUNTIME_PARAMS inout)
{
	lpPckParams = inout;
	CPckClassInit();
}

CPckClass::~CPckClass()
{
	DeAllocMultiNodes(m_RootNode.child);

	if(NULL != m_lpPckIndexTable)
		free(m_lpPckIndexTable);	
}

BOOL CPckClass::Init(LPCTSTR	szFile)
{
	////test();

	lstrcpy(m_PckAllInfo.szFilename, szFile);

	if(MountPckFile(m_PckAllInfo.szFilename))
	{
		BuildDirTree();
		return m_ReadCompleted = TRUE;
	}else{
		return m_ReadCompleted = FALSE;
	}
	//return m_ReadCompleted = MountPckFile(m_Filename, isZupMode);
}

void CPckClass::SetPckVersion(int verID)
{
	m_lpThisPckKey = lpPckParams->lpPckVersion->GetKey(verID);
}

CONST	LPPCKINDEXTABLE CPckClass::GetPckIndexTable()
{
	return m_lpPckIndexTable;
}

CONST	LPPCK_PATH_NODE CPckClass::GetPckPathNode()
{
	return &m_RootNode;
}

CONST	LPPCKHEAD CPckClass::GetPckHead()
{
	return &m_PckAllInfo.PckHead;
}

DWORD CPckClass::GetPckFileCount()
{
	return m_PckAllInfo.PckTail.dwFileCount;
}

PCKADDR CPckClass::GetPckDataAreaSize()
{
	return m_PckAllInfo.dwAddressName - PCK_DATA_START_AT;
}

PCKADDR CPckClass::GetPckRedundancyDataSize()
{
	return m_PckAllInfo.dwAddressName - PCK_DATA_START_AT - m_RootNode.child->qdwDirCipherTextSize;
}

char * CPckClass::GetAdditionalInfo()
{
	return m_PckAllInfo.PckIndexAddr.szAdditionalInfo;
}

BOOL CPckClass::SetAdditionalInfo()
{
	if(0 == *m_PckAllInfo.szFilename)return FALSE;

	CMapViewFileWrite	*lpcWritefile = new CMapViewFileWrite();

	if(NULL == strstr(m_PckAllInfo.PckIndexAddr.szAdditionalInfo, PCK_ADDITIONAL_INFO + 4))
	{
		strcpy(m_PckAllInfo.PckIndexAddr.szAdditionalInfo,	PCK_ADDITIONAL_INFO
															PCK_ADDITIONAL_INFO_STSM);
	}

	if(!lpcWritefile->OpenPck(m_PckAllInfo.szFilename, OPEN_EXISTING))
	{
		PrintLogE(TEXT_OPENWRITENAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);
		delete lpcWritefile;
		return FALSE;
	}

	lpcWritefile->SetFilePointer(-PCK_TAIL_OFFSET, FILE_END);
	
	if(!lpcWritefile->Write(&m_PckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR)))
	{
		PrintLogE(TEXT_WRITEFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcWritefile;
		return FALSE;
	}

	delete lpcWritefile;
	return TRUE;

}
