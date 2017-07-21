VOID CPckClass::COMPRESSTHREADFUNC(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	char	szFileMappingNameSpaceFormat[16];
	char	szFileMappingNameSpace[32];
	int		level = pThis->lpPckParams->dwCompressLevel;

	memcpy(szFileMappingNameSpaceFormat, pThis->m_szMapNameRead, 16);
	strcat_s(szFileMappingNameSpaceFormat, 16, "%d");

	EnterCriticalSection(&g_mt_threadID);
	sprintf_s(szFileMappingNameSpace, 32, szFileMappingNameSpaceFormat, mt_threadID++);
	LeaveCriticalSection(&g_mt_threadID);

	BYTE	*bufCompressData = (BYTE*)1;

	LPBYTE				lpBufferToRead;
	PCKFILEINDEX		pckFileIndex;

	//��ʼ
	LPFILES_TO_COMPRESS			lpfirstFile = pThis->m_firstFile;
	PCKINDEXTABLE_COMPRESS		cPckIndexTableComp;

	memset(pckFileIndex.szFilename, 0, MAX_PATH_PCK_260);

	//patch 140424
	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

	while (NULL != lpfirstFile->next)
	{

#pragma region CriticalSectionCompressedflag
#ifdef _DEBUG
		assert(0 == lpfirstFile->dwCompressedflag || 1 == lpfirstFile->dwCompressedflag);
#endif

		EnterCriticalSection(&g_dwCompressedflag);

		if (0 != lpfirstFile->dwCompressedflag) {
			LeaveCriticalSection(&g_dwCompressedflag);
			lpfirstFile = lpfirstFile->next;
			continue;
		}

		lpfirstFile->dwCompressedflag = 1;

		LeaveCriticalSection(&g_dwCompressedflag);
#pragma endregion

		DWORD dwMallocSize = pThis->GetCompressBoundSizeByFileSize(&pckFileIndex, lpfirstFile->dwFileSize);

		//�ж�ʹ�õ��ڴ��Ƿ񳬹����ֵ
		pThis->detectMaxAndAddMemory(dwMallocSize);

#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_CREATE
			memcpy(pckFileIndex.szFilename, lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy);
#elif TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
			memcpy(mystrcpy(pckFileIndex.szFilename, mt_szCurrentNodeString), lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy - mt_nCurrentNodeStringLen);
#endif
		if (!*(mt_lpbThreadRunning))
			break;

		//����ļ���СΪ0�����������ļ�����
		if (0 != pckFileIndex.dwFileClearTextSize)
		{
			//�ļ���Ϊ0ʱ�Ĵ���
			//��Ҫ����ѹ�����ļ�
			if (NULL == (lpBufferToRead = pThis->OpenMappingAndViewAllRead(lpFileRead, lpfirstFile->szFilename, szFileMappingNameSpace)))
			{
				*(mt_lpbThreadRunning) = FALSE;
				lpFileRead->clear();
				break;
			}

			if (NULL == (bufCompressData = (BYTE*)malloc(dwMallocSize))) {
				*(mt_lpbThreadRunning) = FALSE;
				lpFileRead->clear();
				break;
			}

			if (PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileClearTextSize)
			{
				pThis->compress(bufCompressData, &pckFileIndex.dwFileCipherTextSize,
					lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
			}
			else {
				memcpy(bufCompressData, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
			}

			lpFileRead->clear();

		}
		else {
			bufCompressData = (BYTE*)1;
		}

		memset(&cPckIndexTableComp, 0, sizeof(PCKINDEXTABLE_COMPRESS));

		EnterCriticalSection(&g_cs);

		//����������ʾ���ļ�����
		(*mt_lpdwCount)++;

#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
			//���ظ����ļ�
			if (NULL != lpfirstFile->samePtr)
			{
				//�ظ����ļ�����д����ļ���СС�ڱ����ǵ��ļ���С��ʹ����������
				if (pckFileIndex.dwFileCipherTextSize <= lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize)
				{
					cPckIndexTableComp.dwAddressOfDuplicateOldDataArea = lpfirstFile->samePtr->cFileIndex.dwAddressOffset;
				}

				lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize;
				lpfirstFile->samePtr->cFileIndex.dwFileClearTextSize = pckFileIndex.dwFileClearTextSize;

				cPckIndexTableComp.bInvalid = TRUE;

			}
#endif

		LeaveCriticalSection(&g_cs);

		cPckIndexTableComp.dwMallocSize = dwMallocSize;

#ifdef _DEBUG
		if (1 == (int)bufCompressData) {
			logOutput(__FUNCTION__, formatString("file��bufCompressData(0x%08x):0, \t%s\r\n", (int)bufCompressData, lpfirstFile->szFilename));
		}
		else {
			logOutput(__FUNCTION__, formatString("file��bufCompressData(0x%08x):0x%016llx, \t%s\r\n", (int)bufCompressData, _byteswap_uint64(*(unsigned __int64*)bufCompressData), lpfirstFile->szFilename));
		}
#endif

#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_CREATE

		pThis->putCompressedDataQueue(bufCompressData, &cPckIndexTableComp, &pckFileIndex);

#elif TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
		if (!cPckIndexTableComp.bInvalid)
		{
			//�������
			pThis->putCompressedDataQueue(bufCompressData, &cPckIndexTableComp, &pckFileIndex);
		}
		else {

			//�������
			pThis->putCompressedDataQueue(bufCompressData, &cPckIndexTableComp, &lpfirstFile->samePtr->cFileIndex);
		}
#endif


		//��һ���ļ��б�
		lpfirstFile = lpfirstFile->next;
	}

	delete lpFileRead;

	EnterCriticalSection(&g_mt_threadID);

	mt_threadID--;
	if (0 == mt_threadID) {
		pThis->PrintLogN(TEXT_LOG_FLUSH_CACHE);
		SetEvent(mt_evtAllCompressFinish);
	}

	LeaveCriticalSection(&g_mt_threadID);


	_endthread();
}

#if PCK_COMPRESS_NEED_ST

BOOL CPckClass::COMPRESSSINGLETHREADFUNC(QWORD &dwAddress)
{
#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
	BOOL		bUseCurrent;		//����ʱ�Ƿ�ʹ�õ�ǰ���ݣ����ģʽ����Ч
	QWORD		dwAddressTemp;			//���ģʽ�£�����ʱ����ָʾ�������ļ�Զ��������ļ�ʱ��ѹ�����ݵĴ�ŵ�ַ
	//DWORD		dwOverWriteModeMaxLength;	->dwMallocSize //����ʱ���������Ǳ������ļ�ʱʹ�õ����ViewMap��С����Ȼ����ĵ����ݻ�Ѻ����ļ���������0
#endif
	int		level = lpPckParams->dwCompressLevel;

	LPBYTE		lpBufferToWrite, lpBufferToRead;

	PCKFILEINDEX	pckFileIndex;

	//��ʼ��ָ��
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;
	LPFILES_TO_COMPRESS			lpfirstFile = m_firstFile;

	//��ʼ������ֵ 
	memset(pckFileIndex.szFilename, 0, MAX_PATH_PCK_260);

	//patch 140424
	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

	while (NULL != lpfirstFile->next) {
#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_CREATE
		memcpy(pckFileIndex.szFilename, lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy);
#elif TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
		memcpy(mystrcpy(pckFileIndex.szFilename, mt_szCurrentNodeString), lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy - mt_nCurrentNodeStringLen);
#endif
		DWORD dwMallocSize = GetCompressBoundSizeByFileSize(&pckFileIndex, lpfirstFile->dwFileSize);

		if (!lpPckParams->cVarParams.bThreadRunning) {
			PrintLogW(TEXT_USERCANCLE);

			//Ŀǰ��ѹ���˶����ļ���������д��dwFileCount��д�ļ����б���ļ�ͷ��β������ļ�����
			SET_PCK_FILE_COUNT_GLOBAL
				break;
		}

		//����ļ���СΪ0�����������ļ�����
		if (0 != dwMallocSize) {

			//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
			//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
			//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
			if (!IsNeedExpandWritingFile(mt_lpFileWrite, dwAddress, dwMallocSize, mt_CompressTotalFileSize)) {
				SET_PCK_FILE_COUNT_AT_FAIL
					break;
			}
#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
			//���ģʽ
			if (NULL != lpfirstFile->samePtr) {
				//������е��ļ���С����pck�е��ļ�����ѹ����С��ʹ����buffer
				if (dwMallocSize >= lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize) {
					lpfirstFile->samePtr->cFileIndex.dwAddressOffset = dwAddress;
					bUseCurrent = TRUE;

					dwAddressTemp = dwAddress;

				}
				else {

					dwAddressTemp = lpfirstFile->samePtr->cFileIndex.dwAddressOffset;
					bUseCurrent = FALSE;
					dwMallocSize = lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize;


					lpPckIndexTablePtr->dwAddressOfDuplicateOldDataArea = lpfirstFile->samePtr->cFileIndex.dwAddressOffset;

				}

				lpPckIndexTablePtr->bInvalid = TRUE;

			}
			else {
				dwAddressTemp = dwAddress;
			}

			if (NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddressTemp, dwMallocSize))) {
#elif TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_CREATE
			if (NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddress, dwMallocSize))) {
#endif
				mt_lpFileWrite->UnMaping();
				SET_PCK_FILE_COUNT_AT_FAIL
					break;
			}

#pragma region ��Ҫ����ѹ�����ļ�
			if (NULL == (lpBufferToRead = OpenMappingAndViewAllRead(lpFileRead, lpfirstFile->szFilename, m_szMapNameRead)))
			{
				mt_lpFileWrite->UnMaping();
				lpFileRead->clear();

				SET_PCK_FILE_COUNT_AT_FAIL
					break;
			}
#pragma endregion

			if (PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileClearTextSize) {

				compress(lpBufferToWrite, &pckFileIndex.dwFileCipherTextSize,
					lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
			}
			else {
				memcpy(lpBufferToWrite, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
			}

			mt_lpFileWrite->UnmapView();
			lpFileRead->clear();

			}//�ļ���Ϊ0ʱ�Ĵ���
#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
		else {
			//if (NULL != lpfirstFile->samePtr) {
				bUseCurrent = FALSE;
			//}
		}

		//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
		if (NULL == lpfirstFile->samePtr) {
#endif
			pckFileIndex.dwAddressOffset = dwAddress;		//���ļ���ѹ��������ʼ��ַ
			dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ

			FillAndCompressIndexData(lpPckIndexTablePtr, &pckFileIndex);

			//����������ʾ���ļ�����
			lpPckParams->cVarParams.dwUIProgress++;
#if TARGET_PCK_MODE_COMPRESS == PCK_MODE_COMPRESS_ADD
		}
		else {

			lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize;
			lpfirstFile->samePtr->cFileIndex.dwFileClearTextSize = pckFileIndex.dwFileClearTextSize;

			if (bUseCurrent) {

				dwAddress += pckFileIndex.dwFileCipherTextSize;

			}
		}
#endif
		//���ܵ��ļ�����������
		lpPckIndexTablePtr++;
		//��һ���ļ��б�
		lpfirstFile = lpfirstFile->next;
		}

	delete lpFileRead;
	return TRUE;
	}
#endif

#undef COMPRESSTHREADFUNC
#undef COMPRESSSINGLETHREADFUNC
#undef TARGET_PCK_MODE_COMPRESS