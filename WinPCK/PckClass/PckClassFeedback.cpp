//////////////////////////////////////////////////////////////////////
// PckClassFeedback.cpp: ��Ϣ��������Ϣ��������Ե� 
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.19
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"
#include "PckControlCenter.h"

void CPckClass::PrintLogI(char *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogI(_text);
}

void CPckClass::PrintLogW(char *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogW(_text);
}

void CPckClass::PrintLogE(char *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_text);
}


void CPckClass::PrintLogD(char *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogD(_text);
}


void CPckClass::PrintLogI(wchar_t *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogI(_text);
}

void CPckClass::PrintLogW(wchar_t *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogW(_text);
}


void CPckClass::PrintLogD(wchar_t *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogD(_text);
}


void CPckClass::PrintLogE(wchar_t *_text)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_text);
}

void CPckClass::PrintLogE(char *_maintext, char *_file, char *_func, long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(wchar_t *_maintext, char *_file, char *_func, long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(char *_fmt, char *_maintext, char *_file, char *_func, long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_fmt, _maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(char *_fmt, wchar_t *_maintext, char *_file, char *_func, long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_fmt, _maintext, _file, _func, _line);
}

void CPckClass::PrintLog(char chLevel, char *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _maintext);
}

void CPckClass::PrintLog(char chLevel, wchar_t *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _maintext);
}

void CPckClass::PrintLog(char chLevel, char *_fmt, char *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _fmt, _maintext);
}

void CPckClass::PrintLog(char chLevel, char *_fmt, wchar_t *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _fmt, _maintext);
}


