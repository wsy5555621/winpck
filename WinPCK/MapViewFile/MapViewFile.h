//////////////////////////////////////////////////////////////////////
// CMapViewFile.h: ����ӳ���ļ���ͼ
// 
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "PckConf.h"

#if !defined(_MAPVIEWFILE_H_)
#define _MAPVIEWFILE_H_

typedef unsigned __int64	QWORD;

typedef union _QWORD{
	QWORD qwValue;
	struct {
		DWORD dwValue;
		DWORD dwValueHigh;
	};
	struct {
		LONG lValue;
		LONG lValueHigh;
	};
}UNQWORD, *LPUNQWORD;

#define USE_MAX_SINGLE_FILESIZE


#define PATH_LOCAL_PREFIX			"\\\\?\\"
#define PATH_UNC_PREFIX				"\\\\?\\UNC"

#define PATHW_LOCAL_PREFIX			L"\\\\?\\"
#define PATHW_UNC_PREFIX			L"\\\\?\\UNC"

#define PATH_LOCAL_PREFIX_LEN		4
#define PATH_UNC_PREFIX_LEN			7

#ifdef USE_MAX_SINGLE_FILESIZE

#ifdef _DEBUG
//#define MAX_PCKFILE_SIZE			0x7fffff00
//#define MAX_PCKFILE_SIZE			0x6400000	//100MB

#if defined PCKV202
#define MAX_PCKFILE_SIZE			0x7fffff00
#elif defined PCKV203 || defined PCKV203ZX
#define MAX_PCKFILE_SIZE			0xffffff00
#endif

#else
#if defined PCKV202 || defined PCKV203ZX
#define MAX_PCKFILE_SIZE			0x7fffff00
#elif defined PCKV203 || defined PCKV203ZX
#define MAX_PCKFILE_SIZE			0xffffff00
#endif
#endif

#endif


#ifdef UNICODE
#define tcscpy_s	wcscpy_s
#define	tcslen		wcslen
#else
#define tcscpy_s	strcpy_s
#define	tcslen		strlen
#endif



class CMapViewFile
{
public:
	CMapViewFile();
	virtual ~CMapViewFile();


	LPBYTE	View(QWORD dwAddress, DWORD dwSize = 0);
	virtual LPBYTE	ReView(QWORD dwAddress, DWORD dwSize = 0);

	virtual void	SetFilePointer(QWORD lDistanceToMove, DWORD dwMoveMethod);

	virtual DWORD	Read(LPVOID buffer, DWORD dwBytesToRead);

	QWORD	GetFileSize();

	void	UnmapView();
	void	UnMaping();
	void	clear();

protected:

	
	BOOL isWinNt();
	void MakeUnlimitedPath(WCHAR *_dst, LPCWSTR	_src, size_t size);
	void MakeUnlimitedPath(char *_dst, LPCSTR _src, size_t size);

#ifdef USE_MAX_SINGLE_FILESIZE
	void GetPkxName(char *dst, LPCSTR src);
	void GetPkxName(LPWSTR dst, LPCWSTR src);
#endif

	LPBYTE	ViewReal(LPVOID & lpMapAddress, HANDLE hFileMapping, DWORD dwAddress, DWORD dwSize);

public:

	LPBYTE	lpTargetBuffer;

protected:

	HANDLE	hFile;
	HANDLE	hFileMapping;
	LPVOID	lpMapAddress;
	//LPVOID	lpVirtualAddress;

#ifdef USE_MAX_SINGLE_FILESIZE

	HANDLE	hFile2;
	HANDLE	hFileMapping2;
	LPVOID	lpMapAddress2;

	QWORD	dwPckSize, dwPkxSize;
	UNQWORD	uqdwMaxPckSize;

	UNQWORD	uqwFullSize;
	//UNQWORD	uqwMaxPckSize;

	BOOL	IsPckFile, hasPkx;

	BOOL	isCrossView;	//�Ƿ��ǿ��ļ���VIEW
	LPBYTE	lpCrossBuffer, lpCrossAddressPck;

	DWORD	dwViewSizePck, dwViewSizePkx;

	char	m_szPckFileName[MAX_PATH];
	wchar_t	m_tszPckFileName[MAX_PATH];

	char	m_szPkxFileName[MAX_PATH];
	wchar_t	m_tszPkxFileName[MAX_PATH];
#endif

	//DWORD	dwCurrentPos;
	UNQWORD	uqwCurrentPos;

	BOOL	isWriteMode;
};







class CMapViewFileRead : public CMapViewFile
{
public:
	CMapViewFileRead();
	//CMapViewFileRead(char _filename);
	//CMapViewFileRead(wchar_t _filename);

	virtual ~CMapViewFileRead();

#ifdef USE_MAX_SINGLE_FILESIZE
	BOOL	OpenPck(LPSTR lpszFilename);
	BOOL	OpenPck(LPCWSTR lpszFilename);
#endif

	BOOL	Open(char *lpszFilename);
	BOOL	Open(LPCWSTR lpszFilename);

	BOOL	Mapping(char *lpszNamespace);

protected:


};


class CMapViewFileWrite : public CMapViewFile
{
public:
	CMapViewFileWrite();
	virtual ~CMapViewFileWrite();
	
#ifdef USE_MAX_SINGLE_FILESIZE
	BOOL	OpenPck(char *lpszFilename, DWORD dwCreationDisposition);
	BOOL	OpenPck(LPCWSTR lpszFilename, DWORD dwCreationDisposition);
#endif

	BOOL	Open(char *lpszFilename, DWORD dwCreationDisposition);
	BOOL	Open(LPCWSTR lpszFilename, DWORD dwCreationDisposition);

	BOOL	Mapping(char *lpszNamespace, QWORD dwMaxSize);
	void	SetEndOfFile();

	DWORD	Write(LPVOID buffer, DWORD dwBytesToWrite);

	//BOOL	FlushViewOfFile();

protected:

#ifdef USE_MAX_SINGLE_FILESIZE
	//BOOL	OpenPck(char *lpszFilename, DWORD dwCreationDisposition);
	//BOOL	OpenPck(LPCTSTR lpszFilename, DWORD dwCreationDisposition);

	void	OpenPkx(char *lpszFilename, DWORD dwCreationDisposition);
	void	OpenPkx(LPCWSTR lpszFilename, DWORD dwCreationDisposition);
#endif

public:

protected:
#ifdef USE_MAX_SINGLE_FILESIZE

#endif

};

#endif //_MAPVIEWFILE_H_