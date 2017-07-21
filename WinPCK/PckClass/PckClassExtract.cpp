//////////////////////////////////////////////////////////////////////
// PckClassExtract.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ��ѹ����
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#include "zlib.h"
#include "PckClass.h"


BOOL CPckClass::GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpPckFileIndexTable, char *buffer, size_t sizeOfBuffer)
{

	CMapViewFileRead	*lpFileRead = (CMapViewFileRead*)lpvoidFileRead;

	LPPCKFILEINDEX lpPckFileIndex = &lpPckFileIndexTable->cFileIndex;

	if(NULL == lpvoidFileRead)
	{
		lpFileRead = new CMapViewFileRead();

		if(!OpenPckAndMappingRead(lpFileRead, m_PckAllInfo.szFilename)){
			delete lpFileRead;
			return FALSE;
		}

	}

	BYTE	*lpMapAddress;
	if(NULL == (lpMapAddress = lpFileRead->View(lpPckFileIndex->dwAddressOffset, lpPckFileIndex->dwFileCipherTextSize)))
	{

		PrintLogE(TEXT_VIEWMAPNAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);

		if(NULL == lpvoidFileRead)
			delete lpFileRead;
		return FALSE;
	}

	DWORD	dwFileLengthToWrite = lpPckFileIndex->dwFileClearTextSize;

	if(0 != sizeOfBuffer && sizeOfBuffer < dwFileLengthToWrite)
		dwFileLengthToWrite = sizeOfBuffer;


	if(PCK_BEGINCOMPRESS_SIZE < lpPckFileIndex->dwFileClearTextSize)
	{
		int rtn = uncompress((BYTE*)buffer, &dwFileLengthToWrite,
					lpMapAddress, lpPckFileIndex->dwFileCipherTextSize);

		//if(Z_OK != uncompress((BYTE*)buffer, &dwFileLengthToWrite,
		//			lpMapAddress, lpPckFileIndex->dwFileCipherTextSize))
		if(Z_OK != rtn && !((Z_BUF_ERROR == rtn) && (dwFileLengthToWrite < lpPckFileIndex->dwFileClearTextSize)) )
		{
			if(lpPckFileIndex->dwFileClearTextSize == lpPckFileIndex->dwFileCipherTextSize)
				memcpy(buffer, lpMapAddress, dwFileLengthToWrite);
			else
			{

				PrintLogE(TEXT_UNCOMPRESSDATA_FAIL, lpPckFileIndex->szFilename, __FILE__, __FUNCTION__, __LINE__);

				lpFileRead->UnmapView();

				if(NULL == lpvoidFileRead)
					delete lpFileRead;

				return FALSE;
			}
		}
	}else{
		memcpy(buffer, lpMapAddress, dwFileLengthToWrite);
	}

	lpFileRead->UnmapView();

	if(NULL == lpvoidFileRead)
		delete lpFileRead;



	return TRUE;

}

BOOL CPckClass::ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount)
{

	PrintLogI(TEXT_LOG_EXTRACT);

	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpFileRead, m_PckAllInfo.szFilename)){
		delete lpFileRead;
		return FALSE;
	}
	
	BOOL	ret = TRUE;

	char	szFilename[MAX_PATH_PCK], *szStrchr;

	LPPCKINDEXTABLE *lpIndexToExtractPtr = lpIndexToExtract;

	for(int i=0;i<nFileCount;i++)
	{
		if(!lpPckParams->cVarParams.bThreadRunning)
		{
			PrintLogW(TEXT_USERCANCLE);

			delete lpFileRead;
			return FALSE;
		}

		memcpy(szFilename, (*lpIndexToExtractPtr)->cFileIndex.szFilename, MAX_PATH_PCK);
		szStrchr = szFilename;
		for(int j=0;j<MAX_PATH_PCK;j++)
		{
			if('\\' == *szStrchr)*szStrchr = '_';
			else if('/' == *szStrchr)*szStrchr = '_';
			//else if(0 == *szStrchr)break;
			++szStrchr;
		}

		//��ѹ�ļ�
		if(!(DecompressFile(szFilename, *lpIndexToExtractPtr, lpFileRead)))
		{
			PrintLogE(TEXT_UNCOMP_FAIL, __FILE__, __FUNCTION__, __LINE__);

			delete lpFileRead;
			return FALSE;
		}else{
			//dwCount++;
			++(lpPckParams->cVarParams.dwUIProgress);
		}

		++lpIndexToExtractPtr;
	}

	delete lpFileRead;

	PrintLogI(TEXT_LOG_WORKING_DONE);

	return	ret;
}

BOOL CPckClass::ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount)
{

	PrintLogI(TEXT_LOG_EXTRACT);

	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpFileRead, m_PckAllInfo.szFilename)){
		delete lpFileRead;
		return FALSE;
	}

	BOOL	ret = TRUE;

	LPPCK_PATH_NODE *lpNodeToExtractPtr = lpNodeToExtract;

	for(int i=0;i<nFileCount;i++)
	{
		if(!lpPckParams->cVarParams.bThreadRunning)
		{
			PrintLogW(TEXT_USERCANCLE);

			delete lpFileRead;
			return FALSE;
		}

		if(NULL == (*lpNodeToExtractPtr)->child)
		{

			//��ѹ�ļ�
			if(!(DecompressFile((*lpNodeToExtractPtr)->szName, (*lpNodeToExtractPtr)->lpPckIndexTable, lpFileRead)))
			{
				PrintLogE(TEXT_UNCOMP_FAIL, __FILE__, __FUNCTION__, __LINE__);

				delete lpFileRead;
				return FALSE;
			}else{
				//dwCount++;
				++(lpPckParams->cVarParams.dwUIProgress);
			}

		}else{

			CreateDirectoryA((*lpNodeToExtractPtr)->szName, NULL);
			SetCurrentDirectoryA((*lpNodeToExtractPtr)->szName);
			ret = StartExtract((*lpNodeToExtractPtr)->child->next, lpFileRead);
			SetCurrentDirectoryA("..\\");
		}

		lpNodeToExtractPtr++;
	}

	delete lpFileRead;

	PrintLogI(TEXT_LOG_WORKING_DONE);

	return	ret;
}
BOOL CPckClass::StartExtract(LPPCK_PATH_NODE lpNodeToExtract, /*DWORD	&dwCount, */LPVOID lpvoidFileRead/*, BOOL	&bThreadRunning*/)
{

	CMapViewFileRead	*lpFileRead = (CMapViewFileRead*)lpvoidFileRead;

	do
	{
		if(!lpPckParams->cVarParams.bThreadRunning)
		{
			PrintLogW(TEXT_USERCANCLE);
			return FALSE;
		}

		//lpThisNodePtr = lpNodeToExtract;
		if(NULL != lpNodeToExtract->child)
		{
			
			CreateDirectoryA(lpNodeToExtract->szName, NULL);
			SetCurrentDirectoryA(lpNodeToExtract->szName);
			
			StartExtract(lpNodeToExtract->child->next, lpFileRead);

			SetCurrentDirectoryA("..\\");

		}else{
		
			//��ѹ�ļ�
			if(!DecompressFile(lpNodeToExtract->szName, lpNodeToExtract->lpPckIndexTable, lpFileRead))
			{
				PrintLogE(TEXT_UNCOMP_FAIL, __FILE__, __FUNCTION__, __LINE__);

				return FALSE;
			}else{
				//dwCount++;
				++(lpPckParams->cVarParams.dwUIProgress);
			}
		}

		lpNodeToExtract = lpNodeToExtract->next;

	}while(NULL != lpNodeToExtract);

	return TRUE;
}

BOOL CPckClass::DecompressFile(char	*lpszFilename, LPPCKINDEXTABLE lpPckFileIndexTable, LPVOID lpvoidFileRead)
{
	LPPCKFILEINDEX lpPckFileIndex = &lpPckFileIndexTable->cFileIndex;

	//CMapViewFileRead	*lpFileRead = (CMapViewFileRead*)lpvoidFileRead;
	CMapViewFileWrite	*lpFileWrite = new CMapViewFileWrite();

	//LPBYTE	lpMapAddress;
	LPBYTE	lpMapAddressToWrite;
	DWORD	dwFileLengthToWrite;

	//if (0 == strcmp(lpszFilename, "�����м��ܹ���1.gfx")){
	//	dwFileLengthToWrite = 1;
	//}

	dwFileLengthToWrite = lpPckFileIndex->dwFileClearTextSize;

	//�����Ǵ���һ���ļ������������ѹ������ļ�
	if(!lpFileWrite->Open(lpszFilename, CREATE_ALWAYS))
	{
		PrintLogE(TEXT_OPENWRITENAME_FAIL, lpszFilename, __FILE__, __FUNCTION__, __LINE__);

		delete lpFileWrite;
		return FALSE;
	}

	if(!lpFileWrite->Mapping(m_szMapNameWrite, dwFileLengthToWrite))
	{

		delete lpFileWrite;

		if(0 == dwFileLengthToWrite)return TRUE;

		PrintLogE(TEXT_CREATEMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);

		return FALSE;
	}

	if(NULL == (lpMapAddressToWrite = lpFileWrite->View(0, 0)))
	{
		PrintLogE(TEXT_VIEWMAPNAME_FAIL, lpszFilename, __FILE__, __FUNCTION__, __LINE__);

		delete lpFileWrite;
		return FALSE;
	}


	BOOL rtn = GetSingleFileData(lpvoidFileRead, lpPckFileIndexTable, (char*)lpMapAddressToWrite);

	lpFileWrite->SetFilePointer(dwFileLengthToWrite ,FILE_BEGIN);
	lpFileWrite->SetEndOfFile();
	delete lpFileWrite;

	return rtn;
}

