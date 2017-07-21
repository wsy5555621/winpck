//////////////////////////////////////////////////////////////////////
// guirelated.cpp: WinPCK �����̲߳���
// �����ʼ��
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4312 )
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4005 )

#include "winmain.h"
//#include "globals.h"
#include <strsafe.h>

#ifdef _USE_CUSTOMDRAW_
typedef enum {R_NOTHINGS=0, R_NORMAL, R_SELECT, R_SEL_NOFOCUS} redrawmode;
redrawmode redraw;
HICON				hIconList[3];
HBRUSH				hBrushs[HB_COUNT];

#endif

int initArgumentException(int code);

void TInstDlg::initCommctrls()
{
	//��ʼ��listview
	//LPTSTR	lpszID_LIST[] =	{TEXT("�ļ���"), TEXT("ʵ���С"), TEXT("ѹ����С"), TEXT("ѹ����")/*, TEXT("δ֪2")*/};
	LPTSTR	lpszID_LIST[] =	{GetLoadStr(IDS_STRING_LISTHEAD1), GetLoadStr(IDS_STRING_LISTHEAD2), GetLoadStr(IDS_STRING_LISTHEAD3), GetLoadStr(IDS_STRING_LISTHEAD4)};
	int		iID_LIST_cx[] = {558, 80, 80, 60, -1};
	int		ifmt[] =		{LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT};
	InitListView(GetDlgItem(IDC_LIST), lpszID_LIST, iID_LIST_cx, ifmt);

#ifdef _USE_CUSTOMDRAW_
	//ListView���õĻ�ˢ
	COLORREF		colorHB[] = {	FILL_COLOR_GREEN, 
									FILL_COLOR_GRAY,
									FILL_COLOR_SELECTED/*,
									FILL_COLOR_NOFOCUS*/};
	for(int i = 0;i<HB_COUNT;i++)
	{
		hBrushs[i] = CreateSolidBrush(colorHB[i]);
	}

	//ListView���õ�ͼ��
	hIconList[0] = LoadIcon(TApp::GetInstance(), MAKEINTRESOURCE(IDI_ICON_DIR)); 
	hIconList[1] = LoadIcon(TApp::GetInstance(), MAKEINTRESOURCE(IDI_ICON_FILE)); 
	hIconList[2] = LoadIcon(TApp::GetInstance(), MAKEINTRESOURCE(IDI_ICON_FILE)); 

#endif

	//���ý������ķ�Χ
	SendDlgItemMessage(IDC_PROGRESS, PBM_SETRANGE,(WPARAM)0, MAKELPARAM(0,1024) ); 

	//StatusBar,�ļ������ļ������ļ���С�����ȣ�״̬
	//int		iStatusWidth[] = {150, 250, 320, 530, 1024};
	int		iStatusWidth[] = {150, 260, 350, 590, 1224};
	SendDlgItemMessage(IDC_STATUS, SB_SETPARTS, 5, (LPARAM)iStatusWidth);

	//SetStatusBarText(0, TEXT("textures123456.pck"));
	//SetStatusBarText(1, TEXT("��С: 9999999999"));
	//SetStatusBarText(2, TEXT("����: 9999999"));
	//SetStatusBarText(3, TEXT("����: 9999999/9999999 100.0% ����:100.0%"));

}

void TInstDlg::initParams()
{

	//��ʼ������
	m_lpPckCenter = new CPckControlCenter( hWnd );

	lpPckParams = m_lpPckCenter->GetParams();

	*m_szStrToSearch = 0;			//���ҵ�����
	//bThreadRunning = FALSE;			//�߳����б��
	bGoingToExit = FALSE;			//�Ƿ�׼���˳�

	//��ȡ��ǰ������CPU����
	SYSTEM_INFO sysinfo;								//����ṹ����
	GetSystemInfo(&sysinfo);							//��ȡ��ǰ��������Ϣ
	lpPckParams->dwMTThread = sysinfo.dwNumberOfProcessors;	//��ȡ��ǰ������CPU����;
	lpPckParams->dwMTMaxThread = sysinfo.dwNumberOfProcessors + ((sysinfo.dwNumberOfProcessors + (sysinfo.dwNumberOfProcessors & 1))>>1);

	lpPckParams->dwCompressLevel = 9;
	lpPckParams->dwMTMaxMemory = MT_MAX_MEMORY;

	m_DropFileCount = 0;

	//������
	m_isSearchWindow = FALSE;

	//Timer
	StringCchCopy(szTimerProcessingFormatString, 64, GetLoadStr(IDS_STRING_TIMERING));
	StringCchCopy(szTimerProcessedFormatString, 64, GetLoadStr(IDS_STRING_TIMEROK));


#ifdef _WIN64
	m_hCursorOld = (HCURSOR)GetClassLong(hWnd, GCLP_HCURSOR);
#else
	m_hCursorOld = (HCURSOR)GetClassLong(hWnd, GCL_HCURSOR);
#endif


	m_hCursorAllow = LoadCursor(TApp::GetInstance(), MAKEINTRESOURCE(IDC_CURSOR_DROP));
	m_hCursorNo = LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO));

}

void TInstDlg::initToolbar()
{

	//��ʼ��������
	m_imageList = ImageList_Create(32, 32, ILC_COLOR32, 1, 20);

	WORD	wToolBarIDsList[] = {	IDI_ICON_OPEN,
									IDI_ICON_NEW,
									IDI_ICON_CLOSE,
									IDI_ICON_APP,
									IDI_ICON_SAVE,
									IDI_ICON_SAVEALL,
									IDI_ICON_APP,
									IDI_ICON_ADD,
									IDI_ICON_REBUILD,
									IDI_ICON_APP,
									IDI_ICON_EDIT,
									IDI_ICON_SEARCH,
									IDI_ICON_OPT,
									IDI_ICON_APP,
									IDI_ICON_INFO,
									IDI_ICON_EXIT,
									IDI_ICON_STOP,
									0	};

	WORD	wToolBarTextList[] = {	IDS_STRING_OPEN,
									IDS_STRING_NEW,
									IDS_STRING_CLOSE,
									0,
									IDS_STRING_UNPACK_SELECTED,
									IDS_STRING_UNPACK_ALL,
									0,
									IDS_STRING_ADD,
									IDS_STRING_REBUILD,
									0,
									IDS_STRING_INFO,
									IDS_STRING_SEARCH,
									IDS_STRING_COMPRESS_OPT,
									0,
									IDS_STRING_ABOUT,
									IDS_STRING_EXIT	};

	WORD	wToolBarCommandID[] = {	ID_MENU_OPEN,
									ID_MENU_NEW,
									ID_MENU_CLOSE,
									0,
									ID_MENU_UNPACK_SELECTED,
									ID_MENU_UNPACK_ALL,
									0,
									ID_MENU_ADD,
									ID_MENU_REBUILD,
									0,
									ID_MENU_INFO,
									ID_MENU_SEARCH,
									ID_MENU_COMPRESS_OPT,
									0,
									ID_MENU_ABOUT,
									ID_MENU_EXIT	};

	WORD	*lpIDs = wToolBarIDsList;

	while(0 != *lpIDs)
	{

		HICON hiconItem = LoadIcon(TApp::GetInstance(), MAKEINTRESOURCE(*lpIDs)); 
		ImageList_AddIcon(m_imageList, hiconItem); 
		DestroyIcon(hiconItem); 

		lpIDs++;
	}

	SendDlgItemMessage(IDC_TOOLBAR, TB_SETIMAGELIST, 0, (LPARAM)m_imageList);
	SendDlgItemMessage(IDC_TOOLBAR, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

	TBBUTTON tbbtn;
	memset(&tbbtn, 0, sizeof(TBBUTTON));
	tbbtn.fsState = TBSTATE_ENABLED;
	
	lpIDs = wToolBarCommandID;
	WORD	*lpIDsText = wToolBarTextList;

	for(int i=0;i<16;i++)
	{
		if(0 != *lpIDs)
		{

			//SendDlgItemMessage(IDC_TOOLBAR, TB_ADDSTRING, (WPARAM) 0/*TApp::GetInstance()*/, (LPARAM) GetLoadStr(*lpIDsText)/*MAKELONG (*lpIDsText, 0)*/);
			tbbtn.fsStyle = TBSTYLE_BUTTON;
			tbbtn.iString = (INT_PTR)GetLoadStr(*lpIDsText);
			tbbtn.idCommand = *lpIDs;
			tbbtn.iBitmap = i;
		}else{
			SendDlgItemMessage(IDC_TOOLBAR, TB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("\0"));
			tbbtn.fsStyle = TBSTYLE_SEP;
			tbbtn.iString = -1;
			tbbtn.idCommand = 0;
			tbbtn.iBitmap = -1;
		}

		SendDlgItemMessage(IDC_TOOLBAR, TB_ADDBUTTONS, 1, (LPARAM)&tbbtn);

		lpIDsText++;
		lpIDs++;

	}
	
	::SetWindowPos(GetDlgItem(IDC_TOOLBAR), NULL, 0, 0, 0, 0,SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW); 

	//TBBUTTONINFO tbbtninfo = {0};
	//tbbtninfo.cbSize = sizeof(TBBUTTONINFO);
	//tbbtninfo.dwMask = TBIF_IMAGE | /*TBIF_TEXT | */TBIF_LPARAM | TBIF_SIZE;

	//SendDlgItemMessage(IDC_TOOLBAR, TB_GETBUTTONINFO, /*ID_MENU_OPEN*/ID_MENU_EXIT, (LPARAM)&tbbtninfo);


	//tbbtninfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
	//tbbtninfo.iImage = 16;
	//tbbtninfo.pszText = TEXT("asdasdas");
	//SendDlgItemMessage(IDC_TOOLBAR, TB_SETBUTTONINFO, /*ID_MENU_OPEN*/ID_MENU_EXIT, (LPARAM)&tbbtninfo);

}

#ifdef UNICODE
	#define CommandLineToArgv CommandLineToArgvW
#else
	#define CommandLineToArgv CommandLineToArgvA
#endif

#ifndef UNICODE
PCHAR*  CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
{
    PCHAR* argv;
    PCHAR  _argv;
    ULONG   len;
    ULONG   argc;
    CHAR   a;
    ULONG   i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
        i + (len+2)*sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( a = CmdLine[i] ) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
#endif

void TInstDlg::initArgument()
{
LPTSTR* argv;

	__try{

		//��ʼ�����·��

		memset(&m_PathDirs, 0, sizeof(m_PathDirs));
		*m_PathDirs.lpszDirNames = m_PathDirs.szPaths;

		*m_CurrentPath = 0;
		GetCurrentDirectory(MAX_PATH, m_CurrentPath);

		//ȡcommandline
		
		INT argc;
		argv = CommandLineToArgv(GetCommandLine(), &argc);

		//wchar_t	sargc[256];
		//StringCchPrintf(sargc, 256, L"argc = %d", argc);

		//MessageBox(sargc);
		//MessageBox(argv[0]);
		//MessageBox(argv[1]);

		StringCchCopy(m_MyFileName, MAX_PATH, argv[0]);

		if(argc > 1){
			StringCchCopy(m_Filename, MAX_PATH, argv[1]);
		}
		LocalFree(argv);


		//��ʼ��·��
		if(2 > argc)
		{
			StringCchCopy(m_Filename, MAX_PATH, m_CurrentPath);
			StringCchCat(m_Filename, MAX_PATH, TEXT("\\"));
		}else{
			OpenPckFile(m_Filename);
		}

	}__except(initArgumentException(GetExceptionCode())){
		__try{
			LocalFree(argv);
		}__except(EXCEPTION_EXECUTE_HANDLER){
			//MessageBoxA("bbb");;
		}
	}
}

int initArgumentException(int code)
{

    switch(code)
    {
    case EXCEPTION_ACCESS_VIOLATION :
		MessageBoxA(NULL, "�洢�����쳣 �� initArgument", NULL, MB_OK);
		return EXCEPTION_EXECUTE_HANDLER;
		//return EXCEPTION_CONTINUE_EXECUTION;

	default:
		return 0;
	}

}