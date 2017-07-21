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

#define define_one_PrintLog(_loglevel)	\
void CPckClass::PrintLog##_loglevel(const char *_text)\
{\
	lpPckParams->lpPckControlCenter->PrintLog##_loglevel(_text);\
}\
void CPckClass::PrintLog##_loglevel(const wchar_t *_text)\
{\
	lpPckParams->lpPckControlCenter->PrintLog##_loglevel(_text);\
}

define_one_PrintLog(I);
define_one_PrintLog(W);
define_one_PrintLog(E);
define_one_PrintLog(D);
define_one_PrintLog(N);

void CPckClass::PrintLogE(const char *_maintext, const char *_file, const char *_func, const long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(const wchar_t *_maintext, const char *_file, const char *_func, const long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(const char *_fmt, const char *_maintext, const char *_file, const char *_func, const long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_fmt, _maintext, _file, _func, _line);
}

void CPckClass::PrintLogE(const char *_fmt, const wchar_t *_maintext, const char *_file, const char *_func, const long _line)
{
	lpPckParams->lpPckControlCenter->PrintLogE(_fmt, _maintext, _file, _func, _line);
}

void CPckClass::PrintLog(const char chLevel, const char *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _maintext);
}

void CPckClass::PrintLog(const char chLevel, const wchar_t *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _maintext);
}

void CPckClass::PrintLog(const char chLevel, const char *_fmt, const char *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _fmt, _maintext);
}

void CPckClass::PrintLog(const char chLevel, const char *_fmt, const wchar_t *_maintext)
{
	lpPckParams->lpPckControlCenter->PrintLog(chLevel, _fmt, _maintext);
}


