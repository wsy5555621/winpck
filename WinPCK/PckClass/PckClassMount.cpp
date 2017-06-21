//////////////////////////////////////////////////////////////////////
// PckClassMount.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �й���ĳ�ʼ����
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.27
//////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4996 )

#include "zlib.h"

#include "PckClass.h"


BOOL CPckClass::MountPckFile(LPCTSTR	szFile)
{

	DWORD	dwFileBytesRead;
	CMapViewFileRead	*lpcReadfile = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpcReadfile, szFile)){
		delete lpcReadfile;
		return FALSE;
	}

	//����m_PckHead��pck�ļ�ͷ
	if(!lpcReadfile->Read(&m_PckAllInfo.PckHead, sizeof(PCKHEAD)))
	{

		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcReadfile;
		return FALSE;
	}

	lpcReadfile->SetFilePointer(-PCK_TAIL_OFFSET, FILE_END);
	
	if(!lpcReadfile->Read(&m_PckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR)))
	{

		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcReadfile;
		return FALSE;
	}

	//��������Ϣ
	char	*lpszAdditionalInfo = m_PckAllInfo.PckIndexAddr.szAdditionalInfo;
	for(int i=0;i<255;i++)
	{
		if(0 == *lpszAdditionalInfo)*lpszAdditionalInfo = 32;
		lpszAdditionalInfo++;
	}
	*lpszAdditionalInfo-- = 0;
	for(int i=0;i<255;i++)
	{
		if(32 != *lpszAdditionalInfo)break;
			*lpszAdditionalInfo = 0;
		lpszAdditionalInfo--;
	}

	if(!lpcReadfile->Read(&(m_PckAllInfo.PckTail), sizeof(PCKTAIL)))
	{
		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcReadfile;
		return FALSE;
	}

	if(NULL == (m_lpThisPckKey = lpPckParams->lpPckVersion->findKeyById(&m_PckAllInfo))){

		m_lpThisPckKey = lpPckParams->lpPckVersion->getInitialKey();

		//��ӡ��ϸԭ��
		char szPrintf[1024];

	#if defined PCKV202 || defined PCKV203ZX
		StringCchPrintfA(szPrintf, 1024, "������Ϣ��"
						"HEAD->dwHeadCheckHead = 0x%08x, "
						"HEAD->dwPckSize = 0x%08x, "
						"HEAD->dwHeadCheckTail = 0x%08x, "
						"TAIL->dwFileCount = 0x%08x, "
						"TAIL->dwVersion = 0x%08x, "
						"INDEX->dwIndexTableCheckHead = 0x%08x, "
						"INDEX->dwCryptDataAddr = 0x%08x, "
						"INDEX->dwIndexTableCheckTail = 0x%08x",\
						m_PckAllInfo.PckHead.dwHeadCheckHead, \
						m_PckAllInfo.PckHead.dwPckSize, \
						m_PckAllInfo.PckHead.dwHeadCheckTail, \
						m_PckAllInfo.PckTail.dwFileCount, \
						m_PckAllInfo.PckTail.dwVersion, \
						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckHead, \
						m_PckAllInfo.PckIndexAddr.dwCryptDataAddr, \
						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckTail);



	#elif defined PCKV203
		StringCchPrintfA(szPrintf, 1024, "������Ϣ��"
						"HEAD->dwHeadCheckHead = 0x%08x, "
						"HEAD->dwPckSize = 0x%016llx, "
						"TAIL->dwFileCount = 0x%08x, "
						"TAIL->dwVersion = 0x%08x, "
						"INDEX->dwIndexTableCheckHead = 0x%08x, "
						"INDEX->dwCryptDataAddr = 0x%016llx, "
						"INDEX->dwIndexTableCheckTail = 0x%08x", \
						m_PckAllInfo.PckHead.dwHeadCheckHead, \
						m_PckAllInfo.PckHead.dwPckSize, \
						m_PckAllInfo.PckTail.dwFileCount, \
						m_PckAllInfo.PckTail.dwVersion, \
						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckHead, \
						m_PckAllInfo.PckIndexAddr.dwCryptDataAddr, \
						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckTail);


	#endif

		PrintLogE(TEXT_UNKNOWN_PCK_FILE);
		PrintLogD(szPrintf);
		
		delete lpcReadfile;
		return FALSE;
	}

	if(!AllocIndexTableAndInit(m_lpPckIndexTable, m_PckAllInfo.PckTail.dwFileCount)){
		delete lpcReadfile;
		return FALSE;
	}

	//�ļ���������ʼ��ַ	FILEINDEX_ADDR_CONST		0xA8937462
	m_PckAllInfo.dwAddressName = m_PckAllInfo.PckIndexAddr.dwCryptDataAddr ^ m_lpThisPckKey->IndexesEntryAddressCryptKey;

	LPPCKINDEXTABLE lpPckIndexTable = m_lpPckIndexTable;
	BOOL			isLevel0;
	DWORD			byteLevelKey;

	//pck��ѹ��ʱ���ļ�����ѹ�����Ȳ��ᳬ��0x100�����Ե�
	//��ʼһ���ֽڣ����0x75����û��ѹ���������0x74����ѹ����	0x75->FILEINDEX_LEVEL0
	//
	lpcReadfile->SetFilePointer(m_PckAllInfo.dwAddressName, FILE_BEGIN);

	if(!lpcReadfile->Read(&byteLevelKey, 4))
	{
		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcReadfile;
		return FALSE;
	}

	byteLevelKey ^= m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;

	isLevel0 = (INDEXTABLE_CLEARTEXT_LENGTH == byteLevelKey)/* ? TRUE : FALSE*/;

	//��ʼ���ļ�
	BYTE	*lpFileBuffer;

	if(NULL == (lpFileBuffer = lpcReadfile->View(m_PckAllInfo.dwAddressName, lpcReadfile->GetFileSize() - m_PckAllInfo.dwAddressName)))
	{
		PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcReadfile;
		return FALSE;
	}

	if(isLevel0)
	{
		for(DWORD i = 0;i<m_PckAllInfo.PckTail.dwFileCount;i++)
		{

			memcpy(lpPckIndexTable, lpFileBuffer, dwFileBytesRead = (8 + INDEXTABLE_CLEARTEXT_LENGTH));

			lpPckIndexTable->dwIndexDataLength = lpPckIndexTable->dwIndexValueHead ^ m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;

			if( (lpPckIndexTable->dwIndexValueTail ^ m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2) != lpPckIndexTable->dwIndexDataLength)
			{

				PrintLogE(TEXT_READ_INDEX_FAIL, __FILE__, __FUNCTION__, __LINE__);

				delete lpcReadfile;
				return FALSE;
			}

			lpFileBuffer += dwFileBytesRead;
			++lpPckIndexTable;

		}
	}else{

		dwFileBytesRead = INDEXTABLE_CLEARTEXT_LENGTH;

		for(DWORD i = 0;i<m_PckAllInfo.PckTail.dwFileCount;++i)
		{

			memcpy(lpPckIndexTable, lpFileBuffer, 8);
			lpFileBuffer += 8;

			lpPckIndexTable->dwIndexDataLength = lpPckIndexTable->dwIndexValueHead ^ m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;

			if( (lpPckIndexTable->dwIndexValueTail ^ m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2) != lpPckIndexTable->dwIndexDataLength)
			{

				PrintLogE(TEXT_READ_INDEX_FAIL, __FILE__, __FUNCTION__, __LINE__);

				delete lpcReadfile;
				return FALSE;
			}

			uncompress((BYTE*)&lpPckIndexTable->cFileIndex, &dwFileBytesRead,
						lpFileBuffer, lpPckIndexTable->dwIndexDataLength);

			lpFileBuffer += lpPckIndexTable->dwIndexDataLength;
			++lpPckIndexTable;

		}
	}

	delete lpcReadfile;

	return TRUE;
}

void CPckClass::BuildDirTree()
{

	LPPCKINDEXTABLE lpPckIndexTable = m_lpPckIndexTable;

	for(DWORD i = 0;i<m_PckAllInfo.PckTail.dwFileCount;++i)
	{
		//����Ŀ¼
		AddFileToNode(&m_RootNode, lpPckIndexTable);
		++lpPckIndexTable;
	}

}
