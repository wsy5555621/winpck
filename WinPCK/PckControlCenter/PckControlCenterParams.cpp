//////////////////////////////////////////////////////////////////////
// PckControlCenterParams.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
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



LPPCK_RUNTIME_PARAMS CPckControlCenter::GetParams()
{
	return &cParams;
}

//void CPckControlCenter::SetParams(LPPCK_RUNTIME_PARAMS in)
//{
//	memcpy(&cParams, in, sizeof(PCK_RUNTIME_PARAMS));
//}


//BOOL	CPckControlCenter::ConfirmErrors(LPCSTR lpszMainString, LPCSTR lpszCaption, UINT uType)
//{
//	int rtn = MessageBoxA(hWndMain, lpszMainString, lpszCaption, uType);
//	return ( IDYES == rtn || IDOK == rtn )
//}
//
//BOOL	CPckControlCenter::ConfirmErrors(LPCWSTR lpszMainString, LPCWSTR lpszCaption, UINT uType)
//{
//	int rtn = MessageBoxW(hWndMain, lpszMainString, lpszCaption, uType);
//	return ( IDYES == rtn || IDOK == rtn )
//}


