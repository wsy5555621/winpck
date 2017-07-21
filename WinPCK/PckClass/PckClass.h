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
//#include "Pck.h"
#include "PckHeader.h"
#include <stdio.h>
#include "PckVersion.h"



#if !defined(_PCKCLASS_H_)
#define _PCKCLASS_H_

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
	CONST	LPPCKHEAD			GetPckHead();

	//���ð汾
	void	SetPckVersion(int verID);

	//��ȡ�ļ���
	DWORD	GetPckFileCount();

	//��������С
	PCKADDR	GetPckDataAreaSize();

	//�������������ݴ�С
	PCKADDR	GetPckRedundancyDataSize();
	
	//��ѹ�ļ�
	BOOL	ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount);
	BOOL	ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount);
	
	//���ø�����Ϣ
	char*	GetAdditionalInfo();
	BOOL	SetAdditionalInfo();

	//�½�pck�ļ�
	virtual BOOL	CreatePckFileMT(LPTSTR szPckFile, LPTSTR szPath);

	//�ؽ�pck�ļ�
	virtual BOOL	RebuildPckFile(LPTSTR szRebuildPckFile);

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

	//��ȡ������Ϣ
	BOOL GetPckBasicInfo(LPTSTR lpszFile, PCKHEAD *lpHead, LPBYTE &lpFileIndexData, DWORD &dwPckFileIndexDataSize);
	BOOL SetPckBasicInfo(PCKHEAD *lpHead, LPBYTE lpFileIndexData, DWORD &dwPckFileIndexDataSize);

protected:

	//PckClass.cpp
	BOOL	MountPckFile(LPCTSTR	szFile);

	//PckClassFunction.cpp
	BOOL	OpenPckAndMappingRead(CMapViewFileRead *lpRead, LPCTSTR lpFileName);
	BOOL	OpenPckAndMappingWrite(CMapViewFileWrite *lpWrite, LPCTSTR lpFileName, DWORD dwCreationDisposition, QWORD qdwSizeToMap);


	//PckClassFunction.cpp

	BOOL	AllocIndexTableAndInit(LPPCKINDEXTABLE &lpPckIndexTable, DWORD dwFileCount);

	virtual void	BuildDirTree();
	void* AllocNodes(size_t	sizeStuct);
	VOID	DeAllocMultiNodes(LPPCK_PATH_NODE lpThisNode);

	VOID	MarkSelectedIndex();

	LPFILES_TO_COMPRESS AllocateFileinfo();
	VOID	DeallocateFileinfo();

	BOOL	AddFileToNode(LPPCK_PATH_NODE lpRootNode, LPPCKINDEXTABLE	lpPckIndexNode);
	LPPCK_PATH_NODE	FindFileNode(LPPCK_PATH_NODE lpBaseNode, char* lpszFile);

	BOOL	EnumNode(LPPCK_PATH_NODE lpNodeToExtract, LPVOID lpvoidFileRead, LPVOID lpvoidFileWrite, LPPCKINDEXTABLE_COMPRESS &lpPckIndexTablePtr, PCKADDR &dwAddress/*, DWORD	&dwCount, BOOL &bThreadRunning*/);
	VOID	EnumFile(LPSTR szFilename, BOOL IsPatition, DWORD &dwFileCount, LPFILES_TO_COMPRESS &pFileinfo, QWORD &qwTotalFileSize, size_t nLen);

	//PckClassExtract.cpp

	BOOL	StartExtract(LPPCK_PATH_NODE lpNodeToExtract, LPVOID lpMapAddress);
	BOOL	DecompressFile(char	*lpszFilename, LPPCKINDEXTABLE lpPckFileIndexTable, LPVOID lpvoidFileRead);


	//PckClassThread.cpp

	/////////���¹������ڶ��߳�pckѹ���Ĳ���
	static	VOID CompressThread(VOID* pParam);
	static	VOID WriteThread(VOID* pParam);

	//���ģʽ
	static	VOID CompressThreadAdd(VOID* pParam);
	static	VOID WriteThreadAdd(VOID* pParam);

	//����
	VOID PreProcess(int threadnum);
	BOOL FillPckHeaderAndInitArray(PCK_ALL_INFOS &PckAllInfo, int threadnum, DWORD dwFileCount);

	void MultiThreadInitialize(VOID CompressThread(VOID*), VOID WriteThread(VOID*), int threadnum);
	_inline BOOL __fastcall WritePckIndex(LPPCKINDEXTABLE_COMPRESS lpPckIndexTablePtr, PCKADDR &dwAddress);
	void AfterProcess(CMapViewFileWrite *lpWrite, PCK_ALL_INFOS &PckAllInfo, DWORD dwFileCount, PCKADDR dwAddressName, PCKADDR &dwAddress);

	///�������

	//virtual void test();

	//BOOL	ConfirmErrors(LPCSTR lpszMainString, LPCSTR lpszCaption, UINT uType);
	//BOOL	ConfirmErrors(LPCWSTR lpszMainString, LPCWSTR lpszCaption, UINT uType);
	//void	ConfirmErrors(LPCTSTR lpszCaption = NULL);

	//��ӡ��־
	void PrintLogI(char *_text);
	void PrintLogW(char *_text);
	void PrintLogE(char *_text);
	void PrintLogD(char *_text);


	void PrintLogI(wchar_t *_text);
	void PrintLogW(wchar_t *_text);
	void PrintLogE(wchar_t *_text);
	void PrintLogD(wchar_t *_text);


	void PrintLogE(char *_maintext, char *_file, char *_func, long _line);
	void PrintLogE(wchar_t *_maintext, char *_file, char *_func, long _line);
	void PrintLogE(char *_fmt, char *_maintext, char *_file, char *_func, long _line);
	void PrintLogE(char *_fmt, wchar_t *_maintext, char *_file, char *_func, long _line);

	void PrintLog(char chLevel, char *_maintext);
	void PrintLog(char chLevel, wchar_t *_maintext);
	void PrintLog(char chLevel, char *_fmt, char *_maintext);
	void PrintLog(char chLevel, char *_fmt, wchar_t *_maintext);

//����
public:

	//TCHAR	m_lastErrorString[1024];

protected:

	LPPCK_RUNTIME_PARAMS	lpPckParams;

	BOOL			m_ReadCompleted;
	PCK_ALL_INFOS	m_PckAllInfo;
	LPPCKINDEXTABLE	m_lpPckIndexTable;
	PCK_PATH_NODE	m_RootNode;

	LPFILES_TO_COMPRESS		m_firstFile;
	
	char			m_szEventAllWriteFinish[16];
	char			m_szEventAllCompressFinish[16];
	char			m_szEventMaxMemory[16];

	char			m_szMapNameRead[16];
	char			m_szMapNameWrite[16];

	//PCK�ļ��汾����
	LPPCK_KEYS		m_lpThisPckKey;

};

#endif