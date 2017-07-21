
//////////////////////////////////////////////////////////////////////
// PckControlCenterAttribute.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�,������PCK������ݽ�������������
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.10.10
//////////////////////////////////////////////////////////////////////

#include "PckControlCenter.h"
#include "PckClass.h"



CONST	LPPCKINDEXTABLE		CPckControlCenter::GetPckIndexTable()
{

	return m_lpClassPck->GetPckIndexTable();
}

//CONST	LPPCK_PATH_NODE		CPckControlCenter::GetPckPathNode()
//{
//	return m_lpClassPck->GetPckPathNode();
//}

CONST	LPPCKHEAD			CPckControlCenter::GetPckHead()
{

	return m_lpClassPck->GetPckHead();
}

//LPCTSTR	CPckControlCenter::GetLastErrorString()
//{
//
//	return m_lpClassPck->m_lastErrorString;
//}

DWORD	CPckControlCenter::GetPckFileCount()
{

	return m_lpClassPck->GetPckFileCount();
}

PCKADDR	CPckControlCenter::GetPckSize()
{

	return m_lpClassPck->GetPckHead()->dwPckSize;
}


BOOL	CPckControlCenter::IsValidPck()
{

	return (NULL != m_lpClassPck);
}

BOOL	CPckControlCenter::GetCurrentNodeString(char* szCurrentNodePathString, LPPCK_PATH_NODE lpNode)
{
	return m_lpClassPck->GetCurrentNodeString(szCurrentNodePathString, lpNode);
}

PCKADDR	CPckControlCenter::GetPckRedundancyDataSize()
{

	return m_lpClassPck->GetPckRedundancyDataSize();
}

PCKADDR	CPckControlCenter::GetPckDataAreaSize()
{

	return m_lpClassPck->GetPckDataAreaSize();
}


BOOL	CPckControlCenter::GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpPckFileIndexTable, char *buffer, size_t sizeOfBuffer)
{

	return m_lpClassPck->GetSingleFileData(lpvoidFileRead, lpPckFileIndexTable, buffer, sizeOfBuffer);
}

char*	CPckControlCenter::GetAdditionalInfo()
{

	return m_lpClassPck->GetAdditionalInfo();
}

BOOL	CPckControlCenter::SetAdditionalInfo()
{

	return m_lpClassPck->SetAdditionalInfo();
}

LPCSTR	CPckControlCenter::GetCurrentVersionName()
{
	return cParams.lpPckVersion->getCurrentKey()->name;
}



