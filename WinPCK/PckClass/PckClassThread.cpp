//////////////////////////////////////////////////////////////////////
// PckClassThread.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ��Ҫ���ܣ�����ѹ�������¡���������
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"
#include <process.h>

#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4302 )

#include "PckClassThreadVariables.cpp"
#include "PckClassThreadCreate.cpp"
#include "PckClassThreadAddFile.cpp"
#include "PckClassThreadRecompress.cpp"
/*
** ���߳�ѹ��ʱ��ز����ĳ�ʼ��
**
**
*/

BOOL CPckClass::BeforeSingleOrMultiThreadProcess(LPPCK_ALL_INFOS lpPckAllInfo, CMapViewFileWrite* &lpWrite, LPTSTR szPckFile, DWORD dwCreationDisposition, QWORD qdwSizeToMap, int threadnum)
{
	//����ͷ��βʱ��Ҫ�Ĳ���
	memset(lpPckAllInfo, 0, sizeof(PCK_ALL_INFOS));
	lpPckAllInfo->lpSaveAsPckVerFunc = m_PckAllInfo.lpSaveAsPckVerFunc;

	//��ʼѹ��
	//�����Ǵ���һ���ļ�����������ѹ������ļ�
	lpWrite = new CMapViewFileWrite(m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->dwMaxSinglePckSize);

	//OPEN_ALWAYS���½��µ�pck(CREATE_ALWAYS)����´��ڵ�pck(OPEN_EXISTING)
	if (!OpenPckAndMappingWrite(lpWrite, szPckFile, dwCreationDisposition, qdwSizeToMap)) {

		delete lpWrite;
		return FALSE;
	}

	lpPckParams->cVarParams.dwUIProgress = 0;

	//���߳�ʹ�ò�����ʼ��
	if (PCK_COMPRESS_NEED_ST < threadnum)
	{
		mt_lpbThreadRunning = &lpPckParams->cVarParams.bThreadRunning;	//�����߳���ͣ
		mt_lpdwCount = &lpPckParams->cVarParams.dwUIProgress;			//dwCount;
		mt_MaxMemory = lpPckParams->dwMTMaxMemory;						//������󻺴�;
		mt_lpMaxMemory = &lpPckParams->cVarParams.dwMTMemoryUsed;		//��ʹ�û���;
		mt_nMallocBlocked = 0;											//�ȴ����߳���
	}

	return TRUE;
}

void CPckClass::MultiThreadInitialize(VOID CompressThread(VOID*), VOID WriteThread(VOID*), int threadnum)
{
	mt_evtAllWriteFinish = CreateEventA(NULL, FALSE, FALSE, m_szEventAllWriteFinish);
	mt_evtAllCompressFinish = CreateEventA(NULL, TRUE, FALSE, m_szEventAllCompressFinish);
	mt_evtMaxMemory = CreateEventA(NULL, FALSE, FALSE, m_szEventMaxMemory);

	mt_threadID = 0;

	InitializeCriticalSection(&g_cs);
	InitializeCriticalSection(&g_mt_threadID);
	InitializeCriticalSection(&g_mt_nMallocBlocked);
	InitializeCriticalSection(&g_mt_lpMaxMemory);
	InitializeCriticalSection(&g_dwCompressedflag);
	InitializeCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

	for (int i = 0; i < threadnum; i++)
	{
		_beginthread(CompressThread, 0, this);
	}
	_beginthread(WriteThread, 0, this);

	WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);

	//SetEvent(mt_evtToWrite);		//��ֹWrite�߳̿�ס 
	WaitForSingleObject(mt_evtAllWriteFinish, INFINITE);

	DeleteCriticalSection(&g_cs);
	DeleteCriticalSection(&g_mt_threadID);
	DeleteCriticalSection(&g_mt_nMallocBlocked);
	DeleteCriticalSection(&g_mt_lpMaxMemory);
	DeleteCriticalSection(&g_dwCompressedflag);
	DeleteCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

	CloseHandle(mt_evtMaxMemory);
	CloseHandle(mt_evtAllCompressFinish);
	CloseHandle(mt_evtAllWriteFinish);

	if (!lpPckParams->cVarParams.bThreadRunning)
	{
		PrintLogW(TEXT_USERCANCLE);
	}

}


//�������ļ�
BOOL CPckClass::RenameFilename()
{

	PrintLogI(TEXT_LOG_RENAME);

	int			level = lpPckParams->dwCompressLevel;
	DWORD		IndexCompressedFilenameDataLengthCryptKey1 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2 = m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey2;

	//�����Ǵ���һ���ļ�����������ѹ������ļ�
	CMapViewFileWrite *lpFileWrite = new CMapViewFileWrite(m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->dwMaxSinglePckSize);

	if (!lpFileWrite->OpenPck(m_PckAllInfo.szFilename, OPEN_EXISTING)) {

		PrintLogE(TEXT_OPENWRITENAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);
		delete lpFileWrite;
		return FALSE;
	}

	QWORD dwFileSize = lpFileWrite->GetFileSize() + PCK_RENAME_EXPAND_ADD;

	if (!lpFileWrite->Mapping(m_szMapNameWrite, dwFileSize)) {

		PrintLogE(TEXT_CREATEMAPNAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);
		delete lpFileWrite;
		return FALSE;
	}

	//д�ļ�����
	QWORD dwAddress = m_PckAllInfo.dwAddressName;

	//����������ʾ���ļ����ȣ���ʼ������ʾд��������mt_hFileToWrite
	lpPckParams->cVarParams.dwUIProgress = 0;
	lpPckParams->cVarParams.dwUIProgressUpper = m_PckAllInfo.dwFileCount;

	//дԭ�����ļ�
	LPPCKINDEXTABLE	lpPckIndexTableSrc = m_lpPckIndexTable;

	//д������
	for (DWORD i = 0; i < lpPckParams->cVarParams.dwUIProgressUpper; ++i)
	{

		PCKINDEXTABLE_COMPRESS	pckIndexTableTemp;

		if (lpPckIndexTableSrc->bSelected)
		{
			--m_PckAllInfo.dwFileCount;
			++lpPckIndexTableSrc;

			//����������ʾ���ļ�����
			++lpPckParams->cVarParams.dwUIProgress;
			continue;
		}

		FillAndCompressIndexData(&pckIndexTableTemp, &lpPckIndexTableSrc->cFileIndex);

		WritePckIndex(lpFileWrite, &pckIndexTableTemp, dwAddress);

		++lpPckIndexTableSrc;

	}
	
	AfterProcess(lpFileWrite, m_PckAllInfo, dwAddress, FALSE);

	delete lpFileWrite;

	PrintLogI(TEXT_LOG_WORKING_DONE);

	return TRUE;
}


void CPckClass::detectMaxAndAddMemory(DWORD dwMallocSize)
{
	EnterCriticalSection(&g_mt_lpMaxMemory);

	if ((*mt_lpMaxMemory) >= mt_MaxMemory) {
		LeaveCriticalSection(&g_mt_lpMaxMemory);
		{
			EnterCriticalSection(&g_mt_nMallocBlocked);
			mt_nMallocBlocked++;
			LeaveCriticalSection(&g_mt_nMallocBlocked);
		}
		WaitForSingleObject(mt_evtMaxMemory, INFINITE);
		{
			EnterCriticalSection(&g_mt_nMallocBlocked);
			mt_nMallocBlocked--;
			LeaveCriticalSection(&g_mt_nMallocBlocked);
		}
	} else
		LeaveCriticalSection(&g_mt_lpMaxMemory);

	{
		EnterCriticalSection(&g_mt_lpMaxMemory);
		(*mt_lpMaxMemory) += dwMallocSize;
		LeaveCriticalSection(&g_mt_lpMaxMemory);
	}
}

void CPckClass::freeMaxAndSubtractMemory(DWORD dwMallocSize)
{
	EnterCriticalSection(&g_mt_lpMaxMemory);
	(*mt_lpMaxMemory) -= dwMallocSize;
	if ((*mt_lpMaxMemory) < mt_MaxMemory) {
		LeaveCriticalSection(&g_mt_lpMaxMemory);

		EnterCriticalSection(&g_mt_nMallocBlocked);
		int nMallocBlocked = mt_nMallocBlocked;
		LeaveCriticalSection(&g_mt_nMallocBlocked);
		for (int i = 0; i<nMallocBlocked; i++)
			SetEvent(mt_evtMaxMemory);

	} else {
		LeaveCriticalSection(&g_mt_lpMaxMemory);
	}
}

BOOL CPckClass::initCompressedDataQueue(int threadnum, DWORD dwFileCount, QWORD dwAddressStartAt)
{
	//����ռ�,�ļ���ѹ������ ����
	if (NULL == (mt_lpPckIndexTable = new PCKINDEXTABLE_COMPRESS[dwFileCount]))
	{
		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	memset(mt_lpPckIndexTable, 0, sizeof(PCKINDEXTABLE_COMPRESS) * dwFileCount);

	if (PCK_COMPRESS_NEED_ST < threadnum)
	{
		if (NULL == (mt_pckCompressedDataPtrArray = (BYTE**)malloc(sizeof(BYTE*) * (dwFileCount + 1))))
		{
			PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
		memset(mt_pckCompressedDataPtrArray, 0, sizeof(BYTE*) * (dwFileCount + 1));

		mt_lpPckIndexTablePut = mt_lpPckIndexTableGet = mt_lpPckIndexTable;
		mt_pckCompressedDataPtrArrayPut = mt_pckCompressedDataPtrArrayGet = mt_pckCompressedDataPtrArray;
		mt_dwMaxQueueLength = dwFileCount;
#ifdef _DEBUG
		mt_dwCurrentQueuePosPut = mt_dwCurrentQueuePosGet = 0;
#endif
		mt_dwCurrentQueueLength = 0;
		mt_dwAddressNameQueue = mt_dwAddressQueue = dwAddressStartAt;
	}

	return TRUE;
}

BOOL CPckClass::putCompressedDataQueue(LPBYTE lpBuffer, LPPCKINDEXTABLE_COMPRESS lpPckIndexTableComped, LPPCKFILEINDEX lpPckFileIndexToCompress)
{
	//�Ȳ�����ptr>count�����

	lpPckIndexTableComped->dwCompressedFilesize = lpPckFileIndexToCompress->dwFileCipherTextSize;
	EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
	//lpPckIndexTableComped->dwAddressFileDataToWrite = lpPckFileIndexToCompress->dwAddressOffset = mt_dwAddressQueue;

	if (0 == lpPckIndexTableComped->dwAddressOfDuplicateOldDataArea) {
		lpPckIndexTableComped->dwAddressFileDataToWrite = lpPckFileIndexToCompress->dwAddressOffset = mt_dwAddressQueue;
		mt_dwAddressQueue += lpPckFileIndexToCompress->dwFileCipherTextSize;
		lpPckIndexTableComped->dwAddressAddStep = lpPckFileIndexToCompress->dwFileCipherTextSize;
	} else {
		lpPckIndexTableComped->dwAddressFileDataToWrite = lpPckIndexTableComped->dwAddressOfDuplicateOldDataArea;
		lpPckIndexTableComped->dwAddressAddStep = 0;
	}

	if (!lpPckIndexTableComped->bInvalid)
	{
		FillAndCompressIndexData(lpPckIndexTableComped, lpPckFileIndexToCompress);
	}

#ifdef _DEBUG
	if (1 == (int)lpBuffer) {
		logOutput(__FUNCTION__, formatString("id(%05d), lpBuffer(0x%08x):0x0000000000000000, lpPckIndexTable(0x%08x), szFile(%s)\r\n", mt_dwCurrentQueuePosPut, (int)lpBuffer, (int)mt_lpPckIndexTablePut, lpPckFileIndexToCompress->szFilename));
	}
	else {
		logOutput(__FUNCTION__, formatString("id(%05d), lpBuffer(0x%08x):0x%016llx, lpPckIndexTable(0x%08x), szFile(%s)\r\n", mt_dwCurrentQueuePosPut, (int)lpBuffer, _byteswap_uint64(*(unsigned __int64*)lpBuffer), (int)mt_lpPckIndexTablePut, lpPckFileIndexToCompress->szFilename));
	}

	mt_dwCurrentQueuePosPut++;

	assert(NULL != lpBuffer);
#endif

	(*mt_pckCompressedDataPtrArrayPut) = lpBuffer;
	memcpy(mt_lpPckIndexTablePut, lpPckIndexTableComped, sizeof(PCKINDEXTABLE_COMPRESS));

	mt_dwCurrentQueueLength++;
	mt_lpPckIndexTablePut++;
	mt_pckCompressedDataPtrArrayPut++;
	LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

	return TRUE;
}

BOOL CPckClass::getCompressedDataQueue(LPBYTE &lpBuffer, LPPCKINDEXTABLE_COMPRESS &lpPckIndexTable)
{

	EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
	if (mt_dwCurrentQueueLength) {

		mt_dwCurrentQueueLength--;
		lpBuffer = *mt_pckCompressedDataPtrArrayGet;
		lpPckIndexTable = mt_lpPckIndexTableGet;

		LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

#ifdef _DEBUG

		assert(NULL != lpBuffer);

		if (0 != lpPckIndexTable->dwIndexDataLength) {
			char szFile[MAX_INDEXTABLE_CLEARTEXT_LENGTH];
			DWORD dwTest = MAX_INDEXTABLE_CLEARTEXT_LENGTH;

			decompress(szFile, &dwTest, lpPckIndexTable->buffer, lpPckIndexTable->dwIndexDataLength);
			if (1 == (int)lpBuffer) {
				logOutput(__FUNCTION__, formatString("id(%05d), lpBuffer(0x%08x):0x0000000000000000, lpPckIndexTable(0x%08x), szFile(%s)\r\n", mt_dwCurrentQueuePosGet, (int)lpBuffer, (int)lpPckIndexTable, szFile));
			}
			else {
				logOutput(__FUNCTION__, formatString("id(%05d), lpBuffer(0x%08x):0x%016llx, lpPckIndexTable(0x%08x), szFile(%s)\r\n", mt_dwCurrentQueuePosGet, (int)lpBuffer, _byteswap_uint64(*(unsigned __int64*)lpBuffer), (int)lpPckIndexTable, szFile));
			}
		}
		mt_dwCurrentQueuePosGet++;
#endif

		mt_lpPckIndexTableGet++;
		mt_pckCompressedDataPtrArrayGet++;

		return TRUE;
	} else {
		LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
		return FALSE;
	}	
}

void CPckClass::uninitCompressedDataQueue(int threadnum)
{
	delete[] mt_lpPckIndexTable;

	if (PCK_COMPRESS_NEED_ST < threadnum)
		free(mt_pckCompressedDataPtrArray);
}