//////////////////////////////////////////////////////////////////////
// threadproc.cpp: WinPCK �����̲߳���
// ѹ������ѹ�����µȴ���ʱ���̣�����̵߳��� 
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
// 2012.10.10
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4311 )
//#pragma warning ( disable : 4018 )
#pragma warning ( disable : 4005 )

#include "tlib.h"
#include "resource.h"
#include "globals.h"
#include "winmain.h"
#include <process.h>


//VOID EnumAndDeleteIndex(LPPCK_PATH_NODE lpNode);
VOID GetPckFileNameBySource(TCHAR *dst, TCHAR *src, BOOL isDirectory);

_inline void TInstDlg::EnbaleButtons(int ctlExceptID, BOOL bEnbale)
{

	WORD wToolBarTextList[] = {	IDS_STRING_OPEN,
								IDS_STRING_CLOSE,
								IDS_STRING_NEW,
								0,
								IDS_STRING_UNPACK_SELECTED,
								IDS_STRING_UNPACK_ALL,
								0,
								IDS_STRING_ADD,
								IDS_STRING_REBUILD,
								IDS_STRING_INFO,
								IDS_STRING_SEARCH,
								IDS_STRING_COMPRESS_OPT,
								0,
								IDS_STRING_ABOUT,
								IDS_STRING_EXIT	};


	WORD ctlMenuIDs[] = {ID_MENU_OPEN,
						ID_MENU_CLOSE,
						ID_MENU_NEW,
						1,
						ID_MENU_UNPACK_SELECTED,
						ID_MENU_UNPACK_ALL,
						1,
						ID_MENU_ADD,
						ID_MENU_REBUILD,
						ID_MENU_INFO,
						ID_MENU_SEARCH,
						ID_MENU_COMPRESS_OPT,
						0};

	WORD ctlRMenuIDs[] = {ID_MENU_VIEW,
						ID_MENU_UNPACK_SELECTED,
						ID_MENU_RENAME,
						ID_MENU_DELETE,
						ID_MENU_ATTR, 
						0};

	TBBUTTONINFO tbbtninfo = {0};
	tbbtninfo.cbSize = sizeof(TBBUTTONINFO);
	tbbtninfo.dwMask = TBIF_IMAGE | /*TBIF_TEXT | */TBIF_LPARAM;

	WORD	*pctlIDs = ctlMenuIDs;
	WORD	*pctlStrings = wToolBarTextList;
	
	while(0 != *pctlIDs)
	{
		
		if(ctlExceptID == *pctlIDs)
		{
			SendDlgItemMessage(IDC_TOOLBAR, TB_GETBUTTONINFO, ctlExceptID, (LPARAM)&tbbtninfo);

			if(bEnbale)
			{
				tbbtninfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
				tbbtninfo.iImage = (int)tbbtninfo.lParam;
				tbbtninfo.pszText = GetLoadStr(*pctlStrings);

				EnableButton(ctlExceptID, bEnbale);

			}else{

				tbbtninfo.dwMask = TBIF_IMAGE | TBIF_TEXT | TBIF_LPARAM;
				tbbtninfo.lParam = tbbtninfo.iImage;
				tbbtninfo.iImage = 16;
				tbbtninfo.pszText = GetLoadStr(IDS_STRING_CANCEL);

			}

			SendDlgItemMessage(IDC_TOOLBAR, TB_SETBUTTONINFO, ctlExceptID, (LPARAM)&tbbtninfo);

		}else{
			EnableButton(*pctlIDs, bEnbale);
		}

		pctlStrings++;
		pctlIDs++;
	}


	HMENU	hMenu = ::GetMenu(hWnd);
	HMENU	hSubMenu = ::GetSubMenu(hMenu, 0);
	pctlIDs = ctlMenuIDs;

	while(1 != *pctlIDs)
	{
		::EnableMenuItem(hSubMenu, *pctlIDs, bEnbale ? MF_ENABLED : MF_GRAYED);
		pctlIDs++;
	}

	hSubMenu = ::GetSubMenu(hMenu, 1);

	pctlIDs++;
	while(1 != *pctlIDs)
	{
		::EnableMenuItem(hSubMenu, *pctlIDs, bEnbale ? MF_ENABLED : MF_GRAYED);
		pctlIDs++;
	}

	hSubMenu = ::GetSubMenu(hMenu, 2);

	pctlIDs++;
	while(0 != *pctlIDs)
	{
		::EnableMenuItem(hSubMenu, *pctlIDs, bEnbale ? MF_ENABLED : MF_GRAYED);
		pctlIDs++;
	}

	hMenu = ::LoadMenu(TApp::GetInstance(), (LPCTSTR)IDR_MENU_RCLICK);
	hSubMenu = ::GetSubMenu(hMenu, 0);

	pctlIDs = ctlRMenuIDs;

	while(0 != *pctlIDs)
	{
		::EnableMenuItem(hSubMenu, *pctlIDs, bEnbale ? MF_ENABLED : MF_GRAYED);
		pctlIDs++;
	}

	return;
}


VOID TInstDlg::UpdatePckFile(VOID *pParam)
{

	TInstDlg	*pThis = (TInstDlg*)pParam;
	BOOL		bDeleteClass = !pThis->m_lpPckCenter->IsValidPck();
	TCHAR		szFilenameToSave[MAX_PATH];
	LPPCK_RUNTIME_PARAMS	lpParams = pThis->lpPckParams;
	BOOL		*lpbThreadRunning = &lpParams->cVarParams.bThreadRunning;
	TCHAR		szPrintf[320];
	double		t1, t2;//����ʱ��
	
	if(bDeleteClass)
		pThis->m_lpPckCenter->New();

	*szFilenameToSave = 0;

	if(bDeleteClass)
	{
		//��ʱû�д��ļ�����ʱ�Ĳ����൱���½��ĵ�
		if(1 == pThis->m_DropFileCount)
		{
			GetPckFileNameBySource(szFilenameToSave, *pThis->m_lpszFilePath, FALSE);
		}

		//ѡ�񱣴���ļ���
		int nSelectFilter = pThis->SaveFile(szFilenameToSave, pThis->m_lpPckCenter->GetSaveDlgFilterString());
		if(!nSelectFilter)
		{
			//pThis->DeleteClass();
			pThis->m_lpPckCenter->Close();

			_endthread();
		}

		//�趨Ŀ��pck�İ汾
		pThis->m_lpPckCenter->SetPckVersion(nSelectFilter);

		_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWING), wcsrchr(szFilenameToSave, TEXT('\\')) + 1);
	}else{
		_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWING), wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);
	}

	//��ʼ��ʱ
	t1=GetTickCount();

	pThis->EnbaleButtons(ID_MENU_ADD, FALSE);

	
	pThis->SetStatusBarText(4, szPrintf);
	
	pThis->m_lpPckCenter->Reset(pThis->m_DropFileCount);
	*lpbThreadRunning = TRUE;

	pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

	if(pThis->m_lpPckCenter->UpdatePckFile(szFilenameToSave, pThis->m_lpszFilePath, pThis->m_DropFileCount, pThis->m_currentNodeOnShow))
	{
		if(*lpbThreadRunning)
		{
			//��ʱ����
			t2=GetTickCount() - t1;
			_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWOK), t2);

			pThis->m_lpPckCenter->PrintLogN(szPrintf);
			//pThis->SetStatusBarText(4, szPrintf);
		}else{

			pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
		}

		if(bDeleteClass)
		{

			pThis->OpenPckFile(szFilenameToSave, TRUE);
		}else{
			pThis->OpenPckFile(pThis->m_Filename, TRUE);
		}

	}else{

		pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);

		if(bDeleteClass)
		{
			pThis->m_lpPckCenter->Close();
		}
	}

	pThis->EnbaleButtons(ID_MENU_ADD, TRUE);

	pThis->KillTimer(WM_TIMER_PROGRESS_100);
	pThis->RefreshProgress();

	if((!(*lpbThreadRunning)) && pThis->bGoingToExit)
	{
		pThis->bGoingToExit = FALSE;
		pThis->SendMessage(WM_CLOSE, 0, 0);
	}

	*lpbThreadRunning = FALSE;

	//��ԭDrop״̬
	pThis->m_DropFileCount = 0;
	DragAcceptFiles(pThis->hWnd,TRUE);

	//���ϱ���
	// ��ӡ����
	// pck����ԭ���ļ� %d ��\r\n
	// �¼����ļ� %d ���������ظ��ļ��� %d ��\r\n
	// ʹ��ԭ���ݵ�ַ %d ���������ݵ�ַ %d ��\r\n
	// ͨ���ɵ���pck�ļ��� %d ���ļ�\r\n
	
	if(0 != lpParams->cVarParams.dwPrepareToAddFileCount){

		_stprintf_s(szPrintf, 
			TEXT("�˸��¹����������£�\r\n")
			TEXT("PCK ����ԭ���ļ����� %d\r\n")
			TEXT("�ƻ������ļ����� %d\r\n")
			TEXT("ʵ�ʸ����ļ����� %d\r\n")
			TEXT("�����ļ����� %d\r\n")
			TEXT("�����ļ���ʹ������������������ %d\r\n")
			TEXT("�����ļ���ʹ������������������ %d\r\n")
			TEXT("δ�����ļ����� %d\r\n")
			TEXT("���º� PCK �����ļ����� %d"),
			lpParams->cVarParams.dwOldFileCount,
			lpParams->cVarParams.dwPrepareToAddFileCount,
			lpParams->cVarParams.dwChangedFileCount,
			lpParams->cVarParams.dwDuplicateFileCount,
			lpParams->cVarParams.dwUseNewDataAreaInDuplicateFileSize,
			lpParams->cVarParams.dwChangedFileCount - lpParams->cVarParams.dwUseNewDataAreaInDuplicateFileSize,
			lpParams->cVarParams.dwPrepareToAddFileCount - lpParams->cVarParams.dwChangedFileCount,
			lpParams->cVarParams.dwFinalFileCount);

		pThis->MessageBox(szPrintf, TEXT("���±���"), MB_OK | MB_ICONINFORMATION);

		pThis->m_lpPckCenter->PrintLogI(szPrintf);
	}

	_endthread();

}

VOID TInstDlg::RenamePckFile(VOID *pParam)
{

	TInstDlg	*pThis = (TInstDlg*)pParam;
	TCHAR		szPrintf[64];
	double		t1, t2;//����ʱ��

	_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWING), wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);

	//��ʼ��ʱ
	t1=GetTickCount();

	pThis->EnbaleButtons(ID_MENU_RENAME, FALSE);
	
	pThis->SetStatusBarText(4, szPrintf);

	pThis->m_lpPckCenter->Reset();
	pThis->lpPckParams->cVarParams.bThreadRunning = TRUE;

	pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

	if(pThis->m_lpPckCenter->RenameFilename())
	{

		//��ʱ����
		t2=GetTickCount() - t1;
		_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWOK), t2);

		pThis->SetStatusBarText(4, szPrintf);

		pThis->OpenPckFile(pThis->m_Filename, TRUE);

	}else{
		pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
		pThis->m_lpPckCenter->Close();
	}

	pThis->EnbaleButtons(ID_MENU_RENAME, TRUE);

	pThis->KillTimer(WM_TIMER_PROGRESS_100);
	pThis->RefreshProgress();

	if(pThis->bGoingToExit)
	{
		pThis->bGoingToExit = FALSE;
		pThis->SendMessage(WM_CLOSE, 0, 0);
	}

	pThis->lpPckParams->cVarParams.bThreadRunning = FALSE;


	_endthread();

}

VOID TInstDlg::RebuildPckFile(VOID	*pParam)
{

	TInstDlg	*pThis = (TInstDlg*)pParam;

	BOOL		bDeleteClass = !pThis->m_lpPckCenter->IsValidPck();

	TCHAR		szFilenameToSave[MAX_PATH];
	TCHAR		szPrintf[288];

	BOOL		*lpbThreadRunning = &pThis->lpPckParams->cVarParams.bThreadRunning;

	double		t1, t2;//����ʱ��

	if(bDeleteClass)
	{
		//bDeleteClass = TRUE;
		if(!pThis->OpenPckFile())
		{
			return;
			_endthread();
		}
	}

	_tcscpy_s(szFilenameToSave, pThis->m_Filename);

	TCHAR		*lpszFileTitle = wcsrchr(szFilenameToSave, TEXT('\\')) + 1;
	_tcscpy(lpszFileTitle, TEXT("Rebuild_"));
	_tcscat_s(szFilenameToSave, wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);

	BOOL(CPckControlCenter::*RebuildOrRecompressPckFile)(LPTSTR);

	if (IDYES == ::MessageBoxA(pThis->hWnd, "�Ƿ�ʹ��������ѹ����\r\n�� �� ��ѹ������\r\n�� �� ֱ���ؽ�", "�����ؽ�", MB_YESNO)) {
		RebuildOrRecompressPckFile = &CPckControlCenter::RecompressPckFile;
	} else {
		RebuildOrRecompressPckFile = &CPckControlCenter::RebuildPckFile;
	}

	//ѡ�񱣴���ļ���
	int nSelectFilter = pThis->SaveFile(szFilenameToSave, pThis->m_lpPckCenter->GetSaveDlgFilterString(), pThis->m_lpPckCenter->GetPckVersion());
	if(!nSelectFilter)
	{
		_endthread();
	}

	pThis->m_lpPckCenter->SetPckVersion(nSelectFilter);

	//��ʼ��ʱ
	t1=GetTickCount();

	pThis->EnbaleButtons(ID_MENU_REBUILD, FALSE);

	_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_REBUILDING), wcsrchr(szFilenameToSave, TEXT('\\')) + 1);
	pThis->SetStatusBarText(4, szPrintf);

	pThis->m_lpPckCenter->Reset(pThis->m_lpPckCenter->GetPckFileCount());
	*lpbThreadRunning = TRUE;

	pThis->SetTimer(WM_TIMER_PROGRESS_100, 100, NULL);

	if((pThis->m_lpPckCenter->*RebuildOrRecompressPckFile)(szFilenameToSave))
	{
		if(*lpbThreadRunning)
		{
			//��ʱ����
			t2=GetTickCount() - t1;
			_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_REBUILDOK), t2);

			pThis->m_lpPckCenter->PrintLogN(szPrintf);
			//pThis->SetStatusBarText(4, szPrintf);
		}else{
			pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
		}

	}else{

		pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);

	}
	
	pThis->EnbaleButtons(ID_MENU_REBUILD, TRUE);

	if(bDeleteClass)
	{
		ListView_DeleteAllItems(pThis->GetDlgItem(IDC_LIST));
		pThis->m_lpPckCenter->Close();
	}

	pThis->KillTimer(WM_TIMER_PROGRESS_100);
	pThis->RefreshProgress();

	if((!(*lpbThreadRunning)) && pThis->bGoingToExit)
	{
		pThis->bGoingToExit = FALSE;
		pThis->SendMessage(WM_CLOSE, 0, 0);
	}

	*lpbThreadRunning = FALSE;

	_endthread();

}

VOID TInstDlg::CreateNewPckFile(VOID	*pParam)
{
	TInstDlg	*pThis = (TInstDlg*)pParam;

	BOOL		bDeleteClass = !pThis->m_lpPckCenter->IsValidPck();
	TCHAR		szPathToCompress[MAX_PATH], szFilenameToSave[MAX_PATH];

	TCHAR		szPrintf[64];
	BOOL		*lpbThreadRunning = &pThis->lpPckParams->cVarParams.bThreadRunning;

	double		t1, t2;//����ʱ��

	//ѡ��Ŀ¼
	if(!pThis->BrowseForFolderByPath(pThis->m_CurrentPath))
		_endthread();

	_tcscpy_s(szPathToCompress, pThis->m_CurrentPath);

	GetPckFileNameBySource(szFilenameToSave, szPathToCompress, TRUE);
	
	if(bDeleteClass)
		pThis->m_lpPckCenter->New();

	//ѡ�񱣴���ļ���
	int nSelectFilter = pThis->SaveFile(szFilenameToSave, pThis->m_lpPckCenter->GetSaveDlgFilterString());
	if(!nSelectFilter)
		_endthread();

	//�趨Ŀ��pck�İ汾
	pThis->m_lpPckCenter->SetPckVersion(nSelectFilter);

	//��ʼ��ʱ
	t1=GetTickCount();

	pThis->EnbaleButtons(ID_MENU_NEW, FALSE);

	_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_COMPING), wcsrchr(szFilenameToSave, TEXT('\\')) + 1);
	pThis->SetStatusBarText(4, szPrintf);

	pThis->m_lpPckCenter->Reset();
	*lpbThreadRunning = TRUE;

	pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

	if(pThis->m_lpPckCenter->CreatePckFile(szFilenameToSave, szPathToCompress)){

		if(*lpbThreadRunning){
			//��ʱ����
			t2=GetTickCount() - t1;
			_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_COMPOK), t2);

			pThis->m_lpPckCenter->PrintLogN(szPrintf);
			//pThis->SetStatusBarText(4, szPrintf);
		}else{
			pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
		}

	}else{

		pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
	}

	pThis->EnbaleButtons(ID_MENU_NEW, TRUE);

	if(bDeleteClass)
		pThis->m_lpPckCenter->Close();

	pThis->KillTimer(WM_TIMER_PROGRESS_100);
	pThis->RefreshProgress();

	if((!(*lpbThreadRunning)) && pThis->bGoingToExit){

		pThis->bGoingToExit = FALSE;
		pThis->SendMessage(WM_CLOSE, 0, 0);
	}

	*lpbThreadRunning = FALSE;

	//��ԭDrop״̬
	pThis->m_DropFileCount = 0;
	_endthread();

}

VOID TInstDlg::ToExtractAllFiles(VOID	*pParam)
{
	TInstDlg	*pThis = (TInstDlg*)pParam;

	TCHAR		szPrintf[64];

	BOOL		*lpbThreadRunning = &pThis->lpPckParams->cVarParams.bThreadRunning;

	double		t1, t2;//����ʱ��

	//��ʼ��ʱ
	t1=GetTickCount();

	pThis->EnbaleButtons(ID_MENU_UNPACK_ALL, FALSE);

	_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_EXPING), wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);
	pThis->SetStatusBarText(4, szPrintf);

	pThis->m_lpPckCenter->Reset(pThis->m_lpPckCenter->m_lpPckRootNode->child->dwFilesCount);
	*lpbThreadRunning = TRUE;

	pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

	if(pThis->m_lpPckCenter->ExtractFiles(&pThis->m_lpPckCenter->m_lpPckRootNode, 1))
	{
		//��ʱ����
		t2=GetTickCount() - t1;
		_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_EXPOK), t2);

		pThis->SetStatusBarText(4, szPrintf);

	}else{
		pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
	}

	pThis->EnbaleButtons(ID_MENU_UNPACK_ALL, TRUE);
	
	pThis->KillTimer(WM_TIMER_PROGRESS_100);
	pThis->RefreshProgress();

	if((!(*lpbThreadRunning)) && pThis->bGoingToExit)
	{
		pThis->bGoingToExit = FALSE;
		pThis->SendMessage(WM_CLOSE, 0, 0);
	}

	*lpbThreadRunning = FALSE;

	_endthread();

}

VOID TInstDlg::ToExtractSelectedFiles(VOID	*pParam)
{
	TInstDlg	*pThis = (TInstDlg*)pParam;

	LVITEM item;

	BOOL		*lpbThreadRunning = &pThis->lpPckParams->cVarParams.bThreadRunning;
	LPDWORD		lpdwUIProgressUpper = &pThis->lpPckParams->cVarParams.dwUIProgressUpper;

	pThis->m_lpPckCenter->Reset(0);
	

	HWND	hList = pThis->GetDlgItem(IDC_LIST);

	double		t1, t2;//����ʱ��

	LPPCK_PATH_NODE		*lpNodeToShow, *lpNodeToShowPtr;
	LPPCKINDEXTABLE		*lpIndexToShow, *lpIndexToShowPtr;

	UINT uiSelectCount = ListView_GetSelectedCount(hList);

	if(0 != uiSelectCount)
	{
		if(NULL != (lpNodeToShow = (LPPCK_PATH_NODE*) malloc (sizeof(LPPCK_PATH_NODE) * uiSelectCount)))
		{
			TCHAR		szPrintf[64];

			lpIndexToShow = (LPPCKINDEXTABLE*)lpNodeToShow;

			//ȡlpNodeToShow
			
			int	nCurrentItemCount = ListView_GetItemCount(hList);

			item.mask = LVIF_PARAM | LVIF_STATE;
			item.iSubItem = 0;
			item.stateMask = LVIS_SELECTED;		// get all state flags

			lpNodeToShowPtr = lpNodeToShow;
			lpIndexToShowPtr = lpIndexToShow;

			uiSelectCount = 0;

			if(pThis->m_lpPckCenter->GetListInSearchMode())
			{
				for(item.iItem=1;item.iItem<nCurrentItemCount;item.iItem++)
				{
					ListView_GetItem(hList, &item);

					if( item.state & LVIS_SELECTED )
					{
						*lpIndexToShowPtr = (LPPCKINDEXTABLE)item.lParam;
						(*lpdwUIProgressUpper)++;
						lpIndexToShowPtr++;

						uiSelectCount++;
					}
				}
			}else{
				for(item.iItem=1;item.iItem<nCurrentItemCount;item.iItem++)
				{
					ListView_GetItem(hList, &item);

					if( item.state & LVIS_SELECTED )
					{
						*lpNodeToShowPtr = (LPPCK_PATH_NODE)item.lParam;
						if(NULL == (*lpNodeToShowPtr)->child)
						{
							(*lpdwUIProgressUpper)++;
						}else{
							(*lpdwUIProgressUpper) += (*lpNodeToShowPtr)->child->dwFilesCount;
						}
						lpNodeToShowPtr++;

						uiSelectCount++;
					}
				}
			}

			if(0 == uiSelectCount)_endthread();

			//��ʼ��ʱ
			t1=GetTickCount();

			*lpbThreadRunning = TRUE;

			pThis->EnbaleButtons(ID_MENU_UNPACK_SELECTED, FALSE);

			_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_EXPING), wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);
			pThis->SetStatusBarText(4, szPrintf);

			pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

			lpNodeToShowPtr = lpNodeToShow;

			if(pThis->m_lpPckCenter->GetListInSearchMode())
			{
				if(pThis->m_lpPckCenter->ExtractFiles(lpIndexToShow, uiSelectCount))
				{
					//��ʱ����
					t2=GetTickCount() - t1;
					_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_EXPOK), t2);

					pThis->SetStatusBarText(4, szPrintf);
				}else{
					pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
				}
			}else{
				if(pThis->m_lpPckCenter->ExtractFiles(lpNodeToShow, uiSelectCount))
				{
					//��ʱ����
					t2=GetTickCount() - t1;
					_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_EXPOK), t2);

					pThis->SetStatusBarText(4, szPrintf);
				}else{
					pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
				}

			}

			free(lpNodeToShow);

			pThis->EnbaleButtons(ID_MENU_UNPACK_SELECTED, TRUE);

			pThis->KillTimer(WM_TIMER_PROGRESS_100);
			pThis->RefreshProgress();

			if((!(*lpbThreadRunning)) && pThis->bGoingToExit)
			{
				pThis->bGoingToExit = FALSE;
				pThis->SendMessage(WM_CLOSE, 0, 0);
			}
		}
	}

	*lpbThreadRunning = FALSE;

	_endthread();

}


VOID TInstDlg::DeleteFileFromPckFile(VOID	*pParam)
{
	TInstDlg	*pThis = (TInstDlg*)pParam;

	LVITEM item;

	BOOL		*lpbThreadRunning = &pThis->lpPckParams->cVarParams.bThreadRunning;

	pThis->m_lpPckCenter->Reset();
	

	HWND	hList = pThis->GetDlgItem(IDC_LIST);

	double		t1, t2;//����ʱ��

	LPPCK_PATH_NODE		lpNodeToShow;//, *lpNodeToShowPtr;
	LPPCKINDEXTABLE		lpIndexToShow;//, *lpIndexToShowPtr;

	UINT uiSelectCount = ListView_GetSelectedCount(hList);

	if(0 != uiSelectCount)
	{

		TCHAR		szPrintf[64];

		int	nCurrentItemCount = ListView_GetItemCount(hList);

		item.mask = LVIF_PARAM | LVIF_STATE;
		item.iSubItem = 0;
		item.stateMask = LVIS_SELECTED;		// get all state flags

		uiSelectCount = 0;


		if(pThis->m_lpPckCenter->GetListInSearchMode())
		{
			for(item.iItem=1;item.iItem<nCurrentItemCount;item.iItem++)
			{
				ListView_GetItem(hList, &item);

				if( item.state & LVIS_SELECTED )
				{

					lpIndexToShow = (LPPCKINDEXTABLE)item.lParam;
					lpIndexToShow->bSelected = TRUE;

					uiSelectCount++;
				}
			}
		}else{
			for(item.iItem=1;item.iItem<nCurrentItemCount;item.iItem++)
			{
				ListView_GetItem(hList, &item);

				if( item.state & LVIS_SELECTED )
				{

					lpNodeToShow = (LPPCK_PATH_NODE)item.lParam;

					if(NULL == lpNodeToShow->child)
					{
						lpNodeToShow->lpPckIndexTable->bSelected = TRUE;
					}else{
						//EnumAndDeleteIndex(lpNodeToShow);
						pThis->m_lpPckCenter->DeleteNode(lpNodeToShow);
					}

					uiSelectCount++;
				}
			}
		}

		if(0 == uiSelectCount)_endthread();

		//��ʼ��ʱ
		t1=GetTickCount();

		*lpbThreadRunning = TRUE;

		pThis->EnbaleButtons(ID_MENU_DELETE, FALSE);

		_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWING), wcsrchr(pThis->m_Filename, TEXT('\\')) + 1);
		pThis->SetStatusBarText(4, szPrintf);

		pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);


		if(pThis->m_lpPckCenter->RenameFilename())
		{
			//��ʱ����
			t2=GetTickCount() - t1;
			_stprintf_s(szPrintf, GetLoadStr(IDS_STRING_RENEWOK), t2);

			pThis->SetStatusBarText(4, szPrintf);

			pThis->OpenPckFile(pThis->m_Filename, TRUE);

		}else{
			pThis->SetStatusBarText(4, TEXT_PROCESS_ERROR);
		}

		pThis->EnbaleButtons(ID_MENU_DELETE, TRUE);

		pThis->KillTimer(WM_TIMER_PROGRESS_100);
		pThis->RefreshProgress();

		if(pThis->bGoingToExit)
		{
			pThis->bGoingToExit = FALSE;
			pThis->SendMessage(WM_CLOSE, 0, 0);
		}

	}

	*lpbThreadRunning = FALSE;

	_endthread();

}

//�������Դ�ļ����Ƴ�Ԥ�����pck�ļ���
VOID GetPckFileNameBySource(TCHAR *dst, TCHAR *src, BOOL isDirectory)
{
	int szPathToCompressLen;
	_tcscpy(dst, src);

	if(isDirectory)
	{
		if((szPathToCompressLen = lstrlen(dst)) > 13 && 0 == lstrcmp(dst + szPathToCompressLen - 10, TEXT(".pck.files")))
		{
			*(dst + szPathToCompressLen - 10) = 0;
		}
	}

	_tcscat(dst, TEXT(".pck"));
}