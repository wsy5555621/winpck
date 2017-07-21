//////////////////////////////////////////////////////////////////////
// PckVersion.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д��
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.11
//////////////////////////////////////////////////////////////////////


#include "PckHeader.h"
#include "MapViewFile.h"

#if !defined(_PCKVERSION_H_)
#define _PCKVERSION_H_

class CPckVersion 
{
public:
	//void	CPckVersionInit();
	CPckVersion();
	//CPckVersion(LPPCK_RUNTIME_PARAMS inout);
	virtual ~CPckVersion();
	
	//��ð汾
	LPPCK_KEYS findKeyById(LPPCK_ALL_INFOS lpPckAllInfo/*, CMapViewFileRead *lpRead*/);

	LPPCK_KEYS findKeyByFileName(LPTSTR lpszPckFile);

	LPPCK_KEYS GetKey(int verID);

	LPPCK_KEYS getInitialKey();
	LPPCK_KEYS getCurrentKey();

	LPCTSTR		GetSaveDlgFilterString();

	////���ڰ汾���죬��ͬ�ṹ��ȡֵ
	//LPPCKVER	GetPckVerion();
	//DWORD	getFileClearTextSize(LPPCKFILEINDEX	lpFileIndex);
	//void	setFileClearTextSize(LPPCKFILEINDEX	lpFileIndex, DWORD val);

	//DWORD	getFileCipherTextSize(LPPCKFILEINDEX lpFileIndex);
	//void	setFileCipherTextSize(LPPCKFILEINDEX lpFileIndex, DWORD val);

protected:

	//����δ֪�汾
	//BOOL guessUnknowKey(CMapViewFileRead *lpRead, DWORD dwFileCount, LPPCKINDEXADDR lpPckIndexAddr);
	void SetPckVerion();
	//
	//void addPckVersionKeys(

public:

protected:
	//PCK_KEYS cPckKeys[10];

	PCK_KEYS *lp_defaultPckKey;
	TCHAR		szSaveDlgFilterString[1024];

	//PCKVER	cPckVerion;
};

#endif