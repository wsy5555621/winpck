//////////////////////////////////////////////////////////////////////
// PckClass.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#include "MapViewFile.h"
#include "PckHeader.h"
#include <stdio.h>
#ifdef _DEBUG
#include <assert.h>
#endif

#if !defined(_PCKCLASS_H_)
#define _PCKCLASS_H_

#ifdef _DEBUG
int logOutput(const char *file, const char *text);
char *formatString(const char *format, ...);
#endif

class CPckClass  
{
//����
public:
	void	CPckClassInit();
	//CPckClass();
	CPckClass(LPPCK_RUNTIME_PARAMS inout);
	virtual ~CPckClass();

	virtual BOOL	Init(LPCTSTR szFile);

	virtual CONST	LPPCKINDEXTABLE		GetPckIndexTable();
	virtual CONST	LPPCK_PATH_NODE		GetPckPathNode();
	//CONST	LPPCKHEAD			GetPckHead();

	//���ð汾
	const	PCK_KEYS*	GetPckVersion();
	void	SetPckVersion(int verID);
	LPCTSTR	GetSaveDlgFilterString();

	//�ļ���С
	QWORD	GetPckSize();

	//��ȡ�ļ���
	DWORD	GetPckFileCount();

	//��������С
	QWORD	GetPckDataAreaSize();

	//�������������ݴ�С
	QWORD	GetPckRedundancyDataSize();
	
	//��ѹ�ļ�
	BOOL	ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount);
	BOOL	ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount);
	
	//���ø�����Ϣ
	char*	GetAdditionalInfo();
	BOOL	SetAdditionalInfo();

	//�½�pck�ļ�
	virtual BOOL	CreatePckFile(LPTSTR szPckFile, LPTSTR szPath);

	//�ؽ�pck�ļ�
	virtual BOOL	RebuildPckFile(LPTSTR szRebuildPckFile);
	virtual BOOL	RecompressPckFile(LPTSTR szRecompressPckFile);

	//����pck�ļ�
	virtual BOOL	UpdatePckFile(LPTSTR szPckFile, TCHAR (*lpszFilePath)[MAX_PATH], int nFileCount, LPPCK_PATH_NODE lpNodeToInsert);

	//�������ļ�
	virtual BOOL	RenameFilename();

	//ɾ��һ���ڵ�
	virtual VOID	DeleteNode(LPPCK_PATH_NODE lpNode);

	//������һ���ڵ�
	virtual BOOL	RenameNode(LPPCK_PATH_NODE lpNode, char* lpszReplaceString);
protected:
	virtual BOOL	RenameNodeEnum(LPPCK_PATH_NODE lpNode, size_t lenNodeRes, char* lpszReplaceString, size_t lenrs, size_t lenrp);
	virtual BOOL	RenameNode(LPPCK_PATH_NODE lpNode, size_t lenNodeRes, char* lpszReplaceString, size_t lenrs, size_t lenrp);
public:
	virtual VOID	RenameIndex(LPPCK_PATH_NODE lpNode, char* lpszReplaceString);
	virtual VOID	RenameIndex(LPPCKINDEXTABLE lpIndex, char* lpszReplaceString);

	//Ԥ���ļ�
	virtual BOOL	GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpPckFileIndexTable, char *buffer, size_t sizeOfBuffer = 0);

	//��ȡnode·��
	BOOL	GetCurrentNodeString(char*szCurrentNodePathString, LPPCK_PATH_NODE lpNode);

	//��ȡ������Ϣ������д���ļ�����ɵ��ļ��𻵵Ļָ�
	#include "PckClassExceptionRestore.h"

protected:

	//PckClass.cpp
	BOOL	MountPckFile(LPCTSTR	szFile);

	//PckClassFunction.cpp
	char*	UCS2toA(LPCWSTR src, int max_len = -1);
	BOOL	OpenPckAndMappingRead(CMapViewFileRead *lpRead, LPCSTR lpFileName, LPCSTR lpszMapNamespace);
	BOOL	OpenPckAndMappingRead(CMapViewFileRead *lpRead, LPCWSTR lpFileName, LPCSTR lpszMapNamespace);
	BOOL	OpenPckAndMappingWrite(CMapViewFileWrite *lpWrite, LPCTSTR lpFileName, DWORD dwCreationDisposition, QWORD qdwSizeToMap);
	//�ؽ�ʱ���¼����ļ���������ȥ��Ч�ĺ��ļ����ظ���
	DWORD	ReCountFiles();
	BOOL	IsNeedExpandWritingFile(
		CMapViewFileWrite *lpWrite,
		QWORD dwAddress,
		QWORD dwFileSize,
		QWORD &dwCompressTotalFileSize
	);
	DWORD GetCompressBoundSizeByFileSize(LPPCKFILEINDEX	lpPckFileIndex, DWORD dwFileSize);

	//����
	BOOL BeforeSingleOrMultiThreadProcess(LPPCK_ALL_INFOS lpPckAllInfo, CMapViewFileWrite* &lpWrite, LPTSTR szPckFile, DWORD dwCreationDisposition, QWORD qdwSizeToMap, int threadnum);

	void MultiThreadInitialize(VOID CompressThread(VOID*), VOID WriteThread(VOID*), int threadnum);
	BOOL WritePckIndex(CMapViewFileWrite *lpWrite, LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr, QWORD &dwAddress);
	void AfterProcess(CMapViewFileWrite *lpWrite, PCK_ALL_INFOS &PckAllInfo, QWORD &dwAddress, BOOL isRenewAddtional = TRUE);
	BOOL WritePckIndexTable(CMapViewFileWrite *lpWrite, LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr, DWORD &dwFileCount, QWORD &dwAddress);
	void AfterWriteThreadFailProcess(BOOL *lpbThreadRunning, HANDLE hevtAllCompressFinish, DWORD &dwFileCount, DWORD dwFileHasBeenWritten, QWORD &dwAddressFinal, QWORD dwAddress, BYTE **bufferPtrToWrite);
	LPBYTE OpenMappingAndViewAllRead(CMapViewFileRead *lpRead, LPCSTR lpFileName, LPCSTR lpszMapNamespace);
	LPPCKINDEXTABLE_COMPRESS FillAndCompressIndexData(LPPCKINDEXTABLE_COMPRESS lpPckIndexTableComped, LPPCKFILEINDEX lpPckFileIndexToCompress);


	//PckClassFunction.cpp

	BOOL	AllocIndexTableAndInit(LPPCKINDEXTABLE &lpPckIndexTable, DWORD dwFileCount);

	virtual void	BuildDirTree();
	void* AllocNodes(size_t	sizeStuct);
	VOID	DeAllocMultiNodes(LPPCK_PATH_NODE lpThisNode);

	LPFILES_TO_COMPRESS AllocateFileinfo();
	VOID	DeallocateFileinfo();

	BOOL	AddFileToNode(LPPCK_PATH_NODE lpRootNode, LPPCKINDEXTABLE	lpPckIndexNode);
	LPPCK_PATH_NODE	FindFileNode(LPPCK_PATH_NODE lpBaseNode, char* lpszFile);

	//BOOL	EnumNode(LPPCK_PATH_NODE lpNodeToExtract, LPVOID lpvoidFileRead, LPVOID lpvoidFileWrite, LPPCKINDEXTABLE_COMPRESS &lpPckIndexTablePtr, QWORD &dwAddress);
	VOID	EnumFile(LPSTR szFilename, BOOL IsPatition, DWORD &dwFileCount, LPFILES_TO_COMPRESS &pFileinfo, QWORD &qwTotalFileSize, size_t nLen);

	//PckClassExtract.cpp

	BOOL	StartExtract(LPPCK_PATH_NODE lpNodeToExtract, LPVOID lpMapAddress);
	BOOL	DecompressFile(char	*lpszFilename, LPPCKINDEXTABLE lpPckFileIndexTable, LPVOID lpvoidFileRead);


	//PckClassThread.cpp

	/////////���¹������ڶ��߳�pckѹ���Ĳ���
	//���̰߳�
#if PCK_COMPRESS_NEED_ST
	BOOL	CreatePckFileSingleThread(QWORD &dwAddress);
	BOOL	UpdatePckFileSingleThread(QWORD &dwAddress);
	BOOL	RecompressPckFileSingleThread(CMapViewFileRead	*lpFileRead, CMapViewFileWrite *lpFileWrite, DWORD dwFileCount, QWORD &dwAddress, LPPCKINDEXTABLE_COMPRESS	&lpPckIndexTable);
#endif

	static	VOID CompressThreadCreate(VOID* pParam);
	static	VOID WriteThread(VOID* pParam);

	//���ģʽ

	static	VOID CompressThreadAdd(VOID* pParam);

	//��ѹ��ģʽ
	static	VOID CompressThreadRecompress(VOID* pParam);

	//ѹ�����ݶ���
	void	detectMaxAndAddMemory(DWORD dwMallocSize);
	void	freeMaxAndSubtractMemory(DWORD dwMallocSize);

	BOOL	initCompressedDataQueue(int threadnum, DWORD dwFileCount, QWORD dwAddressStartAt);
	BOOL	putCompressedDataQueue(LPBYTE lpBuffer, LPPCKINDEXTABLE_COMPRESS lpPckIndexTable, LPPCKFILEINDEX lpPckFileIndexToCompress);
	BOOL	getCompressedDataQueue(LPBYTE &lpBuffer, LPPCKINDEXTABLE_COMPRESS &lpPckIndexTable);
	void	uninitCompressedDataQueue(int threadnum);


	///�������
	//virtual void test();

	//��ӡ��־
	void PrintLogI(const char *_text);
	void PrintLogW(const char *_text);
	void PrintLogE(const char *_text);
	void PrintLogD(const char *_text);
	void PrintLogN(const char *_text);

	void PrintLogI(const wchar_t *_text);
	void PrintLogW(const wchar_t *_text);
	void PrintLogE(const wchar_t *_text);
	void PrintLogD(const wchar_t *_text);
	void PrintLogN(const wchar_t *_text);

	void PrintLogE(const char *_maintext, const char *_file, const char *_func, const long _line);
	void PrintLogE(const wchar_t *_maintext, const char *_file, const char *_func, const long _line);
	void PrintLogE(const char *_fmt, const char *_maintext, const char *_file, const char *_func, const long _line);
	void PrintLogE(const char *_fmt, const wchar_t *_maintext, const char *_file, const char *_func, const long _line);

	void PrintLog(const char chLevel, const char *_maintext);
	void PrintLog(const char chLevel, const wchar_t *_maintext);
	void PrintLog(const char chLevel, const  char *_fmt, const char *_maintext);
	void PrintLog(const char chLevel, const char *_fmt, const wchar_t *_maintext);

	//
	#include "PckClassVersionDetect.h"

	// �ļ�ͷ��β�Ƚṹ��Ķ�ȡ
	#include "PckClassReader.h"

	//����ѹ���㷨
	#include "PckClassCompress.h"

//����
public:

	//TCHAR	m_lastErrorString[1024];

protected:

	LPPCK_RUNTIME_PARAMS	lpPckParams;

	BOOL					m_ReadCompleted;
	PCK_ALL_INFOS			m_PckAllInfo;
	LPPCKINDEXTABLE			m_lpPckIndexTable;
	PCK_PATH_NODE			m_RootNode;

	LPFILES_TO_COMPRESS		m_firstFile;
	
	char			m_szEventAllWriteFinish[16];
	char			m_szEventAllCompressFinish[16];
	char			m_szEventMaxMemory[16];

	char			m_szMapNameRead[16];
	char			m_szMapNameWrite[16];
};

#endif