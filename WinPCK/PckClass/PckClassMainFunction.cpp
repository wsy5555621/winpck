//////////////////////////////////////////////////////////////////////
// PckClassFunction.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// һЩ�����Ĺ��� 
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#include "tlib.h"
#endif

#include "PckClass.h"

#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4244 )

//_inline int __fastcall strpathcmp (const char * src, char * &dst)
//{
//	int ret = 0 ;
//
//	while( (! (ret = (int)(*src - *dst))) && *src )
//		++src, ++dst;
//	if(0 != ret && 0 == *src && ('\\' == *dst || '/'== *dst))return 0;
//	return( ret );
//}

//strpathcmp
//���������¶��Ǵ�����ͬ���ļ���
//��ʱret != 0;
//
_inline int __fastcall strpathcmp(const char * src, char * &dst)
{
	int ret = 0;

	while ((!(ret = (int)(*src - *dst))) && *src)
		++src, ++dst;

	if (0 != ret && 0 == *src && ('\\' == *dst || '/' == *dst))return 0;
	return(ret);
}

_inline void __fastcall strpathcpy(char * dst, char * &src)
{
	while ((*dst++ = *src) && '\\' != *++src && '/' != *src)
		;
}

_inline char * __fastcall mystrcpy(char * dest, const char *src)
{
	while ((*dest = *src))
		++dest, ++src;
	return dest;
}

/********************************
*
*��������
*
********************/

BOOL CPckClass::RebuildPckFile(LPTSTR szRebuildPckFile)
{

	PrintLogI(TEXT_LOG_REBUILD);

	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTable, lpPckIndexTablePtr;
	QWORD	dwAddress = PCK_DATA_START_AT, dwAddressName;
	DWORD	dwFileCount = m_PckAllInfo.dwFileCount;
	DWORD	dwNoDupFileCount = ReCountFiles();
	QWORD	dwTotalFileSizeAfterRebuild = m_PckAllInfo.qwPckSize + (dwFileCount << 3);

	PCK_ALL_INFOS		pckAllInfo;

	LPBYTE				lpBufferToWrite, lpBufferToRead;
	CMapViewFileWrite	*lpFileWrite;
	CMapViewFileRead	*lpFileRead;

	//����ͷ��βʱ��Ҫ�Ĳ���
	memset(&pckAllInfo, 0, sizeof(PCK_ALL_INFOS));
	pckAllInfo.lpSaveAsPckVerFunc = m_PckAllInfo.lpSaveAsPckVerFunc;

	//���ý����������ֵ
	lpPckParams->cVarParams.dwUIProgressUpper = dwNoDupFileCount;

	//��Դ�ļ� 
	lpFileRead = new CMapViewFileRead();

	if (!OpenPckAndMappingRead(lpFileRead, m_PckAllInfo.szFilename, m_szMapNameRead)) {
		delete lpFileRead;
		return FALSE;
	}

	//��Ŀ���ļ� 
	//�����Ǵ���һ���ļ������������ؽ�����ļ�
	lpFileWrite = new CMapViewFileWrite(m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->dwMaxSinglePckSize);

	if (!OpenPckAndMappingWrite(lpFileWrite, szRebuildPckFile, CREATE_ALWAYS, dwTotalFileSizeAfterRebuild)) {
		delete lpFileRead;
		delete lpFileWrite;
		return FALSE;
	}

	//����ռ�,�ļ���ѹ������ ����
	if (NULL == (lpPckIndexTable = new PCKINDEXTABLE_COMPRESS[dwNoDupFileCount])) {
		delete lpFileRead;
		delete lpFileWrite;
		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	//��ʹ��Enum���б�����������_PCK_INDEX_TABLE
	lpPckIndexTablePtr = lpPckIndexTable;

	LPPCKINDEXTABLE lpPckIndexTableSource = m_lpPckIndexTable;
	DWORD		IndexCompressedFilenameDataLengthCryptKey1 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey2;
	int			level = lpPckParams->dwCompressLevel;

	for (DWORD i = 0; i < dwFileCount; ++i) {

		if (lpPckIndexTableSource->isInvalid) {
			++lpPckIndexTableSource;
			continue;
		}

		DWORD dwNumberOfBytesToMap = lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize;

		if (NULL == (lpBufferToWrite = lpFileWrite->View(dwAddress, dwNumberOfBytesToMap))) {
			PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);
			delete lpFileRead;
			delete lpFileWrite;
			delete[] lpPckIndexTable;
			return FALSE;
		}

		DWORD dwSrcAddress = lpPckIndexTableSource->cFileIndex.dwAddressOffset;	//����ԭ���ĵ�ַ

		if (NULL == (lpBufferToRead = lpFileRead->View(dwSrcAddress, dwNumberOfBytesToMap))) {
			PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);
			delete lpFileRead;
			delete lpFileWrite;
			delete[] lpPckIndexTable;
			return FALSE;
		}

		memcpy(lpBufferToWrite, lpBufferToRead, dwNumberOfBytesToMap);

		lpFileRead->UnmapView();

		//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
		lpPckIndexTableSource->cFileIndex.dwAddressOffset = dwAddress;	//���ļ���ѹ��������ʼ��ַ

		dwAddress += lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ

		FillAndCompressIndexData(lpPckIndexTablePtr, &lpPckIndexTableSource->cFileIndex);

		lpPckIndexTableSource->cFileIndex.dwAddressOffset = dwSrcAddress;	//��ԭԭ���ĵ�ַ

		lpFileWrite->UnmapView();

		++lpPckIndexTablePtr;
		++lpPckIndexTableSource;
		++(lpPckParams->cVarParams.dwUIProgress);

	}

	if (!lpPckParams->cVarParams.bThreadRunning) {
		PrintLogW(TEXT_USERCANCLE);
		dwFileCount = lpPckParams->cVarParams.dwUIProgress;
	}

	//�رն��ļ�
	delete lpFileRead;

	//д�ļ�����
	dwAddressName = dwAddress;
	WritePckIndexTable(lpFileWrite, lpPckIndexTable, dwNoDupFileCount, dwAddress);

	pckAllInfo.dwAddressName = dwAddressName;
	pckAllInfo.dwFileCount = dwNoDupFileCount;
	AfterProcess(lpFileWrite, pckAllInfo, dwAddress);

	delete lpFileWrite;
	delete[] lpPckIndexTable;

	PrintLogI(TEXT_LOG_WORKING_DONE);

	return TRUE;

}

VOID CPckClass::RenameIndex(LPPCK_PATH_NODE lpNode, char* lpszReplaceString)
{
	size_t nBytesReadayToWrite;
	char	*lpszPosToWrite;

	nBytesReadayToWrite = strlen(lpNode->lpPckIndexTable->cFileIndex.szFilename) - strlen(lpNode->szName);
	lpszPosToWrite = lpNode->lpPckIndexTable->cFileIndex.szFilename + nBytesReadayToWrite;
	//nBytesReadayToWrite = MAX_PATH_PCK - nBytesReadayToWrite;

	memset(lpszPosToWrite, 0, MAX_PATH_PCK_260 - nBytesReadayToWrite);
	strcpy(lpszPosToWrite, lpszReplaceString);

}

VOID CPckClass::RenameIndex(LPPCKINDEXTABLE lpIndex, char* lpszReplaceString)
{
	//size_t	nBytesReadayToWrite;

	memset(lpIndex->cFileIndex.szFilename, 0, MAX_PATH_PCK_260);
	strcpy(lpIndex->cFileIndex.szFilename, lpszReplaceString);

}

BOOL CPckClass::GetCurrentNodeString(char *szCurrentNodePathString, LPPCK_PATH_NODE lpNode)
{

	if (NULL == lpNode->parentfirst)
	{
		*szCurrentNodePathString = 0;
		//return TRUE;
	}
	else {

		GetCurrentNodeString(szCurrentNodePathString, lpNode->parentfirst);
		strcat_s(szCurrentNodePathString, MAX_PATH_PCK_260, lpNode->parent->szName);
		strcat_s(szCurrentNodePathString, MAX_PATH_PCK_260, "\\");
	}

	return TRUE;
}

BOOL CPckClass::RenameNode(LPPCK_PATH_NODE lpNode, char* lpszReplaceString)
{
	size_t lenNodeRes, lenrs, lenrp;
	char	lpReplacePos[MAX_PATH_PCK_260];

	lenNodeRes = strlen(lpNode->szName);
	lenrs = strlen(lpszReplaceString);

	GetCurrentNodeString(lpReplacePos, lpNode->child);

	//lpReplacePos = "models\z1\"
	lenrp = strlen(lpReplacePos) - 1;

	return RenameNodeEnum(lpNode, lenNodeRes, lpszReplaceString, lenrs, lenrp);

}

/********************************
*
*˽�к���
*
********************/
BOOL CPckClass::AddFileToNode(LPPCK_PATH_NODE lpRootNode, LPPCKINDEXTABLE	lpPckIndexTable)
{
	LPPCK_PATH_NODE lpChildNode = lpRootNode;
	LPPCK_PATH_NODE	lpFirstNode = NULL;

	size_t			sizePckPathNode = sizeof(PCK_PATH_NODE);

	char			*lpszFilename = lpPckIndexTable->cFileIndex.szFilename;
	char			*lpszToFind;

	do
	{
		//�˽ڵ��»�û���ļ����������".."
		if (NULL == (lpChildNode->child))
		{
			lpChildNode->child = (LPPCK_PATH_NODE)AllocNodes(sizePckPathNode);
			lpChildNode->child->parent = lpChildNode;
			lpChildNode->child->parentfirst = lpFirstNode;

			//���..Ŀ¼
			//strcpy(lpChildNode->child->szName, "..");
			//*(DWORD*)lpChildNode->child->szName = 0x2e2e;	//".."
			memcpy(lpChildNode->child->szName, "..", 2);
		}

		lpFirstNode = lpChildNode = lpChildNode->child;

		do
		{
			lpszToFind = lpszFilename;

			if ((NULL == lpChildNode->lpPckIndexTable) || (!lpChildNode->lpPckIndexTable->isInvalid)) {

				if (0 == strpathcmp(lpChildNode->szName, lpszToFind))
				{
					//��������ļ����У���
					lpszFilename = lpszToFind;

					//�����ظ����ļ�������ǰһ���ظ����ļ�Ϊ��Ч
					if (0 == *lpszToFind)
						lpChildNode->lpPckIndexTable->isInvalid = TRUE;

					break;
				}

				if (NULL == lpChildNode->next)
				{

					//����ļ����У�
					lpChildNode->next = (LPPCK_PATH_NODE)AllocNodes(sizePckPathNode);
					lpChildNode = lpChildNode->next;

					strpathcpy(lpChildNode->szName, lpszFilename);

					//ͳ�Ƹ��ļ��е����ļ�����
					if (0 != *lpszFilename)
					{
						LPPCK_PATH_NODE	lpAddDirCount = lpFirstNode;
						do {
							++(lpAddDirCount->dwDirsCount);
							lpAddDirCount = lpAddDirCount->parentfirst;

						} while (NULL != lpAddDirCount);

					}

					break;
				}
			}
			lpChildNode = lpChildNode->next;

		} while (1);

		++(lpFirstNode->dwFilesCount);
		lpFirstNode->qdwDirCipherTextSize += lpPckIndexTable->cFileIndex.dwFileCipherTextSize;
		lpFirstNode->qdwDirClearTextSize += lpPckIndexTable->cFileIndex.dwFileClearTextSize;

		if ('\\' == *lpszFilename || '/' == *lpszFilename)
			++lpszFilename;

	} while (*lpszFilename);

	lpChildNode->lpPckIndexTable = lpPckIndexTable;

	return TRUE;

}

LPPCK_PATH_NODE CPckClass::FindFileNode(LPPCK_PATH_NODE lpBaseNode, char* lpszFile)
{
	LPPCK_PATH_NODE lpChildNode = lpBaseNode;
	//LPPCK_PATH_NODE	lpFirstNode = NULL;

	//size_t			sizePckPathNode = sizeof(PCK_PATH_NODE);

	char			*lpszFilename = lpszFile;
	char			*lpszToFind;

	do
	{
		do
		{
			lpszToFind = lpszFilename;

			if (0 == strpathcmp(lpChildNode->szName, lpszToFind))
			{
				lpszFilename = lpszToFind;

				if (NULL == lpChildNode->child && 0 == *lpszFilename)return lpChildNode;

				if ((NULL == lpChildNode->child && ('\\' == *lpszFilename || '/' == *lpszFilename)) || (NULL != lpChildNode->child && 0 == *lpszFilename))
				{
					return (LPPCK_PATH_NODE)-1;
				}

				break;
			}

			if (NULL == lpChildNode->next)
			{
				return NULL;
			}
			lpChildNode = lpChildNode->next;

		} while (1);

		lpChildNode = lpChildNode->child;

		if ('\\' == *lpszFilename || '/' == *lpszFilename)
			++lpszFilename;

	} while (*lpszFilename);

	return NULL;

}

