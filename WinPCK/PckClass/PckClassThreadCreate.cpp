
#define COMPRESSTHREADFUNC CompressThreadCreate
#define COMPRESSSINGLETHREADFUNC CreatePckFileSingleThread
#define TARGET_PCK_MODE_COMPRESS PCK_MODE_COMPRESS_CREATE
#include "PckClassThreadCompressFunctions.h"


VOID CPckClass::WriteThread(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	LPBYTE		dataToWrite;
	QWORD		dwTotalFileSizeAfterCompress = mt_CompressTotalFileSize;
	LPBYTE		lpBufferToWrite;
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTableComp;

	QWORD						dwAddressDataAreaEndAt = mt_dwAddressNameQueue;
	DWORD						nWrite = 0;

	while (1)
	{
		if (pThis->getCompressedDataQueue(dataToWrite, lpPckIndexTableComp))
		{
			QWORD dwAddress = lpPckIndexTableComp->dwAddressFileDataToWrite;

			//�˴�����Ӧ����WriteThread�߳���ʵ��
			//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
			//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
			//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
			if (!pThis->IsNeedExpandWritingFile(mt_lpFileWrite, dwAddress, lpPckIndexTableComp->dwCompressedFilesize, dwTotalFileSizeAfterCompress))
			{

				pThis->AfterWriteThreadFailProcess(
					mt_lpbThreadRunning,
					mt_evtAllCompressFinish,
					mt_dwFileCount,
					nWrite,
					mt_dwAddressQueue,
					dwAddressDataAreaEndAt,
					mt_pckCompressedDataPtrArrayGet);

				break;

			}

			//����lpPckFileIndex->dwAddressOffset
			if (0 != lpPckIndexTableComp->dwCompressedFilesize)
			{

				if (NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddress, lpPckIndexTableComp->dwCompressedFilesize)))
				{
					//д������Ŀ���ļ��޷�View
					mt_lpFileWrite->UnMaping();

					pThis->AfterWriteThreadFailProcess(
						mt_lpbThreadRunning,
						mt_evtAllCompressFinish,
						mt_dwFileCount,
						nWrite,
						mt_dwAddressQueue,
						dwAddressDataAreaEndAt,
						mt_pckCompressedDataPtrArrayGet);

					break;
				}

				memcpy(lpBufferToWrite, dataToWrite, lpPckIndexTableComp->dwCompressedFilesize);

				dwAddressDataAreaEndAt += lpPckIndexTableComp->dwAddressAddStep;

				free(dataToWrite);
				mt_lpFileWrite->UnmapView();
			}

			nWrite++;

			pThis->freeMaxAndSubtractMemory(lpPckIndexTableComp->dwMallocSize);

			if (mt_dwFileCountOfWriteTarget == nWrite)
			{
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
				mt_dwAddressQueue = dwAddressDataAreaEndAt;

				break;
			}

		}
		else {

			if (!(*mt_lpbThreadRunning))
			{
				//ȡ��
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);

				mt_dwFileCountOfWriteTarget = nWrite;
				mt_dwAddressQueue = dwAddressDataAreaEndAt;

				break;
			}

			::Sleep(10);

		}

	}

	SetEvent(mt_evtAllWriteFinish);

	_endthread();
}


BOOL CPckClass::CreatePckFile(LPTSTR szPckFile, LPTSTR szPath)
{
	char		szLogString[LOG_BUFFER];

	sprintf_s(szLogString, TEXT_LOG_CREATE, UCS2toA(szPckFile));
	PrintLogI(szLogString);

	DWORD		dwFileCount = 0;									//�ļ�����, ԭpck�ļ��е��ļ���
	QWORD		qwTotalFileSize = 0, qwTotalFileSizeTemp;			//δѹ��ʱ�����ļ���С
	QWORD		dwAddress = PCK_DATA_START_AT;

	DWORD		IndexCompressedFilenameDataLengthCryptKey1 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey2;

	char		szPathMbsc[MAX_PATH];

	int			level = lpPckParams->dwCompressLevel;
	int			threadnum = lpPckParams->dwMTThread;

	//LOG
	sprintf_s(szLogString, TEXT_LOG_LEVEL_THREAD, level, threadnum);
	PrintLogI(szLogString);

	//��ʼ�����ļ�
	LPFILES_TO_COMPRESS		lpfirstFile;

	size_t nLen = lstrlen(szPath) - 1;
	if ('\\' == *(szPath + nLen))*(szPath + nLen) = 0;
	BOOL	IsPatition = lstrlen(szPath) == 2 ? TRUE : FALSE;

	nLen = WideCharToMultiByte(CP_ACP, 0, szPath, -1, szPathMbsc, MAX_PATH, "_", 0);

	if (NULL == (m_firstFile = AllocateFileinfo())) {

		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	lpfirstFile = m_firstFile;

	//���������ļ�
	EnumFile(szPathMbsc, IsPatition, dwFileCount, lpfirstFile, qwTotalFileSize, nLen);
	if (0 == dwFileCount)return TRUE;

	//�ļ���д�봰�����б�������ʾ����
	mt_dwFileCount = lpPckParams->cVarParams.dwUIProgressUpper = dwFileCount;

	//��������Ҫ���ռ�qwTotalFileSize
	qwTotalFileSizeTemp = qwTotalFileSize * 0.6;

	if (((0 != (qwTotalFileSizeTemp >> 32)) && (0x20002 == m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->Version)) || \
		((0 != (qwTotalFileSizeTemp >> 33)) && (0x20003 == m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->Version))) {

		PrintLogE(TEXT_COMPFILE_TOOBIG, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	mt_CompressTotalFileSize = qwTotalFileSizeTemp;
	if (PCK_SPACE_DETECT_SIZE >= mt_CompressTotalFileSize)mt_CompressTotalFileSize = PCK_STEP_ADD_SIZE;

	PCK_ALL_INFOS	pckAllInfo;
	BeforeSingleOrMultiThreadProcess(&pckAllInfo, mt_lpFileWrite, szPckFile, CREATE_ALWAYS, mt_CompressTotalFileSize, threadnum);
	initCompressedDataQueue(threadnum, mt_dwFileCountOfWriteTarget = mt_dwFileCount, dwAddress);

	if (PCK_COMPRESS_NEED_ST < threadnum) {

		MultiThreadInitialize(CompressThreadCreate, WriteThread, threadnum);
		dwAddress = mt_dwAddressQueue;
	}
#if PCK_COMPRESS_NEED_ST
	else {

		CreatePckFileSingleThread(dwAddress);
	}
#endif

	//LOG
	PrintLogI(TEXT_LOG_COMPRESSOK);

	//д�ļ�����
	pckAllInfo.dwAddressName = dwAddress;
	WritePckIndexTable(mt_lpFileWrite, mt_lpPckIndexTable, mt_dwFileCountOfWriteTarget, dwAddress);

	pckAllInfo.dwFileCount = mt_dwFileCountOfWriteTarget;
	AfterProcess(mt_lpFileWrite, pckAllInfo, dwAddress);

	delete mt_lpFileWrite;
	DeallocateFileinfo();

	uninitCompressedDataQueue(threadnum);

	//LOG
	PrintLogI(TEXT_LOG_WORKING_DONE);

	return TRUE;
}

