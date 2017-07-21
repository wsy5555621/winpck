//////////////////////////////////////////////////////////////////////
// PckClassExceptionRestore.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ����д���ļ�����ɵ��ļ��𻵵Ļָ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.6.27
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable : 4244 )
//#pragma warning ( disable : 4311 )

#include "PckClass.h"

BOOL CPckClass::GetPckBasicInfo(LPTSTR lpszFile, PCKHEAD_V2020 *lpHead, LPBYTE &lpFileIndexData, DWORD &dwPckFileIndexDataSize)
{

	BYTE	*lpFileBuffer;

	dwPckFileIndexDataSize = m_PckAllInfo.qwPckSize - m_PckAllInfo.dwAddressName;

	//lstrcpy(lpszFile, m_Filename);
#ifdef UNICODE
	memcpy(lpszFile, m_PckAllInfo.szFilename, MAX_PATH * 2);
#else
	memcpy(lpszFile, m_PckAllInfo.szFilename, MAX_PATH);
#endif
	
	if(NULL == (lpFileIndexData = (LPBYTE) malloc (dwPckFileIndexDataSize))){

		PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	CMapViewFileRead	*lpcReadfile = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpcReadfile, m_PckAllInfo.szFilename, m_szMapNameRead))
	{
		free(lpFileIndexData);
		delete lpcReadfile;
		return FALSE;
	}

	if(NULL == (lpFileBuffer = lpcReadfile->View(m_PckAllInfo.dwAddressName, dwPckFileIndexDataSize)))
	{
		PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);

		free(lpFileIndexData);
		delete lpcReadfile;
		
		return FALSE;
	}

	memcpy(lpFileIndexData, lpFileBuffer, dwPckFileIndexDataSize);

	delete lpcReadfile;

	memcpy(lpHead, m_PckAllInfo.lpSaveAsPckVerFunc->FillHeadData(&m_PckAllInfo), sizeof(PCKHEAD_V2020));

	return TRUE;

}

//���޸��ļ����Ȳ������pck�ļ����޷���ȡʱ���ָ�pck����������
BOOL CPckClass::SetPckBasicInfo(PCKHEAD_V2020 *lpHead, LPBYTE lpFileIndexData, DWORD &dwPckFileIndexDataSize)
{
	BYTE	*lpFileBuffer;

	CMapViewFileWrite	*lpcWritefile = new CMapViewFileWrite(m_PckAllInfo.lpSaveAsPckVerFunc->cPckXorKeys->dwMaxSinglePckSize);

	if(!OpenPckAndMappingWrite(lpcWritefile, m_PckAllInfo.szFilename, OPEN_EXISTING, lpHead->dwPckSize)){

		delete lpcWritefile;
		return FALSE;
	}

	lpcWritefile->SetFilePointer(0, FILE_BEGIN);
	lpcWritefile->Write(lpHead, sizeof(PCKHEAD_V2020));

	if(NULL == (lpFileBuffer = lpcWritefile->View(lpHead->dwPckSize - dwPckFileIndexDataSize, dwPckFileIndexDataSize)))
	{

		PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpcWritefile;
		return FALSE;
	}

	memcpy(lpFileBuffer, lpFileIndexData, dwPckFileIndexDataSize);

	delete lpcWritefile;
	return TRUE;

}
