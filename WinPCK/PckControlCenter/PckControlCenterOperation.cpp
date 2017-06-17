
//////////////////////////////////////////////////////////////////////
// PckControlCenterOperation.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
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
#include "ZupClass.h"
#include <strsafe.h>


void CPckControlCenter::New()
{
	Close();
	m_lpClassPck = new CPckClass(&cParams);

}

BOOL CPckControlCenter::Open(LPCTSTR lpszFile)
{

	//�ж��ļ���ʽ
	FMTPCK emunFileFormat;
	size_t	nFileLength;

	StringCchLength(lpszFile, MAX_PATH, &nFileLength);

	if(0 == lstrcmpi(lpszFile + nFileLength - 4, TEXT(".pck"))){
		emunFileFormat = FMTPCK_PCK;
	}else if(0 == lstrcmpi(lpszFile + nFileLength - 4, TEXT(".zup"))){
		emunFileFormat = FMTPCK_ZUP;
	}

	while(1){

		Close();
		switch(emunFileFormat)
		{
		case FMTPCK_PCK:
			m_lpClassPck = new CPckClass(&cParams);
			break;

		case FMTPCK_ZUP:
			m_lpClassPck = new CZupClass(&cParams);
			break;

		default:
			m_lpClassPck = new CPckClass(&cParams);
			break;
		}

		//*m_lpClassPck->m_lastErrorString = 0;

		PrintLog(LOG_FLAG_INFO, TEXT_LOG_OPENFILE, (TCHAR *)lpszFile);

		if(m_lpClassPck->Init(lpszFile)){
			
			DeleteRestoreData();

			m_lpPckRootNode = m_lpClassPck->GetPckPathNode();

			m_emunFileFormat = emunFileFormat;

			//�򿪳ɹ���ˢ�±���
			SendMessage(m_hWndMain, WM_FRESH_MAIN_CAPTION, 1, 0);
			return TRUE;

		}else{
			

			if(hasRestoreData){
				if(IDYES == MessageBoxA(m_hWndMain, TEXT_ERROR_OPEN_AFTER_UPDATE, TEXT_ERROR, MB_YESNO | MB_ICONHAND)){

					RestoreData(lpszFile);
				}else{
					//Close();
					return FALSE;
				}

			}else{

				//Close();
				return FALSE;
			}

		}
	}

}

void CPckControlCenter::SetPckVersion(int verID)
{
	m_lpClassPck->SetPckVersion(verID);
}

void CPckControlCenter::Close()
{
	if(NULL != m_lpClassPck)
	{
		PrintLog(LOG_FLAG_INFO, TEXT_LOG_CLOSEFILE);

		delete m_lpClassPck;
		m_lpClassPck = NULL;

		m_lpPckRootNode = NULL;
	}

	m_isSearchMode = FALSE;
	m_emunFileFormat = FMTPCK_UNKNOWN;

	//�ر��ļ���ˢ�±���
	SendMessage(m_hWndMain, WM_FRESH_MAIN_CAPTION, 0, 0);
}

void CPckControlCenter::CreateRestoreData()
{

	if(m_lpClassPck->GetPckBasicInfo(m_lpszFile4Restore, &m_PckHead, m_lpPckFileIndexData, m_dwPckFileIndexDataSize))
	{
		hasRestoreData = TRUE;
	}else{
		PrintLogE(TEXT_ERROR_GET_RESTORE_DATA);
	}

}

void CPckControlCenter::RestoreData(LPCTSTR lpszFile)
{
	if(hasRestoreData)
	{
		if(0 == lstrcmpi(m_lpszFile4Restore, lpszFile)){

			if(!m_lpClassPck->SetPckBasicInfo(&m_PckHead, m_lpPckFileIndexData, m_dwPckFileIndexDataSize))
				PrintLogE(TEXT_ERROR_RESTORING);
			else
				PrintLogI(TEXT_LOG_RESTOR_OK);
		}
	}
	DeleteRestoreData();
}

void CPckControlCenter::DeleteRestoreData()
{
	if(NULL != m_lpPckFileIndexData){

		free(m_lpPckFileIndexData);
		m_lpPckFileIndexData = NULL;
	}

	hasRestoreData = FALSE;
}

VOID	CPckControlCenter::RenameIndex(LPPCK_PATH_NODE lpNode, char* lpszReplaceString)
{
	m_lpClassPck->RenameIndex(lpNode, lpszReplaceString);
}

VOID	CPckControlCenter::RenameIndex(LPPCKINDEXTABLE lpIndex, char* lpszReplaceString)
{
	m_lpClassPck->RenameIndex(lpIndex, lpszReplaceString);
}

BOOL	CPckControlCenter::RenameNode(LPPCK_PATH_NODE lpNode, char* lpszReplaceString)
{
	return m_lpClassPck->RenameNode(lpNode, lpszReplaceString);
}

//����pck�ļ�
BOOL	CPckControlCenter::UpdatePckFile(LPTSTR szPckFile, TCHAR (*lpszFilePath)[MAX_PATH], int nFileCount, LPPCK_PATH_NODE lpNodeToInsert)
{
	return m_lpClassPck->UpdatePckFile(szPckFile, lpszFilePath, nFileCount, lpNodeToInsert);
}

//�������ļ�
BOOL	CPckControlCenter::RenameFilename()
{
	//����һ�����ݣ�����ʧ�ܺ�ָ�
	CreateRestoreData();

	return m_lpClassPck->RenameFilename();
}

//�ؽ�pck�ļ�
BOOL	CPckControlCenter::RebuildPckFile(LPTSTR szRebuildPckFile)
{
	return m_lpClassPck->RebuildPckFile(szRebuildPckFile);
}

//�½�pck�ļ�--
BOOL	CPckControlCenter::CreatePckFileMT(LPTSTR szPckFile, LPTSTR szPath)
{
	return m_lpClassPck->CreatePckFileMT(szPckFile, szPath);
}

//��ѹ�ļ�
BOOL	CPckControlCenter::ExtractFiles(LPPCKINDEXTABLE *lpIndexToExtract, int nFileCount)
{
	return m_lpClassPck->ExtractFiles(lpIndexToExtract, nFileCount);
}

BOOL	CPckControlCenter::ExtractFiles(LPPCK_PATH_NODE *lpNodeToExtract, int nFileCount)
{
	return m_lpClassPck->ExtractFiles(lpNodeToExtract, nFileCount);
}

//ɾ��һ���ڵ�
VOID	CPckControlCenter::DeleteNode(LPPCK_PATH_NODE lpNode)
{
	//����һ�����ݣ�����ʧ�ܺ�ָ�
	CreateRestoreData();

	return m_lpClassPck->DeleteNode(lpNode);
}

