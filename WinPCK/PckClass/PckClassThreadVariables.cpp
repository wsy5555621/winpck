//////////////////////////////////////////////////////////////////////
// PckClassThreadVariables.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// 
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.7.5
//////////////////////////////////////////////////////////////////////
/////////���±������ڶ��߳�pckѹ���Ĳ���
#define PCK_MODE_COMPRESS_CREATE	1
#define PCK_MODE_COMPRESS_ADD		2

//�û�ȡ����������ʱ�����浱ǰ�����ѹ�����ļ���
#define SET_PCK_FILE_COUNT_GLOBAL	mt_dwFileCountOfWriteTarget = lpPckParams->cVarParams.dwUIProgress;
#define SET_PCK_FILE_COUNT_AT_FAIL	lpPckParams->cVarParams.bThreadRunning = FALSE;SET_PCK_FILE_COUNT_GLOBAL

#ifdef _DEBUG
char *formatString(const char *format, ...)
{
	const int BUFFER_SIZE = 4097;
	char strbuf[BUFFER_SIZE];
	memset(strbuf, 0, sizeof(strbuf));

	va_list ap;
	va_start(ap, format);
	int result = ::vsnprintf(strbuf, BUFFER_SIZE - 1, format, ap);
	va_end(ap);

	return strbuf;
}

int logOutput(const char *file, const char *text)
{
	char szFile[MAX_PATH];
	if (0 == strncmp("CPckClass::", file, strlen("CPckClass::"))) {
		sprintf_s(szFile, "G:\\pcktest\\%s.log", file + strlen("CPckClass::"));
	}else{
		sprintf_s(szFile, "G:\\pcktest\\%s.log", file);
	}
	

	FILE *pFile = fopen(szFile, "ab");
	if (pFile != NULL)
	{
		fseek(pFile, 0, SEEK_END);
		//string tstr = getLogString() + " - ";
		//int ret = fwrite(tstr.c_str(), 1, tstr.length(), pFile);
		int ret = fwrite(text, 1, strlen(text), pFile);
		fclose(pFile);
		pFile = NULL;
		return ret;
	}
	return -1;
}
#endif

_inline char * __fastcall mystrcpy(char * dest, const char *src)
{
	while ((*dest = *src))
		dest++, src++;
	return dest;
}

CRITICAL_SECTION	g_cs;
CRITICAL_SECTION	g_mt_threadID;
CRITICAL_SECTION	g_mt_nMallocBlocked;
CRITICAL_SECTION	g_mt_lpMaxMemory;
CRITICAL_SECTION	g_dwCompressedflag;
CRITICAL_SECTION	g_mt_pckCompressedDataPtrArrayPtr;

int					mt_threadID;		//�߳�ID

LPPCKINDEXTABLE_COMPRESS	mt_lpPckIndexTable;										//ѹ������ļ�������
BYTE			**	mt_pckCompressedDataPtrArray;									//�ڴ�����ָ�������
																					//HANDLE			mt_evtToWrite;													//�߳��¼�
HANDLE				mt_evtAllWriteFinish;											//�߳��¼�
HANDLE				mt_evtAllCompressFinish;										//�߳��¼�
HANDLE				mt_evtMaxMemory;												//�߳��¼�
																					//HANDLE				mt_hFileToWrite, mt_hMapFileToWrite;						//ȫ��д�ļ��ľ��
CMapViewFileRead	*mt_lpFileRead;													//ȫ�ֶ��ļ��ľ��,Ŀǰ��������ѹ��
CMapViewFileWrite	*mt_lpFileWrite;												//ȫ��д�ļ��ľ��
QWORD				mt_CompressTotalFileSize;										//Ԥ�Ƶ�ѹ���ļ���С
LPDWORD				mt_lpdwCount;													//�ӽ����̴߳��������ļ�����ָ��
DWORD				mt_dwFileCount;													//�ӽ����̴߳����������ļ�����ָ��
//DWORD				mt_dwNoDupFileCount;											//��ѹ��ʱ����Ч�ļ�����
DWORD				mt_dwFileCountOfWriteTarget;									//д�����ݵ�Ŀ������,һ��=mt_dwFileCount�����ʱ=��ѹ��ʱ����Ч�ļ�����

DWORD				mt_dwMaxQueueLength;											//������󳤶�
#ifdef _DEBUG
DWORD				mt_dwCurrentQueuePosPut, mt_dwCurrentQueuePosGet;				//��ǰ����λ��
#endif
DWORD				mt_dwCurrentQueueLength;										//��ǰ���п�ʹ�����ݵĸ���
BYTE			**	mt_pckCompressedDataPtrArrayGet, **mt_pckCompressedDataPtrArrayPut;
LPPCKINDEXTABLE_COMPRESS	mt_lpPckIndexTablePut, mt_lpPckIndexTableGet;

//QWORD				mt_dwAddress;
QWORD				mt_dwAddressQueue;												//ȫ��ѹ�����̵�д�ļ���λ�ã�ֻ��Queue����
QWORD				mt_dwAddressNameQueue;											//������pck�ļ���ѹ���ļ���������ʼλ��

																					//size_t			mt_nLen;														//������Ŀ¼�ĳ��ȣ����ڽ�ȡ�ļ����ŵ�pck��
BOOL				*mt_lpbThreadRunning;											//�߳��Ƿ�������״̬��ֵ����Ϊfalseʱ�߳��˳�
DWORD				*mt_lpMaxMemory;												//ѹ����������ʹ�õ�����ڴ棬����һ�����������õģ����ôӽ����̴߳�������ֵ������
DWORD				mt_MaxMemory;													//ѹ����������ʹ�õ�����ڴ�
int					mt_nMallocBlocked;												//�򻺴����걻��ͣ���߳���
//DWORD				mt_level;														//ѹ���ȼ�


																					//���ʱʹ�ñ���

char				mt_szCurrentNodeString[MAX_PATH_PCK_260];						//�������߳��е�ǰ��ʾ�ģ��ڵ��Ӧ��pck�е��ļ�·��
int					mt_nCurrentNodeStringLen;										//�䳤��

