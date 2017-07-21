//////////////////////////////////////////////////////////////////////
// PckClassHelpFunction.cpp: PCK�ļ����ܹ����е��ӹ��ܼ���
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.19
//////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4996 )

#include "PckClass.h"

BOOL CPckClass::OpenPckAndMappingRead(CMapViewFileRead *lpRead, LPCTSTR lpFileName)
{

	if(!(lpRead->OpenPck(lpFileName)))
	{
		PrintLogE(TEXT_OPENNAME_FAIL, (TCHAR*)lpFileName, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	if(!(lpRead->Mapping(m_szMapNameRead)))
	{
		PrintLogE(TEXT_CREATEMAPNAME_FAIL, (TCHAR*)lpFileName, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;

	}

	return TRUE;

}
BOOL CPckClass::OpenPckAndMappingWrite(CMapViewFileWrite *lpWrite, LPCTSTR lpFileName, DWORD dwCreationDisposition, QWORD qdwSizeToMap)
{
	if (!lpWrite->OpenPck(lpFileName, dwCreationDisposition))
	{
		PrintLogE(TEXT_OPENWRITENAME_FAIL, (TCHAR*)lpFileName, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	if(!lpWrite->Mapping(m_szMapNameWrite, qdwSizeToMap))
	{
		PrintLogE(TEXT_CREATEMAPNAME_FAIL, (TCHAR*)lpFileName, __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	return TRUE;
}


void CPckClass::AfterProcess(CMapViewFileWrite *lpWrite, PCK_ALL_INFOS &PckAllInfo, DWORD dwFileCount, PCKADDR dwAddressName, PCKADDR &dwAddress)
{//mt_dwAddress->dwAddress

	PrintLogI(TEXT_LOG_FLUSH_CACHE);

	//дPCKINDEXADDR
	PckAllInfo.PckIndexAddr.dwCryptDataAddr = dwAddressName ^ m_lpThisPckKey->IndexesEntryAddressCryptKey;
	strcpy(PckAllInfo.PckIndexAddr.szAdditionalInfo,	PCK_ADDITIONAL_INFO
														PCK_ADDITIONAL_INFO_STSM);

	//дpckIndexAddr��272�ֽ�
	lpWrite->SetFilePointer(dwAddress, FILE_BEGIN);
	dwAddress += lpWrite->Write(&PckAllInfo.PckIndexAddr, sizeof(PCKINDEXADDR));

	//дpckTail
	PckAllInfo.PckTail.dwFileCount = dwFileCount; //dwFileCount + dwOldPckFileCount;//mt_dwFileCount��ʵ��д�������ظ����Ѿ��������ȥ��
	dwAddress += lpWrite->Write(&PckAllInfo.PckTail, sizeof(PCKTAIL));

	//дpckHead
	PckAllInfo.PckHead.dwPckSize = dwAddress;
	lpWrite->SetFilePointer(0, FILE_BEGIN);
	lpWrite->Write(&PckAllInfo.PckHead, sizeof(PCKHEAD));


	//���ｫ�ļ���С��������һ��
	lpWrite->SetFilePointer(dwAddress, FILE_BEGIN);
	lpWrite->SetEndOfFile();

}