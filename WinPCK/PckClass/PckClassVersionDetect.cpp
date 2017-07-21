//////////////////////////////////////////////////////////////////////
// PckClassVersionDetect.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �汾��صļ���Լ����ݵ�д�롢��ȡ
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.6.27
//////////////////////////////////////////////////////////////////////

#include "MapViewFile.h"
#include "PckClass.h"

#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4146 )

#include "PckClassVersionDetectFunctionDefinitions.h"

/*
 **	IndexesEntryAddressCryptKey >= 0x80000000 ->0xffffffff80000000
 ** else
 **	0x7fffffff -> 0x000000007fffffff
 ** 0xA8937462 -> 0xffffffffA8937462
 **
*/
const PCK_KEYS CPckClass::cPckKeys[PCK_VERSION_NUMS] = \
	{	{PCK_VERSION_ZX,	TEXT("����"),		PCK_V2020, 0x20002, 0x4DCA23EF, 0x56A089B7, 0xFDFDFEEE, 0x00000000A8937462, 0xF00DBEEF, 0xA8937462, 0xF1A43653, 0x7fffff00}, \
		{PCK_VERSION_SDS,	TEXT("ʥ��ʿ"),		PCK_V2020, 0x20002, 0x4DCA23EF, 0x56A089B7, 0x7b2a7820, 0x0000000062a4f9e1, 0xa75dc142, 0x62a4f9e1, 0x3520c3d5, 0x7fffff00}, \
		{PCK_VERSION_XAJH,	TEXT("Ц������"),	PCK_V2030, 0x20003, 0x5edb34f0, 0x00000000, 0x7b2a7820, 0x49ab7f1d33c3eddb, 0xa75dc142, 0x62a4f9e1, 0x3520c3d5, 0xffffff00}, \
		{PCK_VERSION_ZXNEW, TEXT("����3��ʮ��"),PCK_V2031, 0x20003, 0x4DCA23EF, 0x00000000, 0xFDFDFEEE, 0xffffffffA8937462, 0xF00DBEEF, 0xA8937462, 0xF1A43653, 0x7fffff00}, \
	};

const PCK_VERSION_FUNC CPckClass::cPckVersionFunc[PCK_VERSION_NUMS] = \
	{	{&cPckKeys[PCK_VERSION_ZX],		sizeof(PCKHEAD_V2020), sizeof(PCKTAIL_V2020), sizeof(PCKFILEINDEX_V2020), PickIndexData_V2020, FillHeadData_V2020, FillTailData_V2020, FillIndexData_V2020}, \
		{&cPckKeys[PCK_VERSION_SDS],	sizeof(PCKHEAD_V2020), sizeof(PCKTAIL_V2020), sizeof(PCKFILEINDEX_V2020), PickIndexData_V2020, FillHeadData_V2020, FillTailData_V2020, FillIndexData_V2020}, \
		{&cPckKeys[PCK_VERSION_XAJH],	sizeof(PCKHEAD_V2030), sizeof(PCKTAIL_V2030), sizeof(PCKFILEINDEX_V2030), PickIndexData_V2030, FillHeadData_V2030, FillTailData_V2030, FillIndexData_V2030}, \
		{&cPckKeys[PCK_VERSION_ZXNEW],	sizeof(PCKHEAD_V2031), sizeof(PCKTAIL_V2031), sizeof(PCKFILEINDEX_V2031), PickIndexData_V2031, FillHeadData_V2030, FillTailData_V2031, FillIndexData_V2031}, \
	};

//���ɽ��汣���ļ�ʱ��Filter�ı�
void CPckClass::BuildSaveDlgFilterString()
{
	*szSaveDlgFilterString = 0;
	TCHAR szPrintf[256];

	for(int i=0;i<PCK_VERSION_NUMS;i++){

		_stprintf_s(szPrintf, TEXT("%sPCK�ļ�(*.pck)|*.pck|"), cPckKeys[i].name);
		_tcscat_s(szSaveDlgFilterString, szPrintf);
	}

	TCHAR *lpszStr = szSaveDlgFilterString;
	while(*lpszStr){

		if(TEXT('|') == *lpszStr)
			*lpszStr = 0;
		++lpszStr;
	}

	*lpszStr = 0;
}

LPCTSTR CPckClass::GetSaveDlgFilterString()
{
	return szSaveDlgFilterString;
}

const PCK_KEYS* CPckClass::GetPckVersion()
{
	return m_PckAllInfo.lpDetectedPckVerFunc->cPckXorKeys;
}

void CPckClass::SetPckVersion(int verID)
{
	if(0 <= verID && PCK_VERSION_NUMS > verID)
		m_PckAllInfo.lpSaveAsPckVerFunc = &cPckVersionFunc[verID];
	else
		PrintLogW(TEXT_INVALID_VERSION);
}
//
//void CPckClass::PrintInvalidVersionDebugInfo()
//{
//
//	if(NULL == (m_lpThisPckKey = lpPckParams->lpPckVersion->findKeyById(&m_PckAllInfo))){
//
//		m_lpThisPckKey = lpPckParams->lpPckVersion->getInitialKey();
//
//		//��ӡ��ϸԭ��
//		char szPrintf[1024];
//
//	#ifdef PCKV202
//		StringCchPrintfA(szPrintf, 1024, "������Ϣ��"
//						"HEAD->dwHeadCheckHead = 0x%08x, "
//						"HEAD->dwPckSize = 0x%08x, "
//						"HEAD->dwHeadCheckTail = 0x%08x, "
//						"TAIL->dwFileCount = 0x%08x, "
//						"TAIL->dwVersion = 0x%08x, "
//						"INDEX->dwIndexTableCheckHead = 0x%08x, "
//						"INDEX->dwCryptDataAddr = 0x%08x, "
//						"INDEX->dwIndexTableCheckTail = 0x%08x",\
//						m_PckAllInfo.PckHead.dwHeadCheckHead, \
//						m_PckAllInfo.PckHead.dwPckSize, \
//						m_PckAllInfo.PckHead.dwHeadCheckTail, \
//						m_PckAllInfo.PckTail.dwFileCount, \
//						m_PckAllInfo.PckTail.dwVersion, \
//						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckHead, \
//						m_PckAllInfo.PckIndexAddr.dwCryptDataAddr, \
//						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckTail);
//
//
//
//	#elif defined PCKV203
//		StringCchPrintfA(szPrintf, 1024, "������Ϣ��"
//						"HEAD->dwHeadCheckHead = 0x%08x, "
//						"HEAD->dwPckSize = 0x%016llx, "
//						"TAIL->dwFileCount = 0x%08x, "
//						"TAIL->dwVersion = 0x%08x, "
//						"INDEX->dwIndexTableCheckHead = 0x%08x, "
//						"INDEX->dwCryptDataAddr = 0x%016llx, "
//						"INDEX->dwIndexTableCheckTail = 0x%08x", \
//						m_PckAllInfo.PckHead.dwHeadCheckHead, \
//						m_PckAllInfo.PckHead.dwPckSize, \
//						m_PckAllInfo.PckTail.dwFileCount, \
//						m_PckAllInfo.PckTail.dwVersion, \
//						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckHead, \
//						m_PckAllInfo.PckIndexAddr.dwCryptDataAddr, \
//						m_PckAllInfo.PckIndexAddr.dwIndexTableCheckTail);
//
//
//	#endif
//
//		PrintLogE(TEXT_UNKNOWN_PCK_FILE);
//		PrintLogD(szPrintf);
//		
//		delete lpcReadfile;
//		return FALSE;
//	}
//
//}


//��ȡ�ļ�ͷ��βȷ��pck�ļ��汾�����ذ汾ID
BOOL CPckClass::DetectPckVerion(LPCTSTR lpszPckFile, LPPCK_ALL_INFOS pckAllInfo)
{
	PCKHEAD_V2020 cPckHead;
	DWORD		dwTailVals[4];

	int iDetectedPckID = -1;
	//��ȡ�ļ�ͷ
	CMapViewFileRead *lpRead = new CMapViewFileRead();

	if(!lpRead->OpenPck(lpszPckFile))
	{
		PrintLogE(TEXT_OPENNAME_FAIL, lpszPckFile, __FILE__, __FUNCTION__, __LINE__);
		goto dect_err;
	}

	if(!lpRead->Read(&cPckHead, sizeof(PCKHEAD_V2020)))
	{
		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);
		goto dect_err;
	}

	lpRead->SetFilePointer(-((QWORD)(sizeof(DWORD) * 4)), FILE_END);

	if(!lpRead->Read(&dwTailVals, sizeof(DWORD) * 4))
	{
		PrintLogE(TEXT_READFILE_FAIL, __FILE__, __FUNCTION__, __LINE__);
		goto dect_err;
	}

	if(0x20003 == dwTailVals[3]){
		if(0 == dwTailVals[1])
			dwTailVals[1] = dwTailVals[0];

		for(int i=0;i<PCK_VERSION_NUMS;i++){
			if(	(cPckKeys[i].Version == 0x20003) && 
				(cPckKeys[i].TailVerifyKey2 == dwTailVals[1]) &&
				(cPckKeys[i].HeadVerifyKey1 == cPckHead.dwHeadCheckHead) ) {

					iDetectedPckID = i;
					break;
			}
		}
	}else{

		for(int i=0;i<PCK_VERSION_NUMS;i++){
			if(	(cPckKeys[i].Version == dwTailVals[3]) && 
				(cPckKeys[i].TailVerifyKey2 == dwTailVals[1]) &&
				(cPckKeys[i].HeadVerifyKey1 == cPckHead.dwHeadCheckHead) && 
				(cPckKeys[i].HeadVerifyKey2 == cPckHead.dwHeadCheckTail) ) {

					iDetectedPckID = i;
					break;
			}
		}
	}

	//��ȡPCKINDEXADDR����֤
	if(-1 == iDetectedPckID)goto dect_err;

	PCKVERSION ver = cPckKeys[iDetectedPckID].VersionId;
	BOOL		isFoundVer = FALSE;

	switch (ver) {
	case PCK_V2020:

		define_get_pckAllInfo_by_version(2020, iDetectedPckID);
		break;
	case PCK_V2030:

		define_get_pckAllInfo_by_version(2030, iDetectedPckID);
		break;
	case PCK_V2031:

		define_get_pckAllInfo_by_version(2031, iDetectedPckID);
		break;
	}

	if(!isFoundVer)goto dect_err;

	pckAllInfo->lpSaveAsPckVerFunc = pckAllInfo->lpDetectedPckVerFunc = &cPckVersionFunc[iDetectedPckID];

	delete lpRead;
	return TRUE;

dect_err:
	iDetectedPckID = -1;
	delete lpRead;
	return FALSE;
}

define_one_FillHeadData_by_version(2020);
define_one_FillHeadData_by_version(2030);

define_one_FillTailData_by_version(2020);
define_one_FillTailData_by_version(2030);
define_one_FillTailData_by_version(2031);

//���ݴ�param -> lpTail
define_one_FillIndexData_by_version(2020);
define_one_FillIndexData_by_version(2030);
define_one_FillIndexData_by_version(2031);

//���ݴ�lpIndex -> param
define_one_PickIndexData_by_version(2020);
define_one_PickIndexData_by_version(2030);
define_one_PickIndexData_by_version(2031);
