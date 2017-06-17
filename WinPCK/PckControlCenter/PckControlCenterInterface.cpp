
//////////////////////////////////////////////////////////////////////
// PckControlCenterInterface.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�,������PCK������ݽ�������������
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.19
//////////////////////////////////////////////////////////////////////

#include "PckControlCenter.h"
#include "PckClass.h"

CONST int	CPckControlCenter::GetListCurrentHotItem()
{
	return cParams.iListHotItem;
}

void		CPckControlCenter::SetListCurrentHotItem(int _val)
{
	cParams.iListHotItem = _val;
}

CONST BOOL	CPckControlCenter::GetListInSearchMode()
{
	return m_isSearchMode;
}

void		CPckControlCenter::SetListInSearchMode(BOOL _val)
{
	m_isSearchMode = _val;
}

LPCTSTR	CPckControlCenter::GetSaveDlgFilterString()
{
	return cParams.lpPckVersion->GetSaveDlgFilterString();
}

BOOL CPckControlCenter::isSupportAddFileToPck()
{
	switch(m_emunFileFormat)
	{
	case FMTPCK_PCK:
		return TRUE;
		break;

	case FMTPCK_ZUP:
		return FALSE;
		break;
	}
	return FALSE;
}
