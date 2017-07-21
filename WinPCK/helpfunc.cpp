//////////////////////////////////////////////////////////////////////
// helpfunc.cpp: WinPCK �����̲߳���
// �Ϸš��򿪱����ļ���Ԥ��
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4995 )
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4005 )

#include "tlib.h"
#include "resource.h"
//#include "globals.h"
#include "winmain.h"
#include "miscdlg.h"
#include <shlobj.h>
#include <strsafe.h>
#include <process.h>

inline LONG RecurseDeleteKey(HKEY hRegKey, LPCTSTR lpszKey);
inline void CreateAndSetDefaultValue(LPCTSTR pszValueName, LPCTSTR pszValue);

VOID TInstDlg::ViewFileAttribute()
{
	if(lpPckParams->cVarParams.bThreadRunning)return;

	HWND	hWndList = GetDlgItem(IDC_LIST);
	//int		iHotitem = ListView_GetHotItem(hWndList);
	int		iHotitem = m_lpPckCenter->GetListCurrentHotItem();
	
	if(0 == iHotitem)return;

	LVITEM	item = {0};

	item.mask = LVIF_PARAM;
	item.iItem = iHotitem;
	ListView_GetItem(hWndList, &item);


	if(m_lpPckCenter->IsValidPck())
	{
		char	szPath[MAX_PATH_PCK_260];

		m_lpPckCenter->GetCurrentNodeString(szPath, m_currentNodeOnShow);

		TAttrDlg	dlg((void*)item.lParam, (void*)m_lpPckCenter->m_lpPckRootNode, m_lpPckCenter->GetPckRedundancyDataSize(), szPath, m_lpPckCenter->GetListInSearchMode(), this);
		dlg.Exec();
	}

}


VOID TInstDlg::ViewFile()
{
	if(lpPckParams->cVarParams.bThreadRunning)return;

	HWND	hWndList = GetDlgItem(IDC_LIST);
	int		iHotitem = m_lpPckCenter->GetListCurrentHotItem();

	DWORD	dwFilesizeToView;

	//LPPCK_PATH_NODE		lpNodeToShow;
	//LPPCKINDEXTABLE		lpIndexToShow;
	LPPCKINDEXTABLE		lpPckFileIndexToShow;

	LVITEM	item = {0};

	item.mask = LVIF_PARAM;
	item.iItem = iHotitem;
	ListView_GetItem(hWndList, &item);

	if(m_lpPckCenter->GetListInSearchMode())
	{
		lpPckFileIndexToShow = (LPPCKINDEXTABLE)item.lParam;
		
	}else{
		lpPckFileIndexToShow = ((LPPCK_PATH_NODE)item.lParam)->lpPckIndexTable;
	}

	dwFilesizeToView = lpPckFileIndexToShow->cFileIndex.dwFileClearTextSize;


	char *buf;

	TDlg *dlg;
	//BOOL	isDds;

	char	*lpszFileExt = strrchr(lpPckFileIndexToShow->cFileIndex.szFilename, '.');
	char	*lpszFileTitle;
	//BOOL ((CPckControlCenter::*GetSingleFileData)(LPVOID, LPVOID, char *, size_t)) = *(m_lpPckCenter->GetSingleFileData);

	if(m_lpPckCenter->GetListInSearchMode())
	{
		if(NULL == (lpszFileTitle = strrchr(lpPckFileIndexToShow->cFileIndex.szFilename, '\\')))
			if(NULL == (lpszFileTitle = strrchr(lpPckFileIndexToShow->cFileIndex.szFilename, '/')))
				lpszFileTitle = lpPckFileIndexToShow->cFileIndex.szFilename - 1;

		//��ǰ���'\\'��'/'ȥ��
		lpszFileTitle++;
	}else
		lpszFileTitle = ((LPPCK_PATH_NODE)item.lParam)->szName;

	if(NULL != lpszFileExt)
	{
		//ת��lpszFileExtΪСд,��strlen(lpszFileExt) > 6 ʱ�����޸�
		if(4 == strnlen_s(lpszFileExt, 6)){
			_strlwr_s(lpszFileExt, 6);

			if(NULL != strstr(FILE_EXT_PIC, lpszFileExt))
			{
				if(0 == strcmpi(lpszFileExt, ".dds"))
					dlg = new TPicDlg(&buf, dwFilesizeToView, FMT_DDS, lpszFileTitle, this);
				else if(0 == strcmpi(lpszFileExt, ".tga"))
					dlg = new TPicDlg(&buf, dwFilesizeToView, FMT_TGA, lpszFileTitle, this);
				else
					dlg = new TPicDlg(&buf, dwFilesizeToView, FMT_RAW, lpszFileTitle, this);
			
			}else{
				dlg = new TViewDlg(&buf, dwFilesizeToView, lpszFileTitle, this);
			}
		}else{
			dlg = new TViewDlg(&buf, dwFilesizeToView, lpszFileTitle, this);
		}
	}else{
		dlg = new TViewDlg(&buf, dwFilesizeToView, lpszFileTitle, this);
	}

	if(0 != dwFilesizeToView){
		if(NULL == buf)
		{
			m_lpPckCenter->PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			delete dlg;
			return;
		}

		m_lpPckCenter->GetSingleFileData(NULL, lpPckFileIndexToShow, buf, dwFilesizeToView);
	}

	dlg->Exec();

	delete dlg;

}

BOOL TInstDlg::AddFiles()
{

	if(lpPckParams->cVarParams.bThreadRunning)return FALSE;

	if(IDCANCEL == MessageBoxA("ȷ������ļ���", "ѯ��", MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2))return FALSE;
	
	//TCHAR	(*lpszFilePathPtr)[MAX_PATH];
	//DWORD	dwFileCount;

	LPVOID	lpszFilePathPtr;

	if(OpenFiles(lpszFilePathPtr, m_DropFileCount))
	{

		m_lpszFilePath = (TCHAR(*)[MAX_PATH])lpszFilePathPtr;
		DragAcceptFiles(hWnd,FALSE);

		//mt_MaxMemoryCount = mt_MaxMemory;
		_beginthread(UpdatePckFile, 0, this);

		
	}

	return FALSE;
}




/******************************************************
ֻ��һ���ļ�
******************************************************/

BOOL TInstDlg::OpenSingleFile(TCHAR * lpszFileName)
{
	OPENFILENAME ofn;
	TCHAR szStrPrintf[MAX_PATH];
	*szStrPrintf = 0;
	
	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	
	if('\\' == *(lpszFileName + lstrlen(lpszFileName) - 1)){
		ofn.lpstrInitialDir   = lpszFileName;
		ofn.lpstrFile         = szStrPrintf;
	}else{
		ofn.lpstrFile         = lpszFileName;
	}

	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hWnd;
	ofn.lpstrFilter       = TEXT_FILE_FILTER ;
	//ofn.lpstrFile         = lpszFileName;
	//ofn.lpstrInitialDir   = lpszFileName;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.Flags             = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING/* | OFN_ALLOWMULTISELECT*/;

	if(!GetOpenFileName( & ofn ))
	{
		return FALSE;
	}

	if(NULL != ofn.lpstrInitialDir)
		StringCchCopy(lpszFileName, MAX_PATH, szStrPrintf);

	return TRUE;

}

BOOL TInstDlg::OpenFiles(LPVOID &lpszFilePathArray, DWORD &dwFileCount)
{


	OPENFILENAME ofn;
	TCHAR * szBuffer, * szBufferPart;
	//FILEINFO * pFileinfo, * pFileinfo_previtem;
	size_t stStringLength;

	szBuffer = (TCHAR *) malloc(MAX_BUFFER_SIZE_OFN * sizeof(TCHAR)); 
	if(szBuffer == NULL){
		m_lpPckCenter->PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	StringCchCopy(szBuffer, MAX_BUFFER_SIZE_OFN, TEXT(""));
	
	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hWnd ;
	ofn.lpstrFilter       = TEXT("�����ļ�\0*.*\0\0");
	ofn.lpstrFile         = szBuffer;
	ofn.nMaxFile          = MAX_BUFFER_SIZE_OFN ;
	ofn.Flags             = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;

	if(!GetOpenFileName( & ofn )){
		if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL){
			MessageBox(TEXT("ѡ����ļ�̫��. �������޷�װ�������ļ����ļ�����"),
						TEXT("����������"), MB_OK);
		}
		free(szBuffer);
		return FALSE;
	}

	dwFileCount = 0;




	// if first part of szBuffer is a directory the user selected multiple files
	// otherwise szBuffer is filename + path
	if(GetFileAttributes(szBuffer) & FILE_ATTRIBUTE_DIRECTORY){
		// szBuffer �е�һ��������Ŀ¼ 
		// ���������� FileTitle


		TCHAR		**szFilenamePtrArray, **szFilenamePtrArrayPtr;
		if(NULL == (szFilenamePtrArray = (TCHAR**) malloc (MAX_BUFFER_SIZE_OFN * sizeof(TCHAR**))))
		{
			m_lpPckCenter->PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			free(szBuffer);
			return FALSE;

		}

		szFilenamePtrArrayPtr = szFilenamePtrArray;

		szBufferPart = szBuffer + ofn.nFileOffset;
		*szFilenamePtrArrayPtr = szBuffer + ofn.nFileOffset;
		
		//��Ŀ¼��ʱĿ¼���л��'\'
		if( 4 == ofn.nFileOffset )
		{
			ofn.nFileOffset--;
			szBuffer[2] = 0;
		}

		
		while(0 != *szBufferPart){

			StringCchLength(szBufferPart, MAX_PATH, & stStringLength);
			szBufferPart += stStringLength + 1;

			*(++szFilenamePtrArrayPtr) = szBufferPart;

			dwFileCount++;

		}

		*szFilenamePtrArrayPtr = 0;

		if(NULL == (lpszFilePathArray = /*(TCHAR(*)[MAX_PATH])*/ malloc (sizeof(TCHAR) * MAX_PATH * dwFileCount)))
		{
			m_lpPckCenter->PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			return	FALSE;
		}

		TCHAR (*lpszFilePathArrayPtr)[MAX_PATH] = (TCHAR(*)[MAX_PATH])lpszFilePathArray;

		szFilenamePtrArrayPtr = szFilenamePtrArray;

		while(0 != *szFilenamePtrArrayPtr)
		{
			StringCchPrintf(*lpszFilePathArrayPtr, MAX_PATH, TEXT("%s\\%s"), szBuffer, *szFilenamePtrArrayPtr);

			lpszFilePathArrayPtr++;
			szFilenamePtrArrayPtr++;
		}

		free(szFilenamePtrArray);
	}
	else{ // ��ѡ����һ���ļ�

		dwFileCount = 1;

		if(NULL == (lpszFilePathArray = /*(TCHAR(*)[MAX_PATH])*/ malloc (sizeof(TCHAR) * MAX_PATH)))
		{
			m_lpPckCenter->PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			return	FALSE;
		}

		StringCchCopy((TCHAR*)lpszFilePathArray, MAX_PATH, szBuffer);

	}

	// ɾ��������
	
	free(szBuffer);

	return TRUE;
}


DWORD TInstDlg::SaveFile(TCHAR * lpszFileName, LPCTSTR lpstrFilter, DWORD nFilterIndex)
{
	OPENFILENAME ofn;
	//TCHAR szStrPrintf[260];
	
	ZeroMemory(& ofn, sizeof (OPENFILENAME));
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hWnd;
	//ofn.lpstrFilter       = TEXT_FILE_FILTER;
	ofn.lpstrFilter       = lpstrFilter;
	ofn.lpstrFile         = lpszFileName ;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.Flags             = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING;
	ofn.lpstrDefExt		  = TEXT("pck");
	ofn.nFilterIndex	  = nFilterIndex;

	if(!GetSaveFileName( & ofn ))
	{
		return FALSE;
	}
	return ofn.nFilterIndex;

}

BOOL TInstDlg::BrowseForFolderByPath(TCHAR * lpszPathName)
{

	BROWSEINFO					cBI;
	ITEMIDLIST					idl;
	LPITEMIDLIST				pidl = &idl;

	cBI.hwndOwner = hWnd;
	cBI.pidlRoot = 0;
	cBI.lpszTitle = TEXT("��ѡ��Ŀ¼");
	cBI.lpfn = BFFCALLBACK(&BFFCallBack);

	cBI.lParam = (LPARAM)lpszPathName;

	cBI.pszDisplayName = lpszPathName;

	cBI.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	//OleInitialize(0);
	pidl = SHBrowseForFolder(&cBI);
	//OleUninitialize();
	if(NULL != pidl)
	{
		SHGetPathFromIDList(pidl, lpszPathName);
		CoTaskMemFree((VOID*)pidl);
		return TRUE;
	}
	return FALSE;
}

int CALLBACK TInstDlg::BFFCallBack(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	//TCHAR szPath[MAX_PATH];
	//TCHAR szExePath[MAX_PATH];

	//ITEMIDLIST					idl;
	//LPITEMIDLIST				pidl = &idl;
	//BOOL bTemp;

    switch(uMsg)
	{
	case BFFM_INITIALIZED:
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		break;
	}

	return 0;
}

void TInstDlg::AddSetupReg()
{
	
	//BOOL	isExistOldReg;
	HKEY	hRegKey;
	LONG	result;
	//DWORD	dwType;

	//TCHAR	szString[MAX_PATH];
	TCHAR	szStringIcon[MAX_PATH];
	TCHAR	szStringExec[MAX_PATH];

	StringCchCopy(szStringIcon, MAX_PATH, m_MyFileName);
	StringCchCat(szStringIcon, MAX_PATH, TEXT(",0"));
	

	StringCchCopy(szStringExec, MAX_PATH, TEXT("\""));
	StringCchCat(szStringExec, MAX_PATH, m_MyFileName);
	StringCchCat(szStringExec, MAX_PATH, TEXT("\" \"%1\""));

	//DWORD	dwDataLength;
	//m_MyFileName

	//����Ƿ����[HKEY_CLASSES_ROOT\pckfile]
	if (ERROR_SUCCESS == (result = RegOpenKeyEx(	HKEY_CLASSES_ROOT, 
													TEXT("pckfile\\shell\\open\\command"),
													0,
													KEY_READ,
													&hRegKey)))
	{
		//����
		//1.����������patcher.exe���¼�
		//result = RegQueryValueEx(hRegKey, NULL, NULL, &dwType, reinterpret_cast<LPBYTE>(szString), &dwDataLength)

		//2.�����滻

		//3.�޸Ĵ򿪷�ʽ����

		RegCloseKey(hRegKey);

		RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT(".pck"));
		RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT("pckfile"));
		RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT(".zup"));
		RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT("ZPWUpdatePack"));

	}

	CreateAndSetDefaultValue(TEXT(".pck"), TEXT("pckfile"));
	CreateAndSetDefaultValue(TEXT("pckfile"), TEXT("Angelica File Package"));
	CreateAndSetDefaultValue(TEXT("pckfile\\DefaultIcon"), szStringIcon);
	CreateAndSetDefaultValue(TEXT("pckfile\\shell"), NULL);
	CreateAndSetDefaultValue(TEXT("pckfile\\shell\\open"), TEXT("ʹ�� WinPCK ��"));
	CreateAndSetDefaultValue(TEXT("pckfile\\shell\\open\\command"), szStringExec);

	CreateAndSetDefaultValue(TEXT(".zup"), TEXT("ZPWUpdatePack"));
	CreateAndSetDefaultValue(TEXT("ZPWUpdatePack"), TEXT("���ɸ��°�"));
	CreateAndSetDefaultValue(TEXT("ZPWUpdatePack\\DefaultIcon"), szStringIcon);
	CreateAndSetDefaultValue(TEXT("ZPWUpdatePack\\shell"), NULL);
	CreateAndSetDefaultValue(TEXT("ZPWUpdatePack\\shell\\open"), TEXT("ʹ�� WinPCK ��"));
	CreateAndSetDefaultValue(TEXT("ZPWUpdatePack\\shell\\open\\command"), szStringExec);

	MessageBox(TEXT("��װ��ɡ�"), TEXT("��Ϣ"), MB_OK | MB_ICONASTERISK);
}

void TInstDlg::DeleteSetupReg()
{
	RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT(".pck"));
	RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT("pckfile"));
	RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT(".zup"));
	RecurseDeleteKey(HKEY_CLASSES_ROOT, TEXT("ZPWUpdatePack"));

	MessageBox(TEXT("ж����ɡ�"), TEXT("��Ϣ"), MB_OK | MB_ICONASTERISK);
}

inline void CreateAndSetDefaultValue(LPCTSTR pszValueName, LPCTSTR pszValue)
{
	HKEY	hRegKey;

	if (ERROR_SUCCESS == RegCreateKeyEx(	HKEY_CLASSES_ROOT,
											pszValueName,
											0,
											REG_NONE,
											REG_OPTION_NON_VOLATILE,
											KEY_READ | KEY_WRITE,
											NULL,
											&hRegKey,
											NULL))
	{
		if(NULL != pszValue)
			RegSetValueEx(hRegKey, NULL, NULL, REG_SZ, reinterpret_cast<const BYTE*>(pszValue), (lstrlen(pszValue)+1)*sizeof(TCHAR));
		
		RegCloseKey(hRegKey);
	}

}

inline LONG RecurseDeleteKey(HKEY hRegKey, LPCTSTR lpszKey)
{
	HKEY hSubRegKey;
	LONG lRes = RegOpenKeyEx(hRegKey, lpszKey, 0, KEY_READ | KEY_WRITE, &hSubRegKey);
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	FILETIME time;
	DWORD dwSize = 256;
	TCHAR szBuffer[256];
	while (RegEnumKeyEx(hSubRegKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
		&time)==ERROR_SUCCESS)
	{
		lRes = RecurseDeleteKey(hSubRegKey, szBuffer);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		dwSize = 256;
	}
	
	RegCloseKey(hSubRegKey);

	return RegDeleteKey(hRegKey, lpszKey);

	
}

void TInstDlg::InitLogWindow()
{

	//Log windows
	logdlg = new TLogDlg(this);
	logdlg->Create();
	m_lpPckCenter->SetLogListWnd(logdlg->GetListWnd());

	//logdlg->Show();

	m_lpPckCenter->PrintLogI(	THIS_NAME \
								THIS_VERSION \
								" is started.");

}


void TInstDlg::RefreshProgress()
{
	TCHAR		szString[MAX_PATH];
	INT			iNewPos;

	//if(0 == lpPckParams->cVarParams.dwUIProgressUpper)lpPckParams->cVarParams.dwUIProgressUpper = 1;
	iNewPos = (INT)((lpPckParams->cVarParams.dwUIProgress << 10) /
		lpPckParams->cVarParams.dwUIProgressUpper);

	SendDlgItemMessage(IDC_PROGRESS, PBM_SETPOS, (WPARAM)iNewPos, (LPARAM)0);

	if (lpPckParams->cVarParams.dwUIProgress == lpPckParams->cVarParams.dwUIProgressUpper)
		StringCchPrintf(szString, MAX_PATH, szTimerProcessedFormatString, lpPckParams->cVarParams.dwUIProgress, lpPckParams->cVarParams.dwUIProgressUpper);
	else
		StringCchPrintf(szString, MAX_PATH, szTimerProcessingFormatString, lpPckParams->cVarParams.dwUIProgress, lpPckParams->cVarParams.dwUIProgressUpper, lpPckParams->cVarParams.dwUIProgress * 100.0 / lpPckParams->cVarParams.dwUIProgressUpper, (lpPckParams->cVarParams.dwMTMemoryUsed >> 10) * 100.0 / (lpPckParams->dwMTMaxMemory >> 10));

	SetStatusBarText(3, szString);

}