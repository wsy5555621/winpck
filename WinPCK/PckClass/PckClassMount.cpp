//////////////////////////////////////////////////////////////////////
// PckClassMount.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �й���ĳ�ʼ����
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.27
//////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4996 )

#include "PckClass.h"

BOOL CPckClass::MountPckFile(LPCTSTR	szFile)
{
	if(!DetectPckVerion(szFile, &m_PckAllInfo))
		return FALSE;

	if(!ReadPckFileIndexes())
		return FALSE;

	return TRUE;
}

void CPckClass::BuildDirTree()
{

	LPPCKINDEXTABLE lpPckIndexTable = m_lpPckIndexTable;

	for(DWORD i = 0;i<m_PckAllInfo.dwFileCount;++i)
	{
		//����Ŀ¼
		AddFileToNode(&m_RootNode, lpPckIndexTable);
		++lpPckIndexTable;
	}

}