BOOL CPckClass::RenameNodeEnum(LPPCK_PATH_NODE lpNode, size_t lenNodeRes, char* lpszReplaceString, size_t lenrs, size_t lenrp)
{

	lpNode = lpNode->child->next;

	while (NULL != lpNode) {

		if (NULL == lpNode->child) {

			if (!RenameNode(lpNode, lenNodeRes, lpszReplaceString, lenrs, lenrp))
				return FALSE;
		}
		else {
			if (!RenameNodeEnum(lpNode, lenNodeRes, lpszReplaceString, lenrs, lenrp))
				return FALSE;
		}

		lpNode = lpNode->next;
	}
	return TRUE;
}

BOOL CPckClass::RenameNode(LPPCK_PATH_NODE lpNode, size_t lenNodeRes, char* lpszReplaceString, size_t lenrs, size_t lenrp)
{
	if (lenrs >= (MAX_PATH_PCK_260 - strlen(lpNode->lpPckIndexTable->cFileIndex.szFilename + lenNodeRes - 2)))return FALSE;
	char	szTemp[MAX_PATH_PCK_260] = { 0 };
	char	*lpszReplacePos = lpNode->lpPckIndexTable->cFileIndex.szFilename + lenrp - lenNodeRes;

	//DebugA("lpszReplaceString = %s \r\nlenNodeRes = %d\r\nlenrs = %d\r\nlenrp = %d\r\n===============================\r\n",
	//		lpszReplaceString, lenNodeRes, lenrs, lenrp);
	memcpy(szTemp, lpNode->lpPckIndexTable->cFileIndex.szFilename + lenrp, MAX_PATH_PCK_260 - lenrp);
	memcpy(lpszReplacePos, lpszReplaceString, lenrs);
	memcpy(lpszReplacePos + lenrs, szTemp, MAX_PATH_PCK_260 - lenrp - lenrs + lenNodeRes);

	return TRUE;
}

//////////////////////////�����ǳ����������Ҫ���õĹ���///////////////////////////////
VOID CPckClass::EnumFile(LPSTR szFilename, BOOL IsPatition, DWORD &dwFileCount, LPFILES_TO_COMPRESS &pFileinfo, QWORD &qwTotalFileSize, size_t nLen)
{

	char		szPath[MAX_PATH], szFile[MAX_PATH];

	size_t nLenBytePath = mystrcpy(szPath, szFilename) - szPath + 1;
	strcat(szFilename, "\\*.*");

	HANDLE					hFile;
	WIN32_FIND_DATAA		WFD;

	if ((hFile = FindFirstFileA(szFilename, &WFD)) != INVALID_HANDLE_VALUE) {
		if (!IsPatition) {
			FindNextFileA(hFile, &WFD);
			if (!FindNextFileA(hFile, &WFD)) {
				return;
			}
		}

		do {
			if ((WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

				if ((MAX_PATH_PCK_260 - 13) >= mystrcpy(mystrcpy(mystrcpy(szFile, szPath), "\\"), WFD.cFileName) - szFile) {
					EnumFile(szFile, FALSE, dwFileCount, pFileinfo, qwTotalFileSize, nLen);
				}

			}
			else {

				if (NULL == pFileinfo)return;
				if (0 != WFD.nFileSizeHigh)continue;

				++dwFileCount;

				if (MAX_PATH_PCK_260 < nLenBytePath + strlen(WFD.cFileName)) {
					mystrcpy(mystrcpy(mystrcpy(pFileinfo->szFilename, szPath), "\\"), WFD.cAlternateFileName);
				}
				else {
					mystrcpy(mystrcpy(mystrcpy(pFileinfo->szFilename, szPath), "\\"), WFD.cFileName);
				}

#if 0
				logOutput(__FUNCTION__, formatString("pFileinfo add:(%d)\t%s\r\n", (int)pFileinfo, pFileinfo->szFilename));
#endif

				pFileinfo->lpszFileTitle = pFileinfo->szFilename + nLen;
				pFileinfo->nBytesToCopy = MAX_PATH - nLen;

				qwTotalFileSize += (pFileinfo->dwFileSize = WFD.nFileSizeLow);

				pFileinfo->next = AllocateFileinfo();
				pFileinfo = pFileinfo->next;

			}

		} while (FindNextFileA(hFile, &WFD));

		FindClose(hFile);
	}

}

