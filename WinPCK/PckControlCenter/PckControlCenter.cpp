//////////////////////////////////////////////////////////////////////
// PckControlCenter.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
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
	
CPckControlCenter::CPckControlCenter()
{
	init();
	Reset();
}

CPckControlCenter::CPckControlCenter(HWND hWnd)
{
	m_hWndMain = hWnd;

	init();
	Reset();
}

CPckControlCenter::~CPckControlCenter()
{
}

void CPckControlCenter::init()
{
	m_lpClassPck = NULL;
	m_isSearchMode = FALSE;
	m_lpPckRootNode = NULL;

	hasRestoreData = FALSE;
	m_lpPckFileIndexData = NULL;

	m_LogListCount = 0;

	m_emunFileFormat = FMTPCK_UNKNOWN;

	//cParams.lpPckVersion = new CPckVersion();
	cParams.lpPckControlCenter = this;
}

void CPckControlCenter::Reset(DWORD dwUIProgressUpper)
{
	memset(&cParams.cVarParams, 0, sizeof(PCK_VARIETY_PARAMS));
	cParams.cVarParams.dwUIProgressUpper = dwUIProgressUpper;

	//if(NULL != m_lpClassPck)*m_lpClassPck->m_lastErrorString = 0;

}