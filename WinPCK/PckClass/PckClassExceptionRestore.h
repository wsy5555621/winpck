//////////////////////////////////////////////////////////////////////
// PckClassExceptionRestore.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ��ȡ������Ϣ������д���ļ�����ɵ��ļ��𻵵Ļָ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.6.27
//////////////////////////////////////////////////////////////////////

BOOL GetPckBasicInfo(LPTSTR lpszFile, PCKHEAD_V2020 *lpHead, LPBYTE &lpFileIndexData, DWORD &dwPckFileIndexDataSize);
BOOL SetPckBasicInfo(PCKHEAD_V2020 *lpHead, LPBYTE lpFileIndexData, DWORD &dwPckFileIndexDataSize);

