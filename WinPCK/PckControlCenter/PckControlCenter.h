//////////////////////////////////////////////////////////////////////
// CPckControlCenter.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�,������PCK������ݽ�������������
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.10.10
//////////////////////////////////////////////////////////////////////

#pragma once

#include "globals.h"
#include "PckHeader.h"



class CPckClass;

class CPckControlCenter
{
	//����
public:
	CPckControlCenter();
	CPckControlCenter(HWND hWnd);
	virtual ~CPckControlCenter();

	void					init();
	void					Reset(DWORD dwUIProgressUpper = 1);

	LPPCK_RUNTIME_PARAMS	GetParams();
	//void					SetParams(LPPCK_RUNTIME_PARAMS in);

	//BOOL					ConfirmErrors(LPCSTR lpszMainString, LPCSTR lpszCaption, UINT uType);
	//BOOL					ConfirmErrors(LPCWSTR lpszMainString, LPCWSTR lpszCaption, UINT uType);

	void					New();
	//BOOL					Open(LPCSTR lpszFile);
	BOOL					Open(LPCTSTR lpszFile);
	void					Close();

	int						GetPckVersion();
	void					SetPckVersion(int verID);

	//��ȡ��ǰ��������
	LPCTSTR					GetCurrentVersionName();

	LPCTSTR					GetSaveDlgFilterString();


	//���潻��
	//��ǰ�������б��index
	CONST INT				GetListCurrentHotItem();
	void					SetListCurrentHotItem(int _val);
	//�б���ʾģʽ ���ļ�����ʾor ������ʾģʽ
	CONST BOOL				GetListInSearchMode();
	void					SetListInSearchMode(BOOL _val);


	///pck ��Ľ���
	CONST	LPPCKINDEXTABLE		GetPckIndexTable();
	//CONST	LPPCK_PATH_NODE		GetPckPathNode();
	//CONST	LPPCKHEAD			GetPckHead();

	//LPCTSTR						GetLastErrorString();


	BOOL						IsValidPck();

	//��ȡnode·��
	BOOL						GetCurrentNodeString(char* szCurrentNodePathString, LPPCK_PATH_NODE lpNode);

	//��ȡ�ļ���
	DWORD						GetPckFileCount();

	QWORD						GetPckSize();
	QWORD						GetPckDataAreaSize();
	QWORD						GetPckRedundancyDataSize();

	//Ԥ���ļ�
	BOOL						GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpPckFileIndexTable, char *buffer, size_t sizeOfBuffer = 0);


	//���ø�����Ϣ
	char*						GetAdditionalInfo();
	BOOL						SetAdditionalInfo();

	VOID						RenameIndex(LPPCK_PATH_NODE lpNode, char* lpszReplaceString);
	VOID						RenameIndex(LPPCKINDEXTABLE lpIndex, char* lpszReplaceString);

	//������һ���ڵ�
	BOOL						RenameNode(LPPCK_PATH_NODE lpNode, char* lpszReplaceString);

	//��ѹ�ļ�
	BOOL						ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount);
	BOOL						ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount);


	//�½�pck�ļ�
	BOOL	CreatePckFile(LPTSTR szPckFile, LPTSTR szPath);

	//�ؽ�pck�ļ�
	BOOL	RebuildPckFile(LPTSTR szRebuildPckFile);
	BOOL	RecompressPckFile(LPTSTR szRecompressPckFile);

	//����pck�ļ�
	BOOL	UpdatePckFile(LPTSTR szPckFile, TCHAR(*lpszFilePath)[MAX_PATH], int nFileCount, LPPCK_PATH_NODE lpNodeToInsert);

	//�Ƿ���֧�ָ��µ��ļ�
	BOOL	isSupportAddFileToPck();

	//�������ļ�
	BOOL	RenameFilename();

	//ɾ��һ���ڵ�
	VOID	DeleteNode(LPPCK_PATH_NODE lpNode);


	//log��־��ع��� 
	void	SetLogListWnd(HWND _hWndList);


	void	PrintLogE(const char *);
	void	PrintLogW(const char *);
	void	PrintLogI(const char *);
	void	PrintLogD(const char *);
	void	PrintLogN(const char *);

	void	PrintLogE(const wchar_t *);
	void	PrintLogW(const wchar_t *);
	void	PrintLogI(const wchar_t *);
	void	PrintLogD(const wchar_t *);
	void	PrintLogN(const wchar_t *);

	//__FILE__, __FUNCTION__, __LINE__
	void	PrintLogE(const char *_maintext, const char *_file, const char *_func, const long _line);
	void	PrintLogE(const wchar_t *_maintext, const char *_file, const char *_func, const long _line);
	void	PrintLogE(const char *_fmt, const char *_maintext, const char *_file, const char *_func, const long _line);
	void	PrintLogE(const char *_fmt, const wchar_t *_maintext, const char *_file, const char *_func, const long _line);

	void	PrintLog(const char chLevel, const char *_maintext);
	void	PrintLog(const char chLevel, const wchar_t *_maintext);
	void	PrintLog(const char chLevel, const char *_fmt, const char *_maintext);
	void	PrintLog(const char chLevel, const char *_fmt, const wchar_t *_maintext);

	//����PCK��ѹ������
	void	ResetCompressor();

private:

	void	CreateRestoreData();
	void	RestoreData(LPCTSTR lpszFile);
	void	DeleteRestoreData();

	//log��־��ع��� 
	void	_InsertLogIntoList(const int, const char *);
	void	_InsertLogIntoList(const int, const wchar_t *);

public:


	LPPCK_PATH_NODE				m_lpPckRootNode;

private:

	BOOL						m_isSearchMode;

	PCK_RUNTIME_PARAMS			cParams;
	CPckClass					*m_lpClassPck;

	BOOL						hasRestoreData;

	TCHAR						m_lpszFile4Restore[MAX_PATH];

	PCKHEAD_V2020				m_PckHeadForRestore;
	LPBYTE						m_lpPckFileIndexData;
	DWORD						m_dwPckFileIndexDataSize;


	//LOG
	HWND						m_hWndMain;
	//HWND						m_hWndLogWnd;
	HWND						m_hWndLogListWnd;
	int							m_LogListCount;

	DWORD						m_dwLastError;

	//��ʽ
	FMTPCK						m_emunFileFormat;

};