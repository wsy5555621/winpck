
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

#include "zlib.h"

#include "PckClass.h"
#include <process.h>


#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4311 )

_inline char * __fastcall mystrcpy(char * dest,const char *src)
{
        while ((*dest = *src))
                dest++, src++;
        return dest;
}



/////////���±������ڶ��߳�pckѹ���Ĳ���
CRITICAL_SECTION	g_cs;
CRITICAL_SECTION	g_mt_threadID;
CRITICAL_SECTION	g_mt_nMallocBlocked;
CRITICAL_SECTION	g_mt_lpMaxMemory;
CRITICAL_SECTION	g_dwCompressedflag;
CRITICAL_SECTION	g_mt_pckCompressedDataPtrArrayPtr;


int					mt_threadID;		//�߳�ID

LPPCKINDEXTABLE_COMPRESS	mt_lpPckIndexTable, mt_lpPckIndexTablePtr;				//ѹ������ļ�����������ȫ��ָ��
BYTE**				mt_pckCompressedDataPtrArray, **mt_pckCompressedDataPtrArrayPtr;//�ڴ�����ָ������鼰��ȫ��ָ��
//HANDLE			mt_evtToWrite;														//�߳��¼�
HANDLE				mt_evtAllWriteFinish;											//�߳��¼�
HANDLE				mt_evtAllCompressFinish;										//�߳��¼�
HANDLE				mt_evtMaxMemory;												//�߳��¼�
//HANDLE				mt_hFileToWrite, mt_hMapFileToWrite;							//ȫ��д�ļ��ľ��
CMapViewFileWrite	*mt_lpFileWrite;												//ȫ��д�ļ��ľ��
QWORD				mt_CompressTotalFileSize;										//Ԥ�Ƶ�ѹ���ļ���С
LPDWORD				mt_lpdwCount;													//�ӽ����̴߳��������ļ�����ָ��
DWORD				mt_dwFileCount;													//�ӽ����̴߳����������ļ�����ָ��
PCKADDR				mt_dwAddress;													//ȫ��ѹ�����̵�д�ļ���λ��

//size_t			mt_nLen;															//������Ŀ¼�ĳ��ȣ����ڽ�ȡ�ļ����ŵ�pck��
BOOL				*mt_lpbThreadRunning;											//�߳��Ƿ�������״̬��ֵ����Ϊfalseʱ�߳��˳�
DWORD				*mt_lpMaxMemory;												//ѹ����������ʹ�õ�����ڴ棬����һ�����������õģ����ôӽ����̴߳�������ֵ������
DWORD				mt_MaxMemory;													//ѹ����������ʹ�õ�����ڴ�
int					mt_nMallocBlocked;												//�򻺴����걻��ͣ���߳���
DWORD				mt_level;														//ѹ���ȼ�


//���ʱʹ�ñ���
PCKADDR				mt_dwAddressName;												//������pck�ļ���ѹ���ļ���������ʼλ��
char				mt_szCurrentNodeString[MAX_PATH_PCK];							//�������߳��е�ǰ��ʾ�ģ��ڵ��Ӧ��pck�е��ļ�·��
int					mt_nCurrentNodeStringLen;										//�䳤��



VOID CPckClass::CompressThread(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	char	szFileMappingNameSpaceFormat[16];
	char	szFileMappingNameSpace[32];
	int		level = mt_level;

	DWORD	IndexCompressedFilenameDataLengthCryptKey1 = pThis->m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD	IndexCompressedFilenameDataLengthCryptKey2 = pThis->m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2;

	
	memcpy(szFileMappingNameSpaceFormat, pThis->m_szMapNameRead, 16);
	strcat_s(szFileMappingNameSpaceFormat, 16, "%d");

	EnterCriticalSection(&g_mt_threadID);
	sprintf_s(szFileMappingNameSpace, 32, szFileMappingNameSpaceFormat, mt_threadID++);
	LeaveCriticalSection(&g_mt_threadID); 

	BYTE	*bufCompressData = (BYTE*)1;
	BYTE	**ptrCurrentIndex;		//�ݴ�mt_pckCompressedDataPtrArrayPtr
	
	//HANDLE	hFileToRead;
	//HANDLE	hMapFileToRead;
	//LPVOID	lpMapAddressToRead;
	LPBYTE		lpBufferToRead;

	
	PCKFILEINDEX		pckFileIndex;

	//memset(pckFileIndex.dwAttachedValue, 0, sizeof(pckFileIndex.dwAttachedValue));
	pckFileIndex.dwUnknown1 = pckFileIndex.dwUnknown2 = 0;
#ifdef PCKV203ZX
	pckFileIndex.dwUnknown3 = pckFileIndex.dwUnknown4 = pckFileIndex.dwUnknown5 = 0;
#endif

	//��ʼ
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;
	LPFILES_TO_COMPRESS			lpfirstFile = pThis->m_firstFile;

	memset(pckFileIndex.szFilename, 0, MAX_PATH_PCK);

	//patch 140424
	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

	while(NULL != lpfirstFile->next)
	{
		EnterCriticalSection(&g_dwCompressedflag);
		if(0 == lpfirstFile->dwCompressedflag)
		{
			lpfirstFile->dwCompressedflag = 1;

			LeaveCriticalSection(&g_dwCompressedflag); 

			if(PCK_BEGINCOMPRESS_SIZE < lpfirstFile->dwFileSize)
			{
				pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
				pckFileIndex.dwFileCipherTextSize = compressBound(lpfirstFile->dwFileSize);//lpfirstFile->dwFileSize * 1.001 + 12;
				lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize;
			}else{
				pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
			}

			//�ж�ʹ�õ��ڴ��Ƿ񳬹����ֵ
			EnterCriticalSection(&g_mt_lpMaxMemory);
			if( (*mt_lpMaxMemory) >= mt_MaxMemory)
			{
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
			}else
				LeaveCriticalSection(&g_mt_lpMaxMemory);
			
			{
				EnterCriticalSection(&g_mt_lpMaxMemory);
				(*mt_lpMaxMemory) += lpfirstFile->dwFileSize;
				LeaveCriticalSection(&g_mt_lpMaxMemory);
			}

			//pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;

			//lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize * 1.001 + 12;
			
			memcpy(pckFileIndex.szFilename, lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy);
			//pckFileIndex.dwUnknown1 = pckFileIndex.dwUnknown2 = 0;

			if(!*(mt_lpbThreadRunning))
			{
				//int i = 1;
				break;
			}

			//����ļ���СΪ0�����������ļ�����
			if(0 != pckFileIndex.dwFileClearTextSize)
			{
				//lpFileRead = new CMapViewFileRead();

				////////////////////////////��Դ�ļ�/////////////////////////////////
				//��Ҫ����ѹ�����ļ�
				if(!lpFileRead->Open(lpfirstFile->szFilename))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();
					break;
				}

				//����һ���ļ�ӳ��
				if(!lpFileRead->Mapping(szFileMappingNameSpace))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();
					break;
				}


				//����һ���ļ�ӳ�����ͼ������Ϊsource
				if(NULL == (lpBufferToRead = lpFileRead->View(0, 0)))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();
					break;
				}

				////////////////////////////��Դ�ļ�/////////////////////////////////

				//lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize * 1.001 + 12;
				//

				bufCompressData = (BYTE*) malloc (pckFileIndex.dwFileCipherTextSize);
			
				if(PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileClearTextSize)
				{
					compress2(bufCompressData, &pckFileIndex.dwFileCipherTextSize, 
									lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
				}else{
					memcpy((BYTE*)bufCompressData, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
				}

				//delete lpFileRead;
				lpFileRead->clear();

			}//�ļ���Ϊ0ʱ�Ĵ���

			{

				EnterCriticalSection(&g_cs);

				ptrCurrentIndex = mt_pckCompressedDataPtrArrayPtr++;

				lpPckIndexTablePtr = mt_lpPckIndexTablePtr;
				mt_lpPckIndexTablePtr++;

				//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
				pckFileIndex.dwAddressOffset = mt_dwAddress;		//���ļ���ѹ��������ʼ��ַ
				mt_dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ
				
				//����������ʾ���ļ�����
				(*mt_lpdwCount)++;

				LeaveCriticalSection(&g_cs);
			}

			//test
			//lpPckIndexTablePtr->dwAddress = pckFileIndex.dwAddressOffset;//////test

			lpPckIndexTablePtr->dwCompressedFilesize = pckFileIndex.dwFileCipherTextSize;
			lpPckIndexTablePtr->dwMallocSize = lpfirstFile->dwFileSize;

			lpPckIndexTablePtr->dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
			compress2(lpPckIndexTablePtr->buffer, &lpPckIndexTablePtr->dwIndexDataLength, 
							(BYTE*)&pckFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
			//����ȡ��
			lpPckIndexTablePtr->dwIndexValueHead = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
			lpPckIndexTablePtr->dwIndexValueTail = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;


			EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//int		ilen = sizeof(BYTE*);
			//BYTE	**tes1t = mt_pckCompressedDataPtrArrayPtr - 1;
			*(ptrCurrentIndex) = bufCompressData;
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//SetEvent(mt_evtToWrite);//��ΪSleep(10);
			
		}else{

			LeaveCriticalSection(&g_dwCompressedflag);
		}
		//���ܵ��ļ�����������
		//lpPckIndexTablePtr++;
		//lpPckFileIndex++;
		//lppckCompressedDataPtrArray++;

		//��һ���ļ��б�
		lpfirstFile = lpfirstFile->next;
	}

	delete lpFileRead;

	EnterCriticalSection(&g_mt_threadID);

	mt_threadID--;
	if(0 == mt_threadID)
		SetEvent(mt_evtAllCompressFinish);

	LeaveCriticalSection(&g_mt_threadID); 
	

	_endthread();
}


VOID CPckClass::WriteThread(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	BYTE**		bufferPtrToWrite = mt_pckCompressedDataPtrArray;
	PCKADDR		dwTotalFileSizeAfterCompress = mt_CompressTotalFileSize;

	//mt_lpFileWrite
	//HANDLE	hFileToWrite = mt_hFileToWrite;
	//HANDLE	hMapFileToWrite = mt_hMapFileToWrite;
	//LPVOID	lpMapAddressToWrite;
	LPBYTE		lpBufferToWrite;

	LPFILES_TO_COMPRESS			lpfirstFile = pThis->m_firstFile;
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;

	PCKADDR						dwAddress = PCK_DATA_START_AT;
	DWORD						nWrite = 0;
	//DWORD						dwFileCount;

	//���ڲ��Ե�һ���ı���
	//HWND	hTest = CreateWindow(TEXT("EDIT"), TEXT("DebugString"), WS_OVERLAPPEDWINDOW | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, (HMENU)1001, TApp::GetInstance(), NULL);

	//VS2010��һ��bug,����ʱ��ʼ�����������¹�����θı���ֵ�����صĶ� �ǳ�ʼֵ 

	while(1)
	{
		
		EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
		if(NULL != (*bufferPtrToWrite))
		{
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//�˴�����Ӧ����WriteThread�߳���ʵ��
			//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
			//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
			//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
			//1mb=0x100000
			//64mb=0x4000000
			if((dwAddress + lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) > dwTotalFileSizeAfterCompress)
			{
		
				dwTotalFileSizeAfterCompress += 
					((lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) > PCK_STEP_ADD_SIZE ? (lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) : PCK_STEP_ADD_SIZE);

				mt_lpFileWrite->UnMaping();

				if(!mt_lpFileWrite->Mapping(pThis->m_szMapNameWrite, dwTotalFileSizeAfterCompress))
				{
					{
						EnterCriticalSection(&g_cs);
						*(mt_lpbThreadRunning) = FALSE;
						LeaveCriticalSection(&g_cs);
					}

					WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
					mt_dwFileCount = nWrite;
					mt_dwAddress = dwAddress;

					//�ͷ�
					while(0 != *bufferPtrToWrite)
					{
						free(*bufferPtrToWrite);
						bufferPtrToWrite++;
					}

					break;

				}

			}

			//����lpPckFileIndex->dwAddressOffset
			//�ļ�ӳ���ַ������64k(0x10000)������

			if(0 != lpPckIndexTablePtr->dwCompressedFilesize)
			{

				//lpBufferToWrite = mt_lpFileWrite->View(dwAddress, lpPckIndexTablePtr->dwCompressedFilesize)

				//DWORD	dwMapViewBlockHigh, dwMapViewBlockLow;

				//dwMapViewBlockHigh = dwAddress & 0xffff0000;
				//dwMapViewBlockLow = dwAddress & 0xffff;

				if(NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddress, lpPckIndexTablePtr->dwCompressedFilesize)))
				{
					mt_lpFileWrite->UnMaping();
					{
						EnterCriticalSection(&g_cs);
						*(mt_lpbThreadRunning) = FALSE;
						LeaveCriticalSection(&g_cs);
					}

					WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
					mt_dwFileCount = nWrite;
					mt_dwAddress = dwAddress;

					//�ͷ�
					while(0 != *bufferPtrToWrite)
					{
						free(*bufferPtrToWrite);
						bufferPtrToWrite++;
					}
					break;
				}
				//����ַ�Ƿ���ȷ
				//if(lpPckIndexTablePtr->dwAddress != dwAddress)OutputDebugStringA("lpPckIndexTablePtr->dwAddress != dwAddress!!!\r\n");

				memcpy(lpBufferToWrite, *bufferPtrToWrite, lpPckIndexTablePtr->dwCompressedFilesize);
				dwAddress += lpPckIndexTablePtr->dwCompressedFilesize;

				free(*bufferPtrToWrite);
				mt_lpFileWrite->UnmapView();
			}

			nWrite++;
			//char	str[64],str1[32];
			//sprintf(str, "%d", nWrite);
			//sprintf(str1, "%d", dwAddress);
			//ConfirmErrors(str1,str,MB_OK);
			
			{
				EnterCriticalSection(&g_mt_lpMaxMemory);
				(*mt_lpMaxMemory) -= lpPckIndexTablePtr->dwMallocSize;

				if((*mt_lpMaxMemory) < mt_MaxMemory)
				{
					LeaveCriticalSection(&g_mt_lpMaxMemory);
					
					{
						EnterCriticalSection(&g_mt_nMallocBlocked);
						int nMallocBlocked = mt_nMallocBlocked;
						LeaveCriticalSection(&g_mt_nMallocBlocked);
						for(int i=0;i<nMallocBlocked;i++)
							SetEvent(mt_evtMaxMemory);

					}


				}else{
					LeaveCriticalSection(&g_mt_lpMaxMemory);
				}
			}		

			if(mt_dwFileCount == nWrite)
			{
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
				mt_dwAddress = dwAddress;
				
				break;
			}
			

			bufferPtrToWrite++;
			lpPckIndexTablePtr++;

		}else{
			
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

			EnterCriticalSection(&g_cs);
			if(!(*mt_lpbThreadRunning))
			{
				LeaveCriticalSection(&g_cs);
				//ȡ��
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
				//lstrcpy(m_lastErrorString, TEXT_USERCANCLE);
				//char	str[64],str1[32];
				//sprintf(str, "%d", nWrite);
				//sprintf(str1, "%d", dwAddress);
				//ConfirmErrors(str1,str,MB_OK);
				mt_dwFileCount = nWrite;
				mt_dwAddress = dwAddress;

				break;
			}else{
				LeaveCriticalSection(&g_cs);
			}

			//ResetEvent(mt_evtToWrite);
			//WaitForSingleObject(mt_evtToWrite, INFINITE); //�ȴ����е����߳̽���
			::Sleep(10);

		}

	}

	//mt_hMapFileToWrite = hMapFileToWrite;
	SetEvent(mt_evtAllWriteFinish);
	
	_endthread();
}

VOID CPckClass::CompressThreadAdd(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	char	szFileMappingNameSpaceFormat[16];
	char	szFileMappingNameSpace[32];
	int		level = mt_level;

	DWORD	IndexCompressedFilenameDataLengthCryptKey1 = pThis->m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD	IndexCompressedFilenameDataLengthCryptKey2 = pThis->m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2;


	
	memcpy(szFileMappingNameSpaceFormat, pThis->m_szMapNameRead, 16);
	strcat_s(szFileMappingNameSpaceFormat, 16, "%d");

	EnterCriticalSection(&g_mt_threadID);
	sprintf_s(szFileMappingNameSpace, 32, szFileMappingNameSpaceFormat, mt_threadID++);
	LeaveCriticalSection(&g_mt_threadID); 

	BYTE	*bufCompressData = (BYTE*)1;
	BYTE	**ptrCurrentIndex;		//�ݴ�mt_pckCompressedDataPtrArrayPtr
	
	//HANDLE	hFileToRead;
	//HANDLE	hMapFileToRead;
	//LPVOID	lpMapAddressToRead;
	LPBYTE				lpBufferToRead;
	
	PCKFILEINDEX		pckFileIndex;

	pckFileIndex.dwUnknown1 = pckFileIndex.dwUnknown2 = 0;
#ifdef PCKV203ZX
	pckFileIndex.dwUnknown3 = pckFileIndex.dwUnknown4 = pckFileIndex.dwUnknown5 = 0;
#endif
	//memset(pckFileIndex.dwAttachedValue, 0, sizeof(pckFileIndex.dwAttachedValue));
//#ifdef PCKV202
//	memset(&pckFileIndex.dwUnknown1, 0, sizeof(DWORD) * 5);
//#elif defined PCKV203
//	memset(&pckFileIndex.dwUnknown1, 0, sizeof(DWORD) * 6);
//#endif

	//��ʼ
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;
	LPFILES_TO_COMPRESS			lpfirstFile = pThis->m_firstFile;
	//LPPCKFILEINDEX				lpPckFileIndex = mt_lpPckFileIndex;

	//char	*lpFilenamePtr = pckFileIndex.szFilename + MAX_PATH_PCK - mt_nLen;
	//memset(lpFilenamePtr, 0, mt_nLen);
	memset(pckFileIndex.szFilename, 0, MAX_PATH_PCK);

	//patch 140424
	CMapViewFileRead	*lpFileRead = new CMapViewFileRead();


	while(NULL != lpfirstFile->next)
	{
		EnterCriticalSection(&g_dwCompressedflag);
		if(0 == lpfirstFile->dwCompressedflag)
		{
			lpfirstFile->dwCompressedflag = 1;

			LeaveCriticalSection(&g_dwCompressedflag); 

			if(PCK_BEGINCOMPRESS_SIZE < lpfirstFile->dwFileSize)
			{
				pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
				pckFileIndex.dwFileCipherTextSize = compressBound(lpfirstFile->dwFileSize);//lpfirstFile->dwFileSize * 1.001 + 12;
				lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize;
			}else{
				pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
			}

			//�ж�ʹ�õ��ڴ��Ƿ񳬹����ֵ
			EnterCriticalSection(&g_mt_lpMaxMemory);
			if( (*mt_lpMaxMemory) >= mt_MaxMemory)
			{
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
			}else
				LeaveCriticalSection(&g_mt_lpMaxMemory);
			
			{
				EnterCriticalSection(&g_mt_lpMaxMemory);
				(*mt_lpMaxMemory) += lpfirstFile->dwFileSize;
				LeaveCriticalSection(&g_mt_lpMaxMemory);
			}

			memcpy(mystrcpy(pckFileIndex.szFilename, mt_szCurrentNodeString), lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy - mt_nCurrentNodeStringLen);

			if(!*(mt_lpbThreadRunning))
			{
				//int i = 1;
				break;
			}

			//����ļ���СΪ0�����������ļ�����
			if(0 != pckFileIndex.dwFileClearTextSize)
			{
				//CMapViewFileRead	*lpFileRead = new CMapViewFileRead();
				////////////////////////////��Դ�ļ�/////////////////////////////////
				//��Ҫ����ѹ�����ļ�
				if(!lpFileRead->Open(lpfirstFile->szFilename))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();

					break;
				}

				//����һ���ļ�ӳ��
				if(!lpFileRead->Mapping(szFileMappingNameSpace))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();
					break;
				}


				//����һ���ļ�ӳ�����ͼ������Ϊsource
				if(NULL == (lpBufferToRead = lpFileRead->View(0, 0)))
				{
					*(mt_lpbThreadRunning) = FALSE;
					//delete lpFileRead;
					lpFileRead->clear();
					break;
				}

				////////////////////////////��Դ�ļ�/////////////////////////////////

				bufCompressData = (BYTE*) malloc (pckFileIndex.dwFileCipherTextSize);
			
				if(PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileClearTextSize)
				{
					compress2(bufCompressData, &pckFileIndex.dwFileCipherTextSize, 
									lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
				}else{
					memcpy(bufCompressData, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
					//pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize;
				}

				//delete lpFileRead;
				lpFileRead->clear();
			}//�ļ���Ϊ0ʱ�Ĵ���


			EnterCriticalSection(&g_cs);

			ptrCurrentIndex = mt_pckCompressedDataPtrArrayPtr++;

			lpPckIndexTablePtr = mt_lpPckIndexTablePtr;
			mt_lpPckIndexTablePtr++;

			//����������ʾ���ļ�����
			(*mt_lpdwCount)++;

			if(NULL != lpfirstFile->samePtr)
			{
				if(pckFileIndex.dwFileCipherTextSize > lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize)
				{

					lpfirstFile->samePtr->cFileIndex.dwAddressOffset = mt_dwAddress;
					mt_dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ

				}else{
					lpPckIndexTablePtr->dwAddressOfDuplicateOldDataArea = lpfirstFile->samePtr->cFileIndex.dwAddressOffset;
				}

				lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize;
				lpfirstFile->samePtr->cFileIndex.dwFileClearTextSize = pckFileIndex.dwFileClearTextSize;

				lpPckIndexTablePtr->bInvalid = TRUE;
				

			}else{

				//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
				pckFileIndex.dwAddressOffset = mt_dwAddress;		//���ļ���ѹ��������ʼ��ַ
				mt_dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ
			}

			LeaveCriticalSection(&g_cs);


			//test
			//lpPckIndexTablePtr->dwAddress = pckFileIndex.dwAddressOffset;//////test
			lpPckIndexTablePtr->dwCompressedFilesize = pckFileIndex.dwFileCipherTextSize;
			lpPckIndexTablePtr->dwMallocSize = lpfirstFile->dwFileSize;

			if(NULL == lpfirstFile->samePtr)
			{

				lpPckIndexTablePtr->dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
				compress2(lpPckIndexTablePtr->buffer, &lpPckIndexTablePtr->dwIndexDataLength, 
								(BYTE*)&pckFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
				//����ȡ��
				//lpPckIndexTablePtr->dwIndexValueHead = lpPckIndexTablePtr->dwIndexDataLength ^ INDEXTABLE_OR_VALUE_HEAD;
				//lpPckIndexTablePtr->dwIndexValueTail = lpPckIndexTablePtr->dwIndexDataLength ^ INDEXTABLE_OR_VALUE_TAIL;
				lpPckIndexTablePtr->dwIndexValueHead = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
				lpPckIndexTablePtr->dwIndexValueTail = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;

			}

			EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//int		ilen = sizeof(BYTE*);
			//BYTE	**tes1t = mt_pckCompressedDataPtrArrayPtr - 1;
			*(ptrCurrentIndex) = bufCompressData;
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//SetEvent(mt_evtToWrite);//��ΪSleep(10);
			
		}else{

			LeaveCriticalSection(&g_dwCompressedflag);
		}
		//���ܵ��ļ�����������
		//lpPckIndexTablePtr++;
		//lpPckFileIndex++;
		//lppckCompressedDataPtrArray++;

		//��һ���ļ��б�
		lpfirstFile = lpfirstFile->next;
	}

	delete lpFileRead;

	EnterCriticalSection(&g_mt_threadID);

	mt_threadID--;
	if(0 == mt_threadID)
		SetEvent(mt_evtAllCompressFinish);

	LeaveCriticalSection(&g_mt_threadID); 
	

	_endthread();
}


VOID CPckClass::WriteThreadAdd(VOID* pParam)
{
	CPckClass *pThis = (CPckClass*)pParam;

	BYTE**		bufferPtrToWrite = mt_pckCompressedDataPtrArray;
	PCKADDR		dwTotalFileSizeAfterCompress = mt_CompressTotalFileSize;

	//HANDLE	hFileToWrite = mt_hFileToWrite;
	//HANDLE	hMapFileToWrite = mt_hMapFileToWrite;
	//LPVOID	lpMapAddressToWrite;
	LPBYTE		lpBufferToWrite;


	LPFILES_TO_COMPRESS			lpfirstFile = pThis->m_firstFile;
	LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;

	PCKADDR						dwAddress = mt_dwAddressName;
	DWORD						nWrite = 0;


	while(1)
	{
		
		EnterCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
		if(NULL != (*bufferPtrToWrite))
		{
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);
			//�˴�����Ӧ����WriteThread�߳���ʵ��
			//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
			//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
			//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
			//1mb=0x100000
			//64mb=0x4000000
			if((dwAddress + lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) > dwTotalFileSizeAfterCompress)
			{
				mt_lpFileWrite->UnMaping();
			
				dwTotalFileSizeAfterCompress += 
					((lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) > PCK_STEP_ADD_SIZE ? (lpPckIndexTablePtr->dwCompressedFilesize + PCK_SPACE_DETECT_SIZE) : PCK_STEP_ADD_SIZE);


				if(!mt_lpFileWrite->Mapping(pThis->m_szMapNameWrite, dwTotalFileSizeAfterCompress))
				{
					{
						EnterCriticalSection(&g_cs);
						*(mt_lpbThreadRunning) = FALSE;
						LeaveCriticalSection(&g_cs);
					}

					WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
					mt_dwFileCount = nWrite;
					mt_dwAddress = dwAddress;

					//�ͷ�
					while(0 != *bufferPtrToWrite)
					{
						free(*bufferPtrToWrite);
						bufferPtrToWrite++;
					}

					break;

				}

			}

			//����lpPckFileIndex->dwAddressOffset
			//�ļ�ӳ���ַ������64k(0x10000)������
			//��� �ļ��õĵ�ַ������ʱ��
			PCKADDR	dwAddressTemp;
			if(0 != lpPckIndexTablePtr->dwAddressOfDuplicateOldDataArea/* && lpPckIndexTablePtr->bInvalid*/)
			{
				//��bInvalidΪ������д����������ʱ������dwIndexDataLength����ָʾ��ʱӦ�ð���������д����ʱ��dwAddress��
				dwAddressTemp = lpPckIndexTablePtr->dwAddressOfDuplicateOldDataArea;
			}else{
				dwAddressTemp = dwAddress;
			}

			if(0 != lpPckIndexTablePtr->dwCompressedFilesize)
			{
				//DWORD	dwMapViewBlockHigh, dwMapViewBlockLow;

				//dwMapViewBlockHigh = dwAddressTemp & 0xffff0000;
				//dwMapViewBlockLow = dwAddressTemp & 0xffff;

				
				if(NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddressTemp, lpPckIndexTablePtr->dwCompressedFilesize)))
				{
					mt_lpFileWrite->UnMaping();
					{
						EnterCriticalSection(&g_cs);
						*(mt_lpbThreadRunning) = FALSE;
						LeaveCriticalSection(&g_cs);
					}

					WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
					mt_dwFileCount = nWrite;
					mt_dwAddress = dwAddress;

					//�ͷ�
					while(0 != *bufferPtrToWrite)
					{
						free(*bufferPtrToWrite);
						bufferPtrToWrite++;
					}
					break;
				}
				//����ַ�Ƿ���ȷ
				//if(lpPckIndexTablePtr->dwAddress != dwAddress)OutputDebugStringA("lpPckIndexTablePtr->dwAddress != dwAddress!!!\r\n");

				memcpy(lpBufferToWrite, *bufferPtrToWrite, lpPckIndexTablePtr->dwCompressedFilesize);
				if(0 == lpPckIndexTablePtr->dwAddressOfDuplicateOldDataArea/* && lpPckIndexTablePtr->bInvalid*/)
				{
					dwAddress += lpPckIndexTablePtr->dwCompressedFilesize;
				}

				free(*bufferPtrToWrite);
				mt_lpFileWrite->UnmapView();
			}

			nWrite++;
			
			{
				EnterCriticalSection(&g_mt_lpMaxMemory);
				(*mt_lpMaxMemory) -= lpPckIndexTablePtr->dwMallocSize;

				if((*mt_lpMaxMemory) < mt_MaxMemory)
				{
					LeaveCriticalSection(&g_mt_lpMaxMemory);
					
					{
						EnterCriticalSection(&g_mt_nMallocBlocked);
						int nMallocBlocked = mt_nMallocBlocked;
						LeaveCriticalSection(&g_mt_nMallocBlocked);
						for(int i=0;i<nMallocBlocked;i++)
							SetEvent(mt_evtMaxMemory);

					}


				}else{
					LeaveCriticalSection(&g_mt_lpMaxMemory);
				}
			}		

			if(mt_dwFileCount == nWrite)
			{
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);
				mt_dwAddress = dwAddress;
				
				break;
			}
			

			bufferPtrToWrite++;
			lpPckIndexTablePtr++;

		}else{
			
			LeaveCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

			EnterCriticalSection(&g_cs);
			if(!(*mt_lpbThreadRunning))
			{
				LeaveCriticalSection(&g_cs);
				//ȡ��
				WaitForSingleObject(mt_evtAllCompressFinish, INFINITE);

				mt_dwFileCount = nWrite;
				mt_dwAddress = dwAddress;

				break;
			}else{
				LeaveCriticalSection(&g_cs);
			}

			::Sleep(10);

		}

	}

	//mt_hMapFileToWrite = hMapFileToWrite;
	SetEvent(mt_evtAllWriteFinish);
	
	_endthread();
}



VOID CPckClass::PreProcess(int threadnum)
{

	if(1 != threadnum)
	{
		mt_lpbThreadRunning = &lpPckParams->cVarParams.bThreadRunning;//&bThreadRunning;
		mt_lpdwCount = &lpPckParams->cVarParams.dwUIProgress;//dwCount;
		mt_MaxMemory = lpPckParams->dwMTMaxMemory;//nMaxMemory;
		mt_lpMaxMemory = &lpPckParams->cVarParams.dwMTMemoryUsed;//nMaxMemory;
		//*mt_lpMaxMemory = 0;
		mt_nMallocBlocked = 0;
		mt_level = lpPckParams->dwCompressLevel;//level;
	}

}

BOOL CPckClass::FillPckHeaderAndInitArray(PCK_ALL_INFOS &PckAllInfo, int threadnum, DWORD dwFileCount)
{

	//����ͷ
	PckAllInfo.PckHead.dwHeadCheckHead = m_lpThisPckKey->HeadVerifyKey1;
#if defined PCKV202 || defined PCKV203ZX
	PckAllInfo.PckHead.dwHeadCheckTail = m_lpThisPckKey->HeadVerifyKey2;
#endif
	//pckHead.dwPckSize = |PCK�ļ���С|;

	ZeroMemory(&PckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR));

	PckAllInfo.PckIndexAddr.dwIndexTableCheckHead = m_lpThisPckKey->TailVerifyKey1;
	PckAllInfo.PckIndexAddr.dwIndexTableCheckTail = m_lpThisPckKey->TailVerifyKey2;
#ifdef PCKV203ZX
	PckAllInfo.PckIndexAddr.dwUnknown1 = 0xffffffff;
	PckAllInfo.PckIndexAddr.dwUnknown2 = 0;
#endif
	//pckIndexAddr.dwIndexValue = |�ļ���ѹ�����ݿ�ʼ| ^ FILEINDEX_ADDR_CONST;

#if defined PCKV202 || defined PCKV203ZX
	PckAllInfo.PckTail.dwVersion = PckAllInfo.PckIndexAddr.dwVersion = m_lpThisPckKey->Version;
#elif defined PCKV203
	PckAllInfo.PckTail.dwVersion = m_lpThisPckKey->Version;
#endif
	//pckTail.dwFileCount = dwFileCount;

	//����ռ�,�ļ���ѹ������ ����
	if(NULL == (mt_lpPckIndexTable = new PCKINDEXTABLE_COMPRESS[dwFileCount]))
	{
		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	memset(mt_lpPckIndexTable, 0, sizeof(PCKINDEXTABLE_COMPRESS) * dwFileCount);

	//////////////MT MODE START//////////////
	if(1 != threadnum)
	{
		if(NULL == (mt_pckCompressedDataPtrArray = (BYTE**) malloc (sizeof(BYTE*) * (dwFileCount  + 1))))
		{
			PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
		memset(mt_pckCompressedDataPtrArray, 0, sizeof(BYTE*) * (dwFileCount + 1));
	}
	//////////////MT MODE END//////////////

	return TRUE;
}

void CPckClass::MultiThreadInitialize(VOID CompressThread(VOID*), VOID WriteThread(VOID*), int threadnum)
{
		//mt_evtToWrite = CreateEventA(NULL, FALSE, FALSE, "WritePckData");
		mt_evtAllWriteFinish = CreateEventA(NULL, FALSE, FALSE, m_szEventAllWriteFinish);
		mt_evtAllCompressFinish =  CreateEventA(NULL, TRUE, FALSE, m_szEventAllCompressFinish);
		mt_evtMaxMemory = CreateEventA(NULL, FALSE, FALSE, m_szEventMaxMemory);
		//mt_evtAbort = CreateEventA(NULL, FALSE, FALSE, "PckCompressAbort");
		mt_lpPckIndexTablePtr = mt_lpPckIndexTable;
		mt_pckCompressedDataPtrArrayPtr = mt_pckCompressedDataPtrArray;

		mt_threadID = 0;

		InitializeCriticalSection(&g_cs);
		InitializeCriticalSection(&g_mt_threadID);
		InitializeCriticalSection(&g_mt_nMallocBlocked);
		InitializeCriticalSection(&g_mt_lpMaxMemory);
		InitializeCriticalSection(&g_dwCompressedflag);
		InitializeCriticalSection(&g_mt_pckCompressedDataPtrArrayPtr);

		for(int i=0;i<threadnum;i++)
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
		//CloseHandle(mt_evtToWrite);
		CloseHandle(mt_evtAllCompressFinish);
		CloseHandle(mt_evtAllWriteFinish);
		
		if(!lpPckParams->cVarParams.bThreadRunning)
		{
			//lstrcpy(m_lastErrorString, TEXT_USERCANCLE);
			PrintLogW(TEXT_USERCANCLE);
		}

}

_inline BOOL __fastcall CPckClass::WritePckIndex(LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr, PCKADDR &dwAddress)
{
	//����lpPckFileIndex->dwAddressOffset
	//�ļ�ӳ���ַ������64k(0x10000)������
	//DWORD	dwMapViewBlockHigh, dwMapViewBlockLow, ;

	//dwMapViewBlockHigh = mt_dwAddress & 0xffff0000;
	//dwMapViewBlockLow = mt_dwAddress & 0xffff;
	LPBYTE	lpBufferToWrite;
	DWORD	dwNumberOfBytesToMap = lpPckIndexTablePtr->dwIndexDataLength + 8;
	
	if(NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddress, dwNumberOfBytesToMap)))
	{
		//delete mt_lpFileWrite;
		PrintLogE(TEXT_WRITE_PCK_INDEX, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	memcpy(lpBufferToWrite, lpPckIndexTablePtr, dwNumberOfBytesToMap);
	mt_lpFileWrite->UnmapView();

	dwAddress += dwNumberOfBytesToMap;
	//lpPckIndexTablePtr++;

	//����������ʾ���ļ�����
	lpPckParams->cVarParams.dwUIProgress++;

	return TRUE;
}


BOOL CPckClass::CreatePckFileMT(LPTSTR szPckFile, LPTSTR szPath)
{
	char		szLogString[LOG_BUFFER];

	sprintf(szLogString, TEXT_LOG_CREATE, szPckFile);
	PrintLogI(szLogString);

	DWORD		dwFileCount = 0/*, dwOldPckFileCount*/;					//�ļ�����, ԭpck�ļ��е��ļ���
	QWORD		qwTotalFileSize = 0, qwTotalFileSizeTemp;			//δѹ��ʱ�����ļ���С
	size_t		nLen;

	DWORD IndexCompressedFilenameDataLengthCryptKey1 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD IndexCompressedFilenameDataLengthCryptKey2 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2;
	
	char		szPathMbsc[MAX_PATH];

	int level = lpPckParams->dwCompressLevel;
	int threadnum = lpPckParams->dwMTThread;

	//LOG
	sprintf(szLogString, TEXT_LOG_LEVEL_THREAD, level, threadnum);
	PrintLogI(szLogString);

	PreProcess(threadnum);

	//��ʼ�����ļ�
	LPFILES_TO_COMPRESS		lpfirstFile;


	mt_dwAddress = PCK_DATA_START_AT;

	nLen = lstrlen(szPath) - 1;
	if('\\' == *(szPath + nLen))*(szPath + nLen) = 0;
	BOOL	IsPatition = lstrlen(szPath) == 2 ? TRUE : FALSE;

	nLen = WideCharToMultiByte(CP_ACP, 0, szPath, -1, szPathMbsc, MAX_PATH, "_", 0);

	if(NULL == m_firstFile)m_firstFile = AllocateFileinfo();
	if(NULL == m_firstFile)
	{
		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	lpfirstFile = m_firstFile;

	EnumFile(szPathMbsc, IsPatition, dwFileCount, lpfirstFile, qwTotalFileSize, nLen);
	if(0 == dwFileCount)return TRUE;


	//�ļ���д�봰�����б�������ʾ����
	mt_dwFileCount = lpPckParams->cVarParams.dwUIProgressUpper = dwFileCount;

	//��������Ҫ���ռ�qwTotalFileSize
	qwTotalFileSizeTemp = qwTotalFileSize * 0.6;
#if defined PCKV202 || defined PCKV203ZX
	if(0 != (qwTotalFileSizeTemp >> 32))
#elif defined PCKV203
	if(0 != (qwTotalFileSizeTemp >> 33))
#endif
	{
		PrintLogE(TEXT_COMPFILE_TOOBIG, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	mt_CompressTotalFileSize = qwTotalFileSizeTemp;
	if(PCK_SPACE_DETECT_SIZE >= mt_CompressTotalFileSize)mt_CompressTotalFileSize = PCK_STEP_ADD_SIZE;

	PCK_ALL_INFOS	pckAllInfo;
	//
	////����ͷ
	////����ռ�,�ļ���ѹ������ ����
	if(!FillPckHeaderAndInitArray(pckAllInfo, threadnum, dwFileCount))
		return FALSE;
	
	//��ʼѹ��
	//���ļ�
	LPBYTE				lpBufferToWrite, lpBufferToRead;
	

	//�����Ǵ���һ���ļ�����������ѹ������ļ�
	mt_lpFileWrite = new CMapViewFileWrite();

	if(!OpenPckAndMappingWrite(mt_lpFileWrite, szPckFile, CREATE_ALWAYS, mt_CompressTotalFileSize)){

		delete mt_lpFileWrite;
		return FALSE;

	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	lpPckParams->cVarParams.dwUIProgress = 0;
	PCKADDR	dwAddressName;

	if(1 == threadnum)
	{

		PCKFILEINDEX	pckFileIndex;
		pckFileIndex.dwUnknown1 = pckFileIndex.dwUnknown2 = 0;
#ifdef PCKV203ZX
		pckFileIndex.dwUnknown3 = pckFileIndex.dwUnknown4 = pckFileIndex.dwUnknown5 = 0;
#endif

		//��ʼ��ָ��
		LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;
		lpfirstFile = m_firstFile;
		
		//��ʼ������ֵ 
		
		//char	*lpFilenamePtr = pckFileIndex.szFilename + MAX_PATH_PCK - mt_nLen;
		//memset(lpFilenamePtr, 0, mt_nLen);
		memset(pckFileIndex.szFilename, 0, MAX_PATH_PCK);
		
		//patch 140424
		CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

		while(NULL != lpfirstFile->next)
		{
			//pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
			memcpy(pckFileIndex.szFilename, lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy);

			if(PCK_BEGINCOMPRESS_SIZE < lpfirstFile->dwFileSize)
			{
				pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
				pckFileIndex.dwFileCipherTextSize = compressBound(lpfirstFile->dwFileSize);//lpfirstFile->dwFileSize * 1.001 + 12;
				lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize;
			}else{
				pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
			}
	
			if(!lpPckParams->cVarParams.bThreadRunning)
			{
				PrintLogW(TEXT_USERCANCLE);
				//Ŀǰ��ѹ���˶����ļ���������д��dwFileCount��д�ļ����б���ļ�ͷ��β������ļ�����
				mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
				break;
			}
	
			//����ļ���СΪ0�����������ļ�����
			if(0 != lpfirstFile->dwFileSize)
			{
				//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
				//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
				//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
				//1mb=0x100000
				//64mb=0x4000000
				if((mt_dwAddress + lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) > mt_CompressTotalFileSize)
				{
					mt_lpFileWrite->UnMaping();
				
					mt_CompressTotalFileSize += 
						((lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) > PCK_STEP_ADD_SIZE ? (lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) : PCK_STEP_ADD_SIZE);
	
					
					if(!mt_lpFileWrite->Mapping(m_szMapNameWrite, mt_CompressTotalFileSize))
					{
						lpPckParams->cVarParams.bThreadRunning = FALSE;
						mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
						break;
					}
	
				}
	
				if(NULL == (lpBufferToWrite = mt_lpFileWrite->View(mt_dwAddress, lpfirstFile->dwFileSize)))
				{
					//CloseHandle(mt_hMapFileToWrite);
					mt_lpFileWrite->UnMaping();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
	
				////////////////////////////��Դ�ļ�/////////////////////////////////
				//CMapViewFileRead	*lpFileRead = new CMapViewFileRead();
				////��Ҫ����ѹ�����ļ�
				if(!lpFileRead->Open(lpfirstFile->szFilename))
				{
					PrintLogE(TEXT_OPENNAME_FAIL, lpfirstFile->szFilename, __FILE__, __FUNCTION__, __LINE__);

					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
	
				//����һ���ļ�ӳ��
				if(!lpFileRead->Mapping(m_szMapNameRead))
				{
					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
	

				//����һ���ļ�ӳ�����ͼ������Ϊsource
				if(NULL == (lpBufferToRead = lpFileRead->View(0, 0)))
				{
					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
	
			////////////////////////////��Դ�ļ�/////////////////////////////////
			
				if(PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileCipherTextSize)
				{
					//pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize * 1.001 + 12;
	
					compress2(lpBufferToWrite, &pckFileIndex.dwFileCipherTextSize, 
									lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
				}else{
					memcpy(lpBufferToWrite, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
				}

				mt_lpFileWrite->UnmapView();
				//delete lpFileRead;
				lpFileRead->clear();

			}//�ļ���Ϊ0ʱ�Ĵ���
	
			//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
			pckFileIndex.dwAddressOffset = mt_dwAddress;		//���ļ���ѹ��������ʼ��ַ
			mt_dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ
	
			lpPckIndexTablePtr->dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
			compress2(lpPckIndexTablePtr->buffer, &lpPckIndexTablePtr->dwIndexDataLength, 
							(BYTE*)&pckFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
			//����ȡ��
			lpPckIndexTablePtr->dwIndexValueHead = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
			lpPckIndexTablePtr->dwIndexValueTail = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;
	
			//����������ʾ���ļ�����
			lpPckParams->cVarParams.dwUIProgress++;
			//���ܵ��ļ�����������
			lpPckIndexTablePtr++;
			//��һ���ļ��б�
			lpfirstFile = lpfirstFile->next;
		}

		delete lpFileRead;

	}else{
	
		MultiThreadInitialize(CompressThread, WriteThread, threadnum);

	}

	
	//LOG
	PrintLogI(TEXT_LOG_COMPRESSOK);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//д�ļ�����
	LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr = mt_lpPckIndexTable;
	dwAddressName = mt_dwAddress;

	//����������ʾ���ļ����ȣ���ʼ������ʾд��������mt_hFileToWrite
	lpPckParams->cVarParams.dwUIProgress = 0;

	for(DWORD	i = 0;i<mt_dwFileCount;i++)
	{
		WritePckIndex(lpPckIndexTablePtr, mt_dwAddress);
		lpPckIndexTablePtr++;

	}
	
	mt_lpFileWrite->UnMaping();

	AfterProcess(mt_lpFileWrite, pckAllInfo, mt_dwFileCount, dwAddressName, mt_dwAddress);

	delete mt_lpFileWrite;
	DeallocateFileinfo();
	delete [] mt_lpPckIndexTable;

	if(1 != threadnum)
	{
		free(mt_pckCompressedDataPtrArray);
	}

	//LOG
	PrintLogI(TEXT_LOG_WORKING_DONE);

	return TRUE;
}



//����pck��
BOOL CPckClass::UpdatePckFile(LPTSTR szPckFile, TCHAR (*lpszFilePath)[MAX_PATH], int nFileCount, LPPCK_PATH_NODE lpNodeToInsert)
{
	DWORD		dwFileCount = 0, dwOldPckFileCount;					//�ļ�����, ԭpck�ļ��е��ļ���
	QWORD		qwTotalFileSize = 0, qwTotalFileSizeTemp;			//δѹ��ʱ�����ļ���С
	size_t		nLen;

	DWORD		IndexCompressedFilenameDataLengthCryptKey1 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2;

	char		szPathMbsc[MAX_PATH];
	char		szLogString[LOG_BUFFER];

	int level = lpPckParams->dwCompressLevel;
	int threadnum = lpPckParams->dwMTThread;

	PreProcess(threadnum);

	//��ʼ�����ļ�
	LPFILES_TO_COMPRESS			lpfirstFile;//, lpPrevFile = NULL;
	//LPFILES_TO_COMPRESS		m_firstFile;

	//mt_dwAddress = mt_dwAddressName;
	//if(!m_ReadCompleted)wcscpy_s(szPckFile, MAX_PATH, m_Filename);

	TCHAR	(*lpszFilePathPtr)[MAX_PATH] = (TCHAR(*)[MAX_PATH])lpszFilePath;
	
	//lpszFilePathPtr = (TCHAR(*)[MAX_PATH])lpszFilePath;

	DWORD				dwAppendCount = nFileCount;
	LPPCK_PATH_NODE		lpNodeToInsertPtr;// = lpNodeToInsert;


	//���ò���
	if(m_ReadCompleted)
	{
		lstrcpy(szPckFile, m_PckAllInfo.szFilename);

		mt_dwAddress = mt_dwAddressName = m_PckAllInfo.dwAddressName;
		dwOldPckFileCount = m_PckAllInfo.PckTail.dwFileCount;

		lpNodeToInsertPtr = lpNodeToInsert;

		//ȡ�õ�ǰ�ڵ�����·��
		if(!GetCurrentNodeString(mt_szCurrentNodeString, lpNodeToInsert))
		{
			free(lpszFilePath);
			return FALSE;
		}
		
		mt_nCurrentNodeStringLen = strlen(mt_szCurrentNodeString);

		sprintf(szLogString, TEXT_LOG_UPDATE_ADD
							"-"
							TEXT_LOG_LEVEL_THREAD, level, threadnum);
		PrintLogI(szLogString);

	}else{

		mt_dwAddress = mt_dwAddressName = m_PckAllInfo.dwAddressName = PCK_DATA_START_AT;
		dwOldPckFileCount = 0;

		lpNodeToInsertPtr = m_RootNode.child;

		*mt_szCurrentNodeString = 0;

		mt_nCurrentNodeStringLen = 0;

		sprintf(szLogString, TEXT_LOG_UPDATE_NEW
							"-"
							TEXT_LOG_LEVEL_THREAD, level, threadnum);
		PrintLogI(szLogString);

	}

	if(NULL == m_firstFile)m_firstFile = AllocateFileinfo();
	if(NULL == m_firstFile)
	{
		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		free(lpszFilePath);
		return FALSE;
	}

	lpfirstFile = m_firstFile;

	//���ļ�
	CMapViewFileRead *lpcFileRead = new CMapViewFileRead();

	for(DWORD i = 0;i<dwAppendCount;i++)
	{
		WideCharToMultiByte(CP_ACP, 0, *lpszFilePathPtr, -1, szPathMbsc, MAX_PATH, "_", 0);
		nLen = (size_t)(strrchr(szPathMbsc, '\\') - szPathMbsc) + 1;
		
		if(FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(szPathMbsc) ))
		{
			//�ļ���
			EnumFile(szPathMbsc, FALSE, dwFileCount, lpfirstFile, qwTotalFileSize, nLen);
		}else{
			//�ļ�
			//CMapViewFileRead	cFileRead;

			if(!lpcFileRead->Open(szPathMbsc))
			{
				DeallocateFileinfo();
				free(lpszFilePath);
				PrintLogE(TEXT_OPENNAME_FAIL, *lpszFilePathPtr, __FILE__, __FUNCTION__, __LINE__);

				delete lpcFileRead;
				return FALSE;
			}
			
			strcpy(lpfirstFile->szFilename, szPathMbsc);
			//memcpy(lpfirstFile->szFilename, szPathMbsc, MAX_PATH);

			lpfirstFile->lpszFileTitle = lpfirstFile->szFilename + nLen;
			lpfirstFile->nBytesToCopy = MAX_PATH_PCK - nLen;

			qwTotalFileSize += (lpfirstFile->dwFileSize = lpcFileRead->GetFileSize());

			lpfirstFile->next = AllocateFileinfo();
			lpfirstFile = lpfirstFile->next;

			lpcFileRead->clear();

			dwFileCount++;
		}

		lpszFilePathPtr++;
	}

	delete lpcFileRead;

	free(lpszFilePath);

	if(0 == dwFileCount)return TRUE;

	//����˵����
	// mt_dwFileCount	��ӵ��ļ����������ظ�
	// dwAllCount		������ʾ������->lpPckParams->cVarParams.dwUIProgressUpper
	// dwFileCount		�������ʹ�ò�����������ļ�������н�ʹ�ô˲�����ʾ��ӵ��ļ������������ظ�
	// 

	//�ļ���д�봰�����б�������ʾ����
	DWORD dwPrepareToAdd = mt_dwFileCount = lpPckParams->cVarParams.dwUIProgressUpper = dwFileCount;

	//��������Ҫ���ռ�qwTotalFileSize
	qwTotalFileSizeTemp = qwTotalFileSize * 0.6 + mt_dwAddressName;
#if defined PCKV202 || defined PCKV203ZX
	if(0 != (qwTotalFileSizeTemp >> 32))
#elif defined PCKV203
	if(0 != (qwTotalFileSizeTemp >> 33))
#endif
	{
		PrintLogE(TEXT_COMPFILE_TOOBIG, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	mt_CompressTotalFileSize = qwTotalFileSizeTemp;

	if(PCK_SPACE_DETECT_SIZE >= mt_CompressTotalFileSize)mt_CompressTotalFileSize = PCK_STEP_ADD_SIZE;


	//��ԭ��Ŀ¼�е��ļ��Աȣ��Ƿ�������
	//���ԣ����������ǰ�				��������Ҷ�Ϊ�ļ����ļ��У��򸲸�
	//
	//����FindFileNode����-1�˳�������0����ʾֱ����ӣ���0�������ظ���
	//дר�õ�writethread��compressthread,�Ե���
	//��PCKINDEXTABLE_COMPRESS�����addר�����ԣ����ж��Ƿ����ô˽ڵ㣨����ʱ��0ʹ�ã�1��ʹ��
	//���� ʱʹ��2��ѭ��д��ѹ������ 

	//dwFileCount�����ڴ˴�ָ������ӵ��ļ���ȥ���������� 

	//lpPrevFile = NULL;
	if(m_ReadCompleted)
	{
		lpfirstFile = m_firstFile;
		while(NULL != lpfirstFile->next)
		{
			LPPCK_PATH_NODE lpDuplicateNode;
			lpDuplicateNode = FindFileNode(lpNodeToInsertPtr, lpfirstFile->lpszFileTitle);


			if(-1 == (int)lpDuplicateNode)
			{
				DeallocateFileinfo();
				PrintLogE(TEXT_ERROR_DUP_FOLDER_FILE);
				return FALSE;
			}

			if(NULL != lpDuplicateNode)
			{
				lpfirstFile->samePtr = lpDuplicateNode->lpPckIndexTable;
				dwFileCount--;
			}

			lpfirstFile = lpfirstFile->next;

		}
	}

	PCK_ALL_INFOS	pckAllInfo;

	//��־
	sprintf(szLogString, "Ԥ����ļ���=%d:Ԥ���ļ���С=%d, ��ʼ������ҵ...", dwPrepareToAdd, mt_CompressTotalFileSize);
	PrintLogI(szLogString);
	
	//����ͷ
	//����ռ�,�ļ���ѹ������ ���飬 �����ǵ�2����
	if(!FillPckHeaderAndInitArray(pckAllInfo, threadnum, mt_dwFileCount))
		return FALSE;
	
	//��ʼѹ��
	//���ļ�
	LPBYTE		lpBufferToWrite, lpBufferToRead;

	//�����Ǵ���һ���ļ�����������ѹ������ļ�
	mt_lpFileWrite = new CMapViewFileWrite();

	//OPEN_ALWAYS���½��µ�pck(CREATE_ALWAYS)����´��ڵ�pck(OPEN_EXISTING)

	if(!OpenPckAndMappingWrite(mt_lpFileWrite, szPckFile, OPEN_ALWAYS, mt_CompressTotalFileSize)){

		delete mt_lpFileWrite;
		return FALSE;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//mt_dwAddress = 12;
	lpPckParams->cVarParams.dwUIProgress = 0;
	//DWORD	/*dwAddress = 12, */dwAddressName;

	BOOL		bUseCurrent;		//����ʱ�Ƿ�ʹ�õ�ǰ���ݣ����ģʽ����Ч
	PCKADDR		dwAddress;			//���ģʽ�£�����ʱ����ָʾ�������ļ�Զ��������ļ�ʱ��ѹ�����ݵĴ�ŵ�ַ
	DWORD		dwOverWriteModeMaxLength;	//����ʱ���������Ǳ������ļ�ʱʹ�õ����ViewMap��С����Ȼ����ĵ����ݻ�Ѻ����ļ���������0

	if(1 == threadnum)
	{

		PCKFILEINDEX	pckFileIndex;
		pckFileIndex.dwUnknown1 = pckFileIndex.dwUnknown2 = 0;
#ifdef PCKV203ZX
		pckFileIndex.dwUnknown3 = pckFileIndex.dwUnknown4 = pckFileIndex.dwUnknown5 = 0;
#endif
		//��ʼ��ָ��
		LPPCKINDEXTABLE_COMPRESS	lpPckIndexTablePtr = mt_lpPckIndexTable;
		lpfirstFile = m_firstFile;
		
		//��ʼ������ֵ 
		
		//char	*lpFilenamePtr = pckFileIndex.szFilename + MAX_PATH_PCK - mt_nLen;
		//memset(lpFilenamePtr, 0, mt_nLen);
		char	*lpFilenamePtr = pckFileIndex.szFilename;
		memset(lpFilenamePtr, 0, MAX_PATH_PCK);

		//patch 140424
		CMapViewFileRead	*lpFileRead = new CMapViewFileRead();
		
		while(NULL != lpfirstFile->next)
		{

			memcpy(mystrcpy(pckFileIndex.szFilename, mt_szCurrentNodeString), lpfirstFile->lpszFileTitle, lpfirstFile->nBytesToCopy - mt_nCurrentNodeStringLen);

			if(PCK_BEGINCOMPRESS_SIZE < lpfirstFile->dwFileSize)
			{
				pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
				pckFileIndex.dwFileCipherTextSize = compressBound(lpfirstFile->dwFileSize);//lpfirstFile->dwFileSize * 1.001 + 12;
				lpfirstFile->dwFileSize = pckFileIndex.dwFileCipherTextSize;
			}else{
				pckFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileClearTextSize = lpfirstFile->dwFileSize;
			}
	
			if(!lpPckParams->cVarParams.bThreadRunning)
			{
				PrintLogW(TEXT_USERCANCLE);

				//Ŀǰ��ѹ���˶����ļ���������д��dwFileCount��д�ļ����б���ļ�ͷ��β������ļ�����
				mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
				break;
			}
	
			//����ļ���СΪ0�����������ļ�����
			if(0 != lpfirstFile->dwFileSize)
			{

				//�ж�һ��dwAddress��ֵ�᲻�ᳬ��dwTotalFileSizeAfterCompress
				//���������˵���ļ��ռ�����Ĺ�С����������һ��ReCreateFileMapping
				//���ļ���С��ԭ���Ļ���������(lpfirstFile->dwFileSize + 1mb) >= 64mb ? (lpfirstFile->dwFileSize + 1mb) :64mb
				//1mb=0x100000
				//64mb=0x4000000
				if((mt_dwAddress + lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) > mt_CompressTotalFileSize)
				{
					//CloseHandle(mt_hMapFileToWrite);
					mt_lpFileWrite->UnMaping();
				
					mt_CompressTotalFileSize += 
						((lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) > PCK_STEP_ADD_SIZE ? (lpfirstFile->dwFileSize + PCK_SPACE_DETECT_SIZE) : PCK_STEP_ADD_SIZE);
	
					if(!mt_lpFileWrite->Mapping(m_szMapNameWrite, mt_CompressTotalFileSize))
					{
						lpPckParams->cVarParams.bThreadRunning = FALSE;
						mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
						break;
					}
	
				}
				//BOOL		bUseCurrent;
				//DWORD		dwAddress;
				//���ģʽ
				dwOverWriteModeMaxLength = lpfirstFile->dwFileSize;

				if(NULL != lpfirstFile->samePtr)
				{
					//������е��ļ���С����pck�е��ļ�����ѹ����С��ʹ����buffer
					if(dwOverWriteModeMaxLength >= lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize)
					{
						lpfirstFile->samePtr->cFileIndex.dwAddressOffset = mt_dwAddress;
						bUseCurrent = TRUE;
						//dwOverWriteModeMaxLength = lpfirstFile->dwFileSize;

						dwAddress = mt_dwAddress;
					}else{

						dwAddress = lpfirstFile->samePtr->cFileIndex.dwAddressOffset;

						bUseCurrent = FALSE;
						dwOverWriteModeMaxLength = lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize;
					}
				}else{
					dwAddress = mt_dwAddress;
					//dwOverWriteModeMaxLength = lpfirstFile->dwFileSize;
				}

				//����lpPckFileIndex->dwAddressOffset
				//�ļ�ӳ���ַ������64k(0x10000)������
				//DWORD	dwMapViewBlockHigh, dwMapViewBlockLow/*, dwNumberOfBytesToMap*/;
	
				//dwMapViewBlockHigh = dwAddress & 0xffff0000;
				//dwMapViewBlockLow = dwAddress & 0xffff;
	
				
				if(NULL == (lpBufferToWrite = mt_lpFileWrite->View(dwAddress, dwOverWriteModeMaxLength)))
				{
					//CloseHandle(mt_hMapFileToWrite);
					mt_lpFileWrite->UnMaping();
					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
				
				//CMapViewFileRead	*lpFileRead = new CMapViewFileRead();

				if(!lpFileRead->Open(lpfirstFile->szFilename))
				{
					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();


					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}

				if(!lpFileRead->Mapping(m_szMapNameRead))
				{
					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}

				if(NULL == (lpBufferToRead = lpFileRead->View(0, 0)))
				{
					mt_lpFileWrite->UnMaping();
					//delete lpFileRead;
					lpFileRead->clear();

					lpPckParams->cVarParams.bThreadRunning = FALSE;
					mt_dwFileCount = lpPckParams->cVarParams.dwUIProgress;
					break;
				}
	
				////////////////////////////��Դ�ļ�/////////////////////////////////
			
				if(PCK_BEGINCOMPRESS_SIZE < pckFileIndex.dwFileClearTextSize)
				{
					//pckFileIndex.dwFileCipherTextSize = dwOverWriteModeMaxLength/*pckFileIndex.dwFileCipherTextSize * 1.001 + 12*/;
	
					compress2(lpBufferToWrite, &pckFileIndex.dwFileCipherTextSize, 
									lpBufferToRead, pckFileIndex.dwFileClearTextSize, level);
				}else{
					memcpy(lpBufferToWrite, lpBufferToRead, pckFileIndex.dwFileClearTextSize);
				}
	
				mt_lpFileWrite->UnmapView();
				//delete lpFileRead;
				lpFileRead->clear();

			}//�ļ���Ϊ0ʱ�Ĵ���
			else{
				if(NULL != lpfirstFile->samePtr)
				{
					bUseCurrent = FALSE;
				}
			}
	
			//д����ļ���PckFileIndex�ļ�ѹ����Ϣ��ѹ��
			if(NULL == lpfirstFile->samePtr)
			{
				
				pckFileIndex.dwAddressOffset = mt_dwAddress;		//���ļ���ѹ��������ʼ��ַ
				mt_dwAddress += pckFileIndex.dwFileCipherTextSize;	//��һ���ļ���ѹ��������ʼ��ַ
		
				lpPckIndexTablePtr->dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
				compress2(lpPckIndexTablePtr->buffer, &lpPckIndexTablePtr->dwIndexDataLength, 
								(BYTE*)&pckFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
				//����ȡ��
				lpPckIndexTablePtr->dwIndexValueHead = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
				lpPckIndexTablePtr->dwIndexValueTail = lpPckIndexTablePtr->dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;

				//����������ʾ���ļ�����
				lpPckParams->cVarParams.dwUIProgress++;

				//���node


			}else{
				lpPckIndexTablePtr->bInvalid = TRUE;

				lpfirstFile->samePtr->cFileIndex.dwFileCipherTextSize = pckFileIndex.dwFileCipherTextSize;
				lpfirstFile->samePtr->cFileIndex.dwFileClearTextSize = pckFileIndex.dwFileClearTextSize;

				if(bUseCurrent)
				{
					mt_dwAddress += pckFileIndex.dwFileCipherTextSize;
					
				}
			}

			//���ܵ��ļ�����������
			lpPckIndexTablePtr++;
			//��һ���ļ��б�
			lpfirstFile = lpfirstFile->next;
		}

		delete lpFileRead;

	}else{

		MultiThreadInitialize(CompressThreadAdd, WriteThreadAdd, threadnum);

	}
	
	
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//��ӡ�����ò���
	DWORD	dwUseNewDataAreaInDuplicateFile = 0;

	//д�ļ�����
	PCKADDR dwAddressName = mt_dwAddress;

	//����������ʾ���ļ����ȣ���ʼ������ʾд��������mt_hFileToWrite
	lpPckParams->cVarParams.dwUIProgress = 0;
	//dwAllCount = mt_dwFileCount + dwOldPckFileCount;	//������ļ���������������������Ӧ��������Ĺ�ʽ
	lpPckParams->cVarParams.dwUIProgressUpper = dwFileCount + dwOldPckFileCount;


	//дԭ�����ļ�
	LPPCKINDEXTABLE	lpPckIndexTableSrc = m_lpPckIndexTable;

	//__try{

		for(DWORD	i = 0;i<dwOldPckFileCount;i++)
		{

			PCKINDEXTABLE_COMPRESS	pckIndexTableTemp;


			pckIndexTableTemp.dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
			compress2(pckIndexTableTemp.buffer, &pckIndexTableTemp.dwIndexDataLength, 
							(BYTE*)&lpPckIndexTableSrc->cFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
			//����ȡ��
			pckIndexTableTemp.dwIndexValueHead = pckIndexTableTemp.dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
			pckIndexTableTemp.dwIndexValueTail = pckIndexTableTemp.dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;

			WritePckIndex(&pckIndexTableTemp, mt_dwAddress);

			//�Ƿ�ʹ��������������ֻ������ʱ���п���
			if(lpPckIndexTableSrc->cFileIndex.dwAddressOffset >= m_PckAllInfo.dwAddressName)
				dwUseNewDataAreaInDuplicateFile++;

			lpPckIndexTableSrc++;

		}

		//д��ӵ��ļ�
		LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr = mt_lpPckIndexTable;

		dwFileCount = mt_dwFileCount;
		for(DWORD	i = 0;i<mt_dwFileCount;i++)
		{
			////����lpPckFileIndex->dwAddressOffset
			////�ļ�ӳ���ַ������64k(0x10000)������

			if(!lpPckIndexTablePtr->bInvalid)
			{
				WritePckIndex(lpPckIndexTablePtr, mt_dwAddress);

			}else{
				dwFileCount--;
			}
			lpPckIndexTablePtr++;

		}
	//}__except{

	//}

	mt_lpFileWrite->UnMaping();

	//pckTail.dwFileCount = dwFileCount + dwOldPckFileCount;//mt_dwFileCount��ʵ��д�������ظ����Ѿ��������ȥ��
	AfterProcess(mt_lpFileWrite, pckAllInfo, dwFileCount + dwOldPckFileCount, dwAddressName, mt_dwAddress);

	delete mt_lpFileWrite;
	DeallocateFileinfo();
	delete [] mt_lpPckIndexTable;

	if(1 != threadnum)
	{
		free(mt_pckCompressedDataPtrArray);
	}

	//���������´�һ�Σ�����ֱ�Ӵ򿪣��ɽ����߳����

	lpPckParams->cVarParams.dwOldFileCount = dwOldPckFileCount;
	lpPckParams->cVarParams.dwPrepareToAddFileCount = dwPrepareToAdd;
	lpPckParams->cVarParams.dwChangedFileCount = mt_dwFileCount;
	lpPckParams->cVarParams.dwDuplicateFileCount = mt_dwFileCount - dwFileCount;
	lpPckParams->cVarParams.dwUseNewDataAreaInDuplicateFileSize = dwUseNewDataAreaInDuplicateFile;
	lpPckParams->cVarParams.dwFinalFileCount = pckAllInfo.PckTail.dwFileCount;


	PrintLogI(TEXT_LOG_WORKING_DONE);

	//sprintf(szLogString, "�������", dwPrepareToAdd, mt_CompressTotalFileSize);
	//PrintLogI(szLogString);

	return TRUE;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//�������ļ�
BOOL CPckClass::RenameFilename()
{

	PrintLogI(TEXT_LOG_RENAME);

	//LPBYTE		lpBufferToWrite;
	int			level = lpPckParams->dwCompressLevel;
	DWORD		IndexCompressedFilenameDataLengthCryptKey1 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2 = m_lpThisPckKey->IndexCompressedFilenameDataLengthCryptKey2;

	//�����Ǵ���һ���ļ�����������ѹ������ļ�
	mt_lpFileWrite = new CMapViewFileWrite();
	if (!mt_lpFileWrite->OpenPck(m_PckAllInfo.szFilename, OPEN_EXISTING))
	{

		PrintLogE(TEXT_OPENWRITENAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);

		delete mt_lpFileWrite;
		return FALSE;
	}

	PCKADDR dwFileSize = mt_lpFileWrite->GetFileSize() + 0x1000000;
	
	if(!mt_lpFileWrite->Mapping(m_szMapNameWrite, dwFileSize))
	{
		PrintLogE(TEXT_CREATEMAPNAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);

		delete mt_lpFileWrite;
		return FALSE;
	}
	//д�ļ�����
	
	PCKADDR dwAddress = m_PckAllInfo.dwAddressName;

	//����������ʾ���ļ����ȣ���ʼ������ʾд��������mt_hFileToWrite
	lpPckParams->cVarParams.dwUIProgress = 0;
	//dwAllCount = mt_dwFileCount + dwOldPckFileCount;	//������ļ���������������������Ӧ��������Ĺ�ʽ
	lpPckParams->cVarParams.dwUIProgressUpper = m_PckAllInfo.PckTail.dwFileCount;


	//дԭ�����ļ�
	LPPCKINDEXTABLE	lpPckIndexTableSrc = m_lpPckIndexTable;

	////д�������Ƿ�ɹ�
	//BOOL isWritePckIndexSuccess = TRUE;

	for(DWORD	i = 0;i<lpPckParams->cVarParams.dwUIProgressUpper;++i)
	{

		PCKINDEXTABLE_COMPRESS	pckIndexTableTemp;

		if(lpPckIndexTableSrc->bSelected)
		{
			--m_PckAllInfo.PckTail.dwFileCount;
			++lpPckIndexTableSrc;

			//����������ʾ���ļ�����
			++lpPckParams->cVarParams.dwUIProgress;
			continue;
		}


		pckIndexTableTemp.dwIndexDataLength = INDEXTABLE_CLEARTEXT_LENGTH;
		compress2(pckIndexTableTemp.buffer, &pckIndexTableTemp.dwIndexDataLength, 
						(BYTE*)&lpPckIndexTableSrc->cFileIndex, INDEXTABLE_CLEARTEXT_LENGTH, level);
		//����ȡ��
		pckIndexTableTemp.dwIndexValueHead = pckIndexTableTemp.dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey1;
		pckIndexTableTemp.dwIndexValueTail = pckIndexTableTemp.dwIndexDataLength ^ IndexCompressedFilenameDataLengthCryptKey2;

		WritePckIndex(&pckIndexTableTemp, dwAddress);

		++lpPckIndexTableSrc;

	}

	mt_lpFileWrite->UnMaping();

	//дpckIndexAddr��272�ֽ�
	mt_lpFileWrite->SetFilePointer(dwAddress, FILE_BEGIN);
	//DWORD	dwBytesWrite;
	//WriteFile(mt_hFileToWrite, &m_PckIndexAddr, sizeof(PCKINDEXADDR), &dwBytesWrite, NULL);
	dwAddress += mt_lpFileWrite->Write(&m_PckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR));

	//дpckTail
	//pckTail.dwFileCount = mt_dwFileCount + dwOldPckFileCount;//mt_dwFileCount��ʵ��д�������ظ����Ѿ��������ȥ��
	//WriteFile(mt_hFileToWrite, &m_PckTail, sizeof(PCKTAIL), &dwBytesWrite, NULL);
	dwAddress += mt_lpFileWrite->Write(&(m_PckAllInfo.PckTail), sizeof(PCKTAIL));
	
	//дpckHead
	m_PckAllInfo.PckHead.dwPckSize = dwAddress;
	mt_lpFileWrite->SetFilePointer(0, FILE_BEGIN);
	mt_lpFileWrite->Write(&m_PckAllInfo.PckHead, sizeof(PCKHEAD));


	//���ｫ�ļ���С��������һ��
	mt_lpFileWrite->SetFilePointer(dwAddress, FILE_BEGIN);
	mt_lpFileWrite->SetEndOfFile();

	delete mt_lpFileWrite;

	PrintLogI(TEXT_LOG_WORKING_DONE);

	return TRUE;
}

