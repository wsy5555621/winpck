//////////////////////////////////////////////////////////////////////
// ZupClass.cpp: ���ڽ����������繫˾��zup�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.5.23
//////////////////////////////////////////////////////////////////////
//#include "zlib.h"
#include "ZupClass.h"


CZupClass::CZupClass(LPPCK_RUNTIME_PARAMS inout) : CPckClass(inout)
{
	lpPckParams = inout;

	m_lpZupIndexTable = NULL;
	//m_firstFile = NULL;
	memset(&m_RootNodeZup, 0, sizeof(PCK_PATH_NODE));

}

CZupClass::~CZupClass()
{
	DeAllocMultiNodes(m_RootNodeZup.child);
	if(NULL != m_lpZupIndexTable)free(m_lpZupIndexTable);	
}

CONST	LPPCKINDEXTABLE CZupClass::GetPckIndexTable()
{
	return m_lpZupIndexTable;
}

CONST	LPPCK_PATH_NODE CZupClass::GetPckPathNode()
{
	return &m_RootNodeZup;
}


void CZupClass::BuildDirTree()
{

	size_t	sizeFileIndex = sizeof(PCKINDEXTABLE);

	CMapViewFileRead	*lpcReadfile = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpcReadfile, m_PckAllInfo.szFilename, m_szMapNameRead)){
		delete lpcReadfile;
		return;
	}

	//if(!lpcReadfile->Open(m_Filename))
	//{
	//	//swprintf(m_lastErrorString, 1024,
	//	//					TEXT_OPENNAME_FAIL, m_Filename);
	//	PrintLogE(TEXT_OPENNAME_FAIL, m_Filename, __FILE__, __FUNCTION__, __LINE__);
	//	delete lpcReadfile;
	//	return;
	//}

	//if(!lpcReadfile->Mapping(m_szMapNameRead))
	//{
	//	//swprintf(m_lastErrorString, 1024,
	//	//					TEXT_CREATEMAPNAME_FAIL, m_Filename);
	//	PrintLogE(TEXT_CREATEMAPNAME_FAIL, m_Filename, __FILE__, __FUNCTION__, __LINE__);

	//	delete lpcReadfile;
	//	return;

	//}

	LPPCKINDEXTABLE lpPckIndexTable = m_lpPckIndexTable;
	LPPCKINDEXTABLE lpZupIndexTable = m_lpZupIndexTable;

	for(DWORD i = 0;i<m_PckAllInfo.dwFileCount;i++)
	{

		//��element\��ͷ�Ķ���Ҫ����
		//����ֱ�Ӹ���
		//"element\" = 0x6d656c65, 0x5c746e656d656c65
		if(0x6d656c65 == *(DWORD*)lpPckIndexTable->cFileIndex.szFilename){

			//�����ļ���
			memcpy(lpZupIndexTable, lpPckIndexTable, sizeFileIndex);
			DecodeFilename(lpZupIndexTable->cFileIndex.szFilename, lpPckIndexTable->cFileIndex.szFilename);

			BYTE	*lpbuffer = lpcReadfile->ReView(lpZupIndexTable->cFileIndex.dwAddressOffset, lpZupIndexTable->cFileIndex.dwFileCipherTextSize);
			if(NULL == lpbuffer)
			{

				PrintLogE(TEXT_VIEWMAPNAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);

				delete lpcReadfile;
				return ;
			}

			if(lpZupIndexTable->cFileIndex.dwFileCipherTextSize == lpZupIndexTable->cFileIndex.dwFileClearTextSize){

				lpZupIndexTable->cFileIndex.dwFileClearTextSize = *(DWORD*)lpbuffer;

				lpZupIndexTable->cFileIndex.dwAddressOffset += 4;
				lpZupIndexTable->cFileIndex.dwFileCipherTextSize -= 4;

			}else{

				DWORD	dwFileBytesRead = 4;
				//if(Z_OK != 
					decompress((BYTE*)&lpZupIndexTable->cFileIndex.dwFileClearTextSize, &dwFileBytesRead,
						lpbuffer, lpZupIndexTable->cFileIndex.dwFileCipherTextSize);

				lpZupIndexTable->cFileIndex.dwFileCipherTextSize = lpPckIndexTable->cFileIndex.dwFileClearTextSize;

			}


		}else{
			//ֱ�Ӹ���
			memcpy(lpZupIndexTable, lpPckIndexTable, sizeFileIndex);

		}

		//����Ŀ¼
		AddFileToNode(&m_RootNodeZup, lpZupIndexTable);
		AddFileToNode(&m_RootNode, lpPckIndexTable);

		lpPckIndexTable++;
		lpZupIndexTable++;
	}
	
	delete lpcReadfile;

}

BOOL CZupClass::Init(LPCTSTR szFile)
{

	lstrcpy(m_PckAllInfo.szFilename, szFile);

	if(m_ReadCompleted = MountPckFile(m_PckAllInfo.szFilename))
	{

		if(!AllocIndexTableAndInit(m_lpZupIndexTable, m_PckAllInfo.dwFileCount)){
			return FALSE;
		}

		//��һ��ѭ������ȡinc�ļ��������ֵ�
		m_lpDictHash = new CDictHash();
		if(BuildZupBaseDict()){

			//�ڶ���ѭ���������ļ���������Ŀ¼��
			BuildDirTree();
		}

		//ɾ���ֵ�
		delete m_lpDictHash;

		return TRUE;

	}else{
		return FALSE;
	}

}

