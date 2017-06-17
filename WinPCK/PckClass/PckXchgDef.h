//////////////////////////////////////////////////////////////////////
// PckXchgDef.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.22
//////////////////////////////////////////////////////////////////////

#pragma once

class CPckVersion;
class CPckControlCenter;

typedef struct _PCK_VARIETY_PARAMS {

	DWORD		dwOldFileCount;
	DWORD		dwPrepareToAddFileCount;
	DWORD		dwChangedFileCount;
	DWORD		dwDuplicateFileCount;
	DWORD		dwFinalFileCount;

	DWORD		dwUseNewDataAreaInDuplicateFileSize;
	DWORD		dwDataAreaSize;
	DWORD		dwRedundancyDataSize;

	LPCSTR		lpszAdditionalInfo;

	DWORD		dwUIProgress;
	DWORD		dwUIProgressUpper;

	DWORD		dwMTMemoryUsed;

	BOOL		bThreadRunning;

}PCK_VARIETY_PARAMS;


typedef struct _PCK_RUNTIME_PARAMS {

	//DWORD		dwOldFileCount;
	//DWORD		dwPrepareToAddFileCount;
	//DWORD		dwChangedFileCount;
	//DWORD		dwFinalFileCount;

	//DWORD		dwUseNewDataAreaInDuplicateFileSize;
	//DWORD		dwDataAreaSize;
	//DWORD		dwRedundancyDataSize;

	//LPCSTR		lpszAdditionalInfo;
	//DWORD		dwUIProgress;
	//DWORD		dwUIProgressUpper;


	//DWORD		dwMTMemoryUsed;
	
	PCK_VARIETY_PARAMS	cVarParams;

	DWORD		dwMTMaxMemory;

	DWORD		dwMTThread;
	DWORD		dwMTMaxThread;		//��ǰCPU��1.5��

	DWORD		dwCompressLevel;



	CPckVersion	*lpPckVersion;
	CPckControlCenter	*lpPckControlCenter;

	//int			iSelectedItem;
	int			iListHotItem;

}PCK_RUNTIME_PARAMS, *LPPCK_RUNTIME_PARAMS;


