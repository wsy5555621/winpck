
VOID CPckClass::CompressThreadRecompress(VOID* pParam)
{
	EnterCriticalSection(&g_mt_threadID);
	++mt_threadID;
	LeaveCriticalSection(&g_mt_threadID);

	CPckClass *pThis = (CPckClass*)pParam;

	int		level = pThis->lpPckParams->dwCompressLevel;

	//���ļ���СΪ0С��ʹ��1���ָ��
	BYTE	*bufCompressData = (BYTE*)1;

	LPBYTE				lpBufferToRead;
	//������ѹ�����ݵĽ�ѹ������
	LPBYTE				lpDecompressBuffer = NULL;
	//������ѹ�����ݵ�Դ����
	LPBYTE				lpSourceBuffer = NULL;
	//
	DWORD				dwMaxMallocSource = 0, dwMaxMallocDecompress = 0;

	//��ʼ
	LPPCKINDEXTABLE					lpPckIndexTablePtrSrc = pThis->m_lpPckIndexTable;		//Դ�ļ����ļ�����Ϣ
	PCKINDEXTABLE_COMPRESS			cPckIndexTableDst;

	DWORD	dwFileCount = pThis->m_PckAllInfo.dwFileCount;

	for (DWORD i = 0; i < dwFileCount; ++i) {

		if (lpPckIndexTablePtrSrc->isInvalid) {
			++lpPckIndexTablePtrSrc;
			continue;
		}

		//�жϴ��ļ��Ƿ��Ѿ�ѹ���������
		{
			EnterCriticalSection(&g_dwCompressedflag);

			if (0 != lpPckIndexTablePtrSrc->isRecompressed) {
				LeaveCriticalSection(&g_dwCompressedflag);
				++lpPckIndexTablePtrSrc;
				continue;
			}

			lpPckIndexTablePtrSrc->isRecompressed = 1;
			LeaveCriticalSection(&g_dwCompressedflag);
		}

		DWORD dwNumberOfBytesToMap = lpPckIndexTablePtrSrc->cFileIndex.dwFileCipherTextSize;
		DWORD dwFileClearTextSize = lpPckIndexTablePtrSrc->cFileIndex.dwFileClearTextSize;

		PCKFILEINDEX cPckFileIndexDst;
		memcpy(&cPckFileIndexDst, &lpPckIndexTablePtrSrc->cFileIndex, sizeof(PCKFILEINDEX));

		if (PCK_BEGINCOMPRESS_SIZE < dwFileClearTextSize) {
			cPckFileIndexDst.dwFileCipherTextSize = pThis->compressBound(dwFileClearTextSize);
		}

		DWORD dwMallocSize = cPckFileIndexDst.dwFileCipherTextSize;

		if (!*(mt_lpbThreadRunning))
			break;

		if (0 != dwFileClearTextSize) {

			//�ж�ʹ�õ��ڴ��Ƿ񳬹����ֵ
			pThis->detectMaxAndAddMemory(dwMallocSize);

			bufCompressData = (BYTE*)malloc(dwMallocSize);

			//�ļ�������Ҫ��ѹ��
			if (PCK_BEGINCOMPRESS_SIZE < dwFileClearTextSize) {
				//����Դ���ݵĿռ�
				if (dwMaxMallocSource < dwNumberOfBytesToMap) {

					dwMaxMallocSource = dwNumberOfBytesToMap;

					if (NULL != lpSourceBuffer)
						free(lpSourceBuffer);

					if (NULL == (lpSourceBuffer = (LPBYTE)malloc(dwNumberOfBytesToMap))) {
						*(mt_lpbThreadRunning) = FALSE;
						if (NULL != lpDecompressBuffer)
							free(lpDecompressBuffer);
						break;
					}
				}

				//�����ѹ���ݵĿռ�
				if (dwMaxMallocDecompress < dwFileClearTextSize) {

					dwMaxMallocDecompress = dwFileClearTextSize;

					if (NULL != lpDecompressBuffer)
						free(lpDecompressBuffer);

					if (NULL == (lpDecompressBuffer = (LPBYTE)malloc(dwFileClearTextSize))) {
						free(lpSourceBuffer);
						*(mt_lpbThreadRunning) = FALSE;
						break;
					}
				}

				EnterCriticalSection(&g_mt_threadID);
				if (NULL == (lpBufferToRead = mt_lpFileRead->View(lpPckIndexTablePtrSrc->cFileIndex.dwAddressOffset, dwNumberOfBytesToMap))) {
					free(lpSourceBuffer);
					free(lpDecompressBuffer);
					*(mt_lpbThreadRunning) = FALSE;
					break;
				}
				memcpy(lpSourceBuffer, lpBufferToRead, dwNumberOfBytesToMap);
				mt_lpFileRead->UnmapView();
				LeaveCriticalSection(&g_mt_threadID);

				pThis->decompress(lpDecompressBuffer, &dwFileClearTextSize, lpSourceBuffer, dwNumberOfBytesToMap);
				if (dwFileClearTextSize == lpPckIndexTablePtrSrc->cFileIndex.dwFileClearTextSize) {

					pThis->compress(bufCompressData, &cPckFileIndexDst.dwFileCipherTextSize, lpDecompressBuffer, dwFileClearTextSize, level);
				}
				else {
					memcpy(bufCompressData, lpSourceBuffer, dwNumberOfBytesToMap);
				}
			}
			else {
#pragma region �ļ���С����Ҫѹ��ʱ
				EnterCriticalSection(&g_mt_threadID);
				if (NULL == (lpBufferToRead = mt_lpFileRead->View(lpPckIndexTablePtrSrc->cFileIndex.dwAddressOffset, dwNumberOfBytesToMap))) {
					*(mt_lpbThreadRunning) = FALSE;
					break;
				}
				memcpy(bufCompressData, lpBufferToRead, dwNumberOfBytesToMap);
				mt_lpFileRead->UnmapView();
				LeaveCriticalSection(&g_mt_threadID);
#pragma endregion
			}

		}
		else {
			bufCompressData = (BYTE*)1;
		}

		EnterCriticalSection(&g_cs);
		(*mt_lpdwCount)++;										//����������ʾ���ļ�����
		LeaveCriticalSection(&g_cs);

		memset(&cPckIndexTableDst, 0, sizeof(PCKINDEXTABLE_COMPRESS));

		cPckIndexTableDst.dwMallocSize = dwMallocSize;

		//�������
		pThis->putCompressedDataQueue(bufCompressData, &cPckIndexTableDst, &cPckFileIndexDst);

		++lpPckIndexTablePtrSrc;
	}

	free(lpSourceBuffer);
	free(lpDecompressBuffer);

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
BOOL CPckClass::RecompressPckFileSingleThread(CMapViewFileRead	*lpFileRead, CMapViewFileWrite *lpFileWrite, DWORD dwFileCount, QWORD &dwAddress, LPPCKINDEXTABLE_COMPRESS	&lpPckIndexTable)
{

	LPBYTE		lpBufferToWrite, lpBufferToRead;

	//����ռ�,�ļ���ѹ������ ����
	//��ʹ��Enum���б�����������_PCK_INDEX_TABLE
	LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr = lpPckIndexTable = mt_lpPckIndexTable;

	LPPCKINDEXTABLE lpPckIndexTableSource = m_lpPckIndexTable;

	int			level = lpPckParams->dwCompressLevel;
	size_t		nMaxMallocSize = 0;
	LPBYTE		lpDecompressBuffer = NULL;

	for (DWORD i = 0; i < dwFileCount; ++i) {

		if (lpPckIndexTableSource->isInvalid) {
			++lpPckIndexTableSource;
			continue;
		}

		DWORD dwNumberOfBytesToMap = lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize;
		DWORD dwCompressBound = this->compressBound(lpPckIndexTableSource->cFileIndex.dwFileClearTextSize);

		if (!lpPckParams->cVarParams.bThreadRunning) {
			PrintLogW(TEXT_USERCANCLE);
			//Ŀǰ��ѹ���˶����ļ���������д��dwFileCount��д�ļ����б���ļ�ͷ��β������ļ�����
			SET_PCK_FILE_COUNT_GLOBAL
				break;
		}

		if (NULL == (lpBufferToWrite = lpFileWrite->View(dwAddress, dwCompressBound))) {
			PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);
			SET_PCK_FILE_COUNT_AT_FAIL
				break;
		}

		DWORD dwSrcAddress = lpPckIndexTableSource->cFileIndex.dwAddressOffset;	//����ԭ���ĵ�ַ

		if (NULL == (lpBufferToRead = lpFileRead->View(dwSrcAddress, dwNumberOfBytesToMap))) {
			PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);
			SET_PCK_FILE_COUNT_AT_FAIL
				break;
		}

		//memcpy(lpBufferToWrite, lpBufferToRead, dwNumberOfBytesToMap);
		//����ռ䲢��ѹ�����ݿ�
		if (nMaxMallocSize < lpPckIndexTableSource->cFileIndex.dwFileClearTextSize) {
			nMaxMallocSize = lpPckIndexTableSource->cFileIndex.dwFileClearTextSize;

			if (NULL != lpDecompressBuffer)
				free(lpDecompressBuffer);

			if (NULL == (lpDecompressBuffer = (LPBYTE)malloc(nMaxMallocSize))) {
				PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
				SET_PCK_FILE_COUNT_AT_FAIL
					break;
			}
		}

		//����ɹ�����ѹ�����ݣ�ʧ�ܾ�ֱ�Ӹ���Դ����
		DWORD dwFileCipherTextSizeSrc = lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize;

		if (PCK_BEGINCOMPRESS_SIZE < lpPckIndexTableSource->cFileIndex.dwFileClearTextSize) {

			DWORD dwFileBytesRead = lpPckIndexTableSource->cFileIndex.dwFileClearTextSize;
			decompress(lpDecompressBuffer, &dwFileBytesRead, lpBufferToRead, lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize);

			if (dwFileBytesRead == lpPckIndexTableSource->cFileIndex.dwFileClearTextSize) {

				compress(lpBufferToWrite, &dwCompressBound,
					lpDecompressBuffer, dwFileBytesRead, level);

				lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize = dwCompressBound;

			}
			else
				memcpy(lpBufferToWrite, lpBufferToRead, lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize);
		}
		else {
			memcpy(lpBufferToWrite, lpBufferToRead, lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize);
		}

		lpFileRead->UnmapView();

		//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
		lpPckIndexTableSource->cFileIndex.dwAddressOffset = dwAddress;	//���ļ���ѹ��������ʼ��ַ
		dwAddress += lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ

		FillAndCompressIndexData(lpPckIndexTablePtr, &lpPckIndexTableSource->cFileIndex);

		lpPckIndexTableSource->cFileIndex.dwAddressOffset = dwSrcAddress;	//��ԭԭ���ĵ�ַ
		lpPckIndexTableSource->cFileIndex.dwFileCipherTextSize = dwFileCipherTextSizeSrc;

		lpFileWrite->UnmapView();

		++lpPckIndexTablePtr;
		++lpPckIndexTableSource;
		++(lpPckParams->cVarParams.dwUIProgress);

	}

	if (NULL != lpDecompressBuffer)
		free(lpDecompressBuffer);

	return TRUE;
}
#endif
//��ѹ���ļ�
BOOL CPckClass::RecompressPckFile(LPTSTR szRecompressPckFile)
{

	PrintLogI(TEXT_LOG_RECOMPRESS);

	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTable, lpPckIndexTablePtr;
	QWORD	dwAddress = PCK_DATA_START_AT, dwAddressName;
	DWORD	dwFileCount = m_PckAllInfo.dwFileCount;
	DWORD	dwNoDupFileCount = ReCountFiles();
	QWORD	dwTotalFileSizeAfterRebuild = m_PckAllInfo.qwPckSize + PCK_STEP_ADD_SIZE;

	CMapViewFileWrite	*lpFileWrite;
	CMapViewFileRead	*lpFileRead;
	int					threadnum = lpPckParams->dwMTThread;

	//����ͷ��βʱ��Ҫ�Ĳ���
	PCK_ALL_INFOS		pckAllInfo;

	//���ý����������ֵ
	lpPckParams->cVarParams.dwUIProgressUpper = dwNoDupFileCount;

	//��Դ�ļ� 
	lpFileRead = new CMapViewFileRead();

	if (!OpenPckAndMappingRead(lpFileRead, m_PckAllInfo.szFilename, m_szMapNameRead)) {
		delete lpFileRead;
		return FALSE;
	}

	BeforeSingleOrMultiThreadProcess(&pckAllInfo, lpFileWrite, szRecompressPckFile, CREATE_ALWAYS, dwTotalFileSizeAfterRebuild, threadnum);
	initCompressedDataQueue(threadnum, dwFileCount, PCK_DATA_START_AT);

	if (PCK_COMPRESS_NEED_ST < threadnum) {

		//mt_dwAddress = dwAddress;
		mt_lpFileRead = lpFileRead;
		mt_lpFileWrite = lpFileWrite;
		mt_dwFileCount = dwFileCount;
		mt_dwFileCountOfWriteTarget = dwNoDupFileCount;
		//mt_level = lpPckParams->dwCompressLevel;

		MultiThreadInitialize(CompressThreadRecompress, WriteThread, threadnum);

		//д�ļ�����
		lpPckIndexTablePtr = lpPckIndexTable = mt_lpPckIndexTable;
		dwAddressName = dwAddress = mt_dwAddressQueue;


		//��������߳�ʹ�õ���ѹ�����λ
		LPPCKINDEXTABLE lpPckIndexTablePtrSrc = m_lpPckIndexTable;

		for (DWORD i = 0; i < m_PckAllInfo.dwFileCount; ++i) {

			lpPckIndexTablePtrSrc->isRecompressed = FALSE;
			lpPckIndexTablePtrSrc++;
		}
	}
#if PCK_COMPRESS_NEED_ST
	else {

		RecompressPckFileSingleThread(lpFileRead, lpFileWrite, dwFileCount, dwAddress, lpPckIndexTable);

		//д�ļ�����
		lpPckIndexTablePtr = lpPckIndexTable;
		dwAddressName = dwAddress;

	}
#endif
	//�رն��ļ�
	delete lpFileRead;

	//ȡ�����ļ�����
	dwNoDupFileCount = mt_dwFileCountOfWriteTarget;

	WritePckIndexTable(lpFileWrite, mt_lpPckIndexTable, dwNoDupFileCount, dwAddress);

	pckAllInfo.dwAddressName = dwAddressName;
	pckAllInfo.dwFileCount = dwNoDupFileCount;
	AfterProcess(lpFileWrite, pckAllInfo, dwAddress);

	//���������ý���Ϊ100%
	//lpPckParams->cVarParams.dwUIProgress = lpPckParams->cVarParams.dwUIProgressUpper;

	delete lpFileWrite;

	uninitCompressedDataQueue(threadnum);

	PrintLogN(TEXT_LOG_WORKING_DONE);

	return TRUE;
}
