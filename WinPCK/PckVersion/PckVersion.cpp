//////////////////////////////////////////////////////////////////////
// PckVersion.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д��
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.11
//////////////////////////////////////////////////////////////////////


#include "PckVersion.h"


#ifdef PCKV202
																			//ver	HeadChkHead HeadChkTail  TblChkHead IndexValue	TblChkTail	
PCK_KEYS cPckKeys[10] = {	{"����", TEXT("����"),	PCK_VERSION_ZX,			0x20002, 0x4DCA23EF, 0x56A089B7, 0xFDFDFEEE, 0xA8937462, 0xF00DBEEF, 0xA8937462, 0xF1A43653},\
							{"ʥ��ʿ",TEXT("ʥ��ʿ"), PCK_VERSION_SDS,		0x20002, 0x4DCA23EF, 0x56A089B7, 0x7b2a7820, 0x62a4f9e1, 0xa75dc142, 0x62a4f9e1, 0x3520c3d5},\
							{0},\
						};
#elif defined PCKV203
PCK_KEYS cPckKeys[10] = {	{"Ц������",TEXT("Ц������"), PCK_VERSION_XAJH,0x20003, 0x5edb34f0, 0x00000000, 0x7b2a7820, 0x49ab7f1d33c3eddb, 0xa75dc142, 0x62a4f9e1, 0x3520c3d5},\
							{0},\
						};
#elif defined PCKV203ZX
PCK_KEYS cPckKeys[10] = { { "����", TEXT("����"),	PCK_VERSION_ZX,			0x20003, 0x4DCA23EF, 0x00000000, 0xFDFDFEEE, 0xA8937462, 0xF00DBEEF, 0xA8937462, 0xF1A43653 },\
						{0},\
						};
#endif

CPckVersion::CPckVersion()
{
	lp_defaultPckKey = cPckKeys;
	*szSaveDlgFilterString = 0;
	LPPCK_KEYS lpKeyPtr = cPckKeys;

	TCHAR szPrintf[256];

	while(lpKeyPtr->id){
		StringCchPrintf(szPrintf, 256, TEXT("%sPCK�ļ�(*.pck)|*.pck|"), lpKeyPtr->tname);
		StringCchCat(szSaveDlgFilterString, 1024, szPrintf);

		++lpKeyPtr;
	}

	//StringCchCat(szSaveDlgFilterString, 1024, TEXT("|"));

	TCHAR *lpszStr = szSaveDlgFilterString;
	while(*lpszStr){

		if(TEXT('|') == *lpszStr)
			*lpszStr = 0;
		++lpszStr;
	}

	*lpszStr = 0;

}

CPckVersion::~CPckVersion()
{
	
}

LPCTSTR CPckVersion::GetSaveDlgFilterString()
{
	return szSaveDlgFilterString;
}

LPPCK_KEYS CPckVersion::getInitialKey()
{
	return cPckKeys;
}


LPPCK_KEYS CPckVersion::getCurrentKey()
{
	return lp_defaultPckKey;
}

LPPCK_KEYS CPckVersion::GetKey(int verID)
{
	return &cPckKeys[verID];
}

LPPCK_KEYS CPckVersion::findKeyByFileName(LPTSTR lpszPckFile)
{
	//PCKHEAD cPckHead;
	//PCKINDEXADDR cPckIndexAddr;
	//PCKTAIL cPckTail;
	PCK_ALL_INFOS pckAllInfo;

	CMapViewFileRead *lpRead = new CMapViewFileRead();

	if(!lpRead->OpenPck(lpszPckFile))
	{
		delete lpRead;
		return NULL;
	}

	if(!lpRead->Read(&pckAllInfo.PckHead, sizeof(PCKHEAD)))
	{
		delete lpRead;
		return NULL;
	}

	lpRead->SetFilePointer(-PCK_TAIL_OFFSET, FILE_END);

	if(!lpRead->Read(&pckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR)))
	{
		delete lpRead;
		return NULL;
	}

	if(!lpRead->Read(&pckAllInfo.PckTail, sizeof(PCKTAIL)))
	{
		delete lpRead;
		return NULL;
	}

	delete lpRead;

	return findKeyById(&pckAllInfo);
}

LPPCK_KEYS CPckVersion::findKeyById(LPPCK_ALL_INFOS lpPckAllInfo/*, CMapViewFileRead *lpRead*/)
{

	BOOL isFound = FALSE;
	//����ƥ����
	LPPCK_KEYS lpKeyPtr = cPckKeys;

	while(lpKeyPtr->id){

		if(	lpKeyPtr->HeadVerifyKey1 == lpPckAllInfo->PckHead.dwHeadCheckHead &&
#if defined PCKV202 || defined PCKV203ZX
			lpKeyPtr->HeadVerifyKey2 == lpPckAllInfo->PckHead.dwHeadCheckTail &&
#endif
			lpKeyPtr->TailVerifyKey1 == lpPckAllInfo->PckIndexAddr.dwIndexTableCheckHead &&
			lpKeyPtr->TailVerifyKey2 == lpPckAllInfo->PckIndexAddr.dwIndexTableCheckTail &&
			lpKeyPtr->Version == lpPckAllInfo->PckTail.dwVersion){

				isFound = TRUE;

				lp_defaultPckKey = lpKeyPtr;
				//memcpy(cPckKeys, lpKeyPtr, sizeof(PCK_KEYS));

				//if(cPckKeys->Version != lpPckTail->dwVersion)
				//	cPckKeys->Version = lpPckTail->dwVersion;

				break;
			//return lpKeyPtr;
		}

		++lpKeyPtr;
	}
	////����ļ�ͷ
	//if(PCKHEAD_VERIFY_HEAD != lpPckHead->dwHeadCheckHead || PCKHEAD_VERIFY_TAIL != lpPckHead->dwHeadCheckTail)
	//{
	//	//lstrcpy(m_lastErrorString, TEXT("PCK�ļ�ͷ�𻵣�"));
	//	//delete lpcReadfile;

	//	return NULL;
	//}


	////����ļ�β	FILEINDEX_VERIFY_HEAD		0xFDFDFEEE	#define	FILEINDEX_VERIFY_TAIL		0xF00DBEEF
	//if(FILEINDEX_VERIFY_HEAD != lpPckIndexAddr->dwIndexTableCheckHead || FILEINDEX_VERIFY_TAIL != lpPckIndexAddr->dwIndexTableCheckTail)
	//{
	//	//if(!ConfirmErrors(TEXT("PCK�ļ�β�𻵣��Ƿ������"), NULL, MB_YESNO | MB_ICONQUESTION)){
	//	//	lstrcpy(m_lastErrorString, TEXT("PCK�ļ�β�𻵣�"));
	//		
	//		//delete lpcReadfile;
	//		return NULL;
	//	//}
	//}

	//if(!isFound){
	//	cPckKeys->HeadVerifyKey1 = lpPckHead->dwHeadCheckHead;
	//	cPckKeys->HeadVerifyKey2 = lpPckHead->dwHeadCheckTail;
	//	cPckKeys->TailVerifyKey1 = lpPckIndexAddr->dwIndexTableCheckHead;
	//	cPckKeys->TailVerifyKey2 = lpPckIndexAddr->dwIndexTableCheckTail;
	//	cPckKeys->Version = lpPckTail->dwVersion;
	//	isFound = guessUnknowKey(lpRead, lpPckTail->dwFileCount, lpPckIndexAddr);

	//}
	if(isFound){
		//strcat(cPckKeys->name, "-��ǰ����");
		return lp_defaultPckKey;// = cPckKeys;
	}else{
		lp_defaultPckKey = cPckKeys;
		return NULL;
	}

}


//BOOL CPckVersion::guessUnknowKey(CMapViewFileRead *lpRead, DWORD dwFileCount, LPPCKINDEXADDR lpPckIndexAddr)
//{
//	////m_dwAddressName start at = filesize - 280 - 0x114 * dwFileCount
//	////m_PckIndexAddr.dwIndexValue
//	////m_dwAddressName = m_PckIndexAddr.dwIndexValue ^ m_lpThisPckKey->IndexesEntryAddressCryptKey;
//	//DWORD	dwAddress280 = lpRead->GetFileSize() - PCK_TAIL_OFFSET;
//	//DWORD	dwSearchAreaSize = INDEXTABLE_CLEARTEXT_LENGTH * dwFileCount;
//	//DWORD	dwSearchAreaStart = dwAddress280 - dwSearchAreaSize;
//
//	//DWORD	dwGuessAssistKey1 = dwAddress280 ^ dwSearchAreaStart;
//
//	//DWORD	dwPaternKey1 = lpPckIndexAddr->dwIndexValue ^ 0xef2b4;
//
//	return FALSE;
//}

