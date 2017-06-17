//////////////////////////////////////////////////////////////////////
// ZupClass.h: ���ڽ����������繫˾��zup�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.5.23
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"
#include "ZupHeader.h"
#include "DictHash.h"

#if !defined(_ZUPCLASS_H_)
#define _ZUPCLASS_H_

class CZupClass : public CPckClass
{
public:

	//CZupClass();
	CZupClass(LPPCK_RUNTIME_PARAMS inout);
	virtual ~CZupClass();

	BOOL	Init(LPCTSTR	szFile);

	CONST	LPPCKINDEXTABLE		GetPckIndexTable();
	CONST	LPPCK_PATH_NODE		GetPckPathNode();

	//��ѹ�ļ�
	//virtual BOOL	ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount, DWORD &dwCount, BOOL	&bThreadRunning);
	//virtual BOOL	ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount, DWORD &dwCount, BOOL	&bThreadRunning);
	
	//���ø�����Ϣ
	//char*	GetAdditionalInfo();
	//BOOL	SetAdditionalInfo();

	//�½�pck�ļ�
	//BOOL	CreatePckFile(LPTSTR szPckFile, LPTSTR szPath, int level, DWORD &dwCount, DWORD &dwAllCount, BOOL	&bThreadRunning);
	virtual BOOL	CreatePckFileMT(LPTSTR szPckFile, LPTSTR szPath/*, int level, int threadnum, int &nMaxMemory, DWORD &dwCount, DWORD &dwAllCount, BOOL	&bThreadRunning*/);

	//�ؽ�pck�ļ�
	virtual BOOL	RebuildPckFile(LPTSTR szRebuildPckFile/*,  DWORD &dwCount, BOOL &bThreadRunning*/);

	//����pck�ļ�//��bug
	virtual BOOL	UpdatePckFile(LPTSTR szPckFile, TCHAR (*lpszFilePath)[MAX_PATH], int nFileCount, LPPCK_PATH_NODE lpNodeToInsert);

	//�������ļ�
	virtual BOOL	RenameFilename(/*int level, DWORD &dwCount, DWORD &dwAllCount*/);

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
	BOOL	GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpZupFileIndexTable, char *buffer, size_t sizeOfBuffer = 0);


protected:


protected:

	PCK_PATH_NODE				m_RootNodeZup;
	LPPCKINDEXTABLE				m_lpZupIndexTable;

	//LPZUP_FILENAME_DICT			m_lpZupDict;
	CDictHash					*m_lpDictHash;


protected:

	virtual void	BuildDirTree();
	BOOL	BuildZupBaseDict();
	void	DecodeFilename(char *_dst, char *_src);

	_inline void	DecodeDict(LPZUP_FILENAME_DICT lpZupDict);
	VOID	AddDict(char *&lpszStringToAdd);
	//void	DeleteDictBuffer(LPZUP_FILENAME_DICT lpDictHead);
	VOID	EnumAndDecodeNode(LPPCK_PATH_NODE lpNodeToDecodeZup, LPPCK_PATH_NODE lpNodeToDecodePck);

	LPPCKINDEXTABLE GetBaseFileIndex(LPPCKINDEXTABLE lpIndex, LPPCKINDEXTABLE lpZeroBaseIndex);

};




#endif