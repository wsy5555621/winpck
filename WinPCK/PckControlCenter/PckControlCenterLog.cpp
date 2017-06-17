
//////////////////////////////////////////////////////////////////////
// PckControlCenterLog.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�,������PCK������ݽ�������������
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.21
//////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4996 )

#include "PckControlCenter.h"
//#include "PckClass.h"
#include <commctrl.h>


char *GetErrorMsg( CONST DWORD dwError)
{
	static char szMessage[1024] = "����ԭ��";
	//strcpy(szMessage, "����ԭ��");

	char *lpszMessage = szMessage + 10;
	// retrieve a message from the system message table
	switch(dwError)
	{
	case 2:
		strcpy(lpszMessage, "ϵͳ�Ҳ���ָ�����ļ���");
		break;
	case 3:
		strcpy(lpszMessage, "ϵͳ�Ҳ���ָ����·����");
		break;
	case 4:
		strcpy(lpszMessage, "ϵͳ�޷����ļ���");
		break;
	case 5:
		strcpy(lpszMessage, "�ܾ����ʡ�");
		break;
	case 32:
		strcpy(lpszMessage, "��һ����������ʹ�ô��ļ��������޷����ʡ�");
		break;
	default:
		if (!FormatMessageA( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			lpszMessage,
			1014,
			NULL ))
		{
			strcpy(lpszMessage, "δ֪����");
		}
	}
	SetLastError(0);
	return szMessage;
}


inline char *UCS2toA(const WCHAR *src, int max_len=-1)
{
	static char dst[8192];
	//DWORD dwLastError = GetLastError();

	::WideCharToMultiByte(CP_ACP, 0, src, max_len, dst, 8192, "_", 0);

	//SetLastError(dwLastError);
	return dst;
}

void CPckControlCenter::SetLogListWnd(HWND _hWndList)
{
	m_hWndLogListWnd = _hWndList;
}


void CPckControlCenter::_InsertLogIntoList(int _loglevel, char *_logtext)
{

	LVITEMA	item;
	SYSTEMTIME systime;

	char szPrintf[4096];

	GetLocalTime (&systime);

	StringCchPrintfA(szPrintf, 4096, "%02d:%02d:%02d %s", systime.wHour, systime.wMinute, systime.wSecond, _logtext);


	ZeroMemory(&item, sizeof(LVITEMA));

	item.iItem = INT_MAX;			//��0��ʼ
	item.iImage = _loglevel;
	//item.iSubItem = 0;
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.pszText = szPrintf;

	if(-1 != (m_LogListCount = ::SendMessageA(m_hWndLogListWnd, LVM_INSERTITEMA, 0, (LPARAM)&item))){
		ListView_EnsureVisible(m_hWndLogListWnd, m_LogListCount, 0);
		//++m_LogListCount;
	}

}

void CPckControlCenter::PrintLogI(char *_logtext){
	_InsertLogIntoList(LOG_IMAGE_INFO, _logtext);
}

void CPckControlCenter::PrintLogW(char *_logtext){
	_InsertLogIntoList(LOG_IMAGE_WARNING, _logtext);
}

void CPckControlCenter::PrintLogE(char *_logtext){
	_InsertLogIntoList(LOG_IMAGE_ERROR, _logtext);
	ShowWindow(GetParent(m_hWndLogListWnd), SW_SHOW);
}


void CPckControlCenter::PrintLogD(char *_logtext){
	_InsertLogIntoList(LOG_IMAGE_DEBUG, _logtext);
}


void CPckControlCenter::_InsertLogIntoList(int _loglevel, wchar_t *_logtext)
{

	LVITEMW	item;
	SYSTEMTIME systime;

	wchar_t szPrintf[4096];

	GetLocalTime (&systime);

	StringCchPrintfW(szPrintf, 4096, L"%02d:%02d:%02d %s", systime.wHour, systime.wMinute, systime.wSecond, _logtext);


	ZeroMemory(&item, sizeof(LVITEMW));

	item.iItem = INT_MAX;			//��0��ʼ
	item.iImage = _loglevel;
	//item.iSubItem = 0;
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.pszText = szPrintf;

	if(-1 != (m_LogListCount = ::SendMessageW(m_hWndLogListWnd, LVM_INSERTITEMW, 0, (LPARAM)&item))){
		ListView_EnsureVisible(m_hWndLogListWnd, m_LogListCount, 0);
		//++m_LogListCount;
	}
	
}

void CPckControlCenter::PrintLogI(wchar_t *_logtext){
	_InsertLogIntoList(LOG_IMAGE_INFO, _logtext);
}

void CPckControlCenter::PrintLogW(wchar_t *_logtext){
	_InsertLogIntoList(LOG_IMAGE_WARNING, _logtext);
}

void CPckControlCenter::PrintLogE(wchar_t *_logtext){
	_InsertLogIntoList(LOG_IMAGE_ERROR, _logtext);
	ShowWindow(GetParent(m_hWndLogListWnd), SW_SHOW);
}


void CPckControlCenter::PrintLogD(wchar_t *_logtext){
	_InsertLogIntoList(LOG_IMAGE_DEBUG, _logtext);
}


void CPckControlCenter::PrintLogE(char *_maintext, char *_file, char *_func, long _line)
{
	char szPrintf[8192];
	StringCchPrintfA(szPrintf, 8192, "%s (%s ���������� %s ����:%d)", _maintext, _file, _func, _line);
	PrintLogE(szPrintf);
	if(0 == m_dwLastError)
		PrintLogE(GetErrorMsg(GetLastError()));
	else{
		PrintLogE(GetErrorMsg(m_dwLastError));
		m_dwLastError = 0;
	}
}

void CPckControlCenter::PrintLogE(wchar_t *_maintext, char *_file, char *_func, long _line)
{
	m_dwLastError = GetLastError();
	PrintLogE(UCS2toA(_maintext), _file, _func, _line);
}

void CPckControlCenter::PrintLogE(char *_fmt, char *_maintext, char *_file, char *_func, long _line)
{
	char szPrintf[8192];
	StringCchPrintfA(szPrintf, 8192, _fmt, _maintext);
	PrintLogE(szPrintf, _file, _func, _line);

}
void CPckControlCenter::PrintLogE(char *_fmt, wchar_t *_maintext, char *_file, char *_func, long _line)
{
	m_dwLastError = GetLastError();
	PrintLogE(_fmt, UCS2toA(_maintext), _file, _func, _line);
}


void CPckControlCenter::PrintLog(char chLevel, char *_maintext)
{
	//char szPrintf[8192];
	//StringCchPrintfA(szPrintf, 8192, "%s", _maintext);
	switch(chLevel)
	{
	case LOG_FLAG_ERROR:
		PrintLogE(_maintext);
		break;
	case LOG_FLAG_WARNING:
		PrintLogW(_maintext);
		break;
	case LOG_FLAG_INFO:
		PrintLogI(_maintext);
		break;
#ifdef _DEBUG
	case LOG_FLAG_DEBUG:
		PrintLogD(_maintext);
		break;
#endif
	default:
		PrintLogI(_maintext);
	}
	//PrintLogE(szPrintf);
}

void CPckControlCenter::PrintLog(char chLevel, wchar_t *_maintext)
{
	PrintLog(chLevel, UCS2toA(_maintext));
}

void CPckControlCenter::PrintLog(char chLevel, char *_fmt, char *_maintext)
{
	char szPrintf[8192];
	StringCchPrintfA(szPrintf, 8192, _fmt, _maintext);
	PrintLog(chLevel, szPrintf);

}
void CPckControlCenter::PrintLog(char chLevel, char *_fmt, wchar_t *_maintext)
{
	PrintLog(chLevel, _fmt, UCS2toA(_maintext));
}