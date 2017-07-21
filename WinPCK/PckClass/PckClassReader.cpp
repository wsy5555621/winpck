//////////////////////////////////////////////////////////////////////
// PckClassReader.cpp: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �ļ�ͷ��β�Ƚṹ��Ķ�ȡ
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.6.23
//////////////////////////////////////////////////////////////////////

#include "PckClass.h"

#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )

BOOL CPckClass::ReadPckFileIndexes()
{
	CMapViewFileRead *lpRead = new CMapViewFileRead();

	if(!OpenPckAndMappingRead(lpRead, m_PckAllInfo.szFilename, m_szMapNameRead))
	{
		PrintLogE(TEXT_OPENNAME_FAIL, m_PckAllInfo.szFilename, __FILE__, __FUNCTION__, __LINE__);
		delete lpRead;
		return FALSE;
	}

	if(!AllocIndexTableAndInit(m_lpPckIndexTable, m_PckAllInfo.dwFileCount)){
		delete lpRead;
		return FALSE;
	}

	//��ʼ���ļ�
	BYTE	*lpFileBuffer;
	if(NULL == (lpFileBuffer = lpRead->View(m_PckAllInfo.dwAddressName, lpRead->GetFileSize() - m_PckAllInfo.dwAddressName)))
	{
		PrintLogE(TEXT_VIEWMAP_FAIL, __FILE__, __FUNCTION__, __LINE__);

		delete lpRead;
		return FALSE;
	}
	
	LPPCKINDEXTABLE lpPckIndexTable = m_lpPckIndexTable;
	BOOL			isLevel0;
	DWORD			byteLevelKey;
	//���ÿ���ļ�������ͷ��������DWORDѹ�����ݳ�����Ϣ
	DWORD			dwFileIndexTableCryptedDataLength[2];
	DWORD			dwFileIndexTableClearDataLength = m_PckAllInfo.lpDetectedPckVerFunc->dwFileIndexSize;
	DWORD			IndexCompressedFilenameDataLengthCryptKey[2] = { \
		m_PckAllInfo.lpDetectedPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey1, \
		m_PckAllInfo.lpDetectedPckVerFunc->cPckXorKeys->IndexCompressedFilenameDataLengthCryptKey2};

	//pck��ѹ��ʱ���ļ�����ѹ�����Ȳ��ᳬ��0x100�����Ե�
	//��ʼһ���ֽڣ����0x75����û��ѹ���������0x74����ѹ����	0x75->FILEINDEX_LEVEL0
	lpRead->SetFilePointer(m_PckAllInfo.dwAddressName, FILE_BEGIN);

	byteLevelKey = (*(DWORD*)lpFileBuffer) ^ IndexCompressedFilenameDataLengthCryptKey[0];
	isLevel0 = (m_PckAllInfo.lpDetectedPckVerFunc->dwFileIndexSize == byteLevelKey)/* ? TRUE : FALSE*/;

	if(isLevel0){

		for(DWORD i = 0;i<m_PckAllInfo.dwFileCount;i++){
			//�ȸ�������ѹ�����ݳ�����Ϣ
			memcpy(dwFileIndexTableCryptedDataLength, lpFileBuffer, 8);

			*(QWORD*)dwFileIndexTableCryptedDataLength ^= *(QWORD*)IndexCompressedFilenameDataLengthCryptKey;

			if( dwFileIndexTableCryptedDataLength[0] != dwFileIndexTableCryptedDataLength[1]){

				PrintLogE(TEXT_READ_INDEX_FAIL, __FILE__, __FUNCTION__, __LINE__);
				delete lpRead;
				return FALSE;
			}

			lpFileBuffer += 8;

			m_PckAllInfo.lpDetectedPckVerFunc->PickIndexData(&lpPckIndexTable->cFileIndex, lpFileBuffer);

			lpFileBuffer += dwFileIndexTableClearDataLength;
			++lpPckIndexTable;

		}
	}else{

		for(DWORD i = 0;i<m_PckAllInfo.dwFileCount;++i){

			memcpy(dwFileIndexTableCryptedDataLength, lpFileBuffer, 8);
			*(QWORD*)dwFileIndexTableCryptedDataLength ^= *(QWORD*)IndexCompressedFilenameDataLengthCryptKey;
			lpFileBuffer += 8;

			if( dwFileIndexTableCryptedDataLength[0] != dwFileIndexTableCryptedDataLength[1]){

				PrintLogE(TEXT_READ_INDEX_FAIL, __FILE__, __FUNCTION__, __LINE__);
				delete lpRead;
				return FALSE;
			}

			DWORD dwFileBytesRead = dwFileIndexTableClearDataLength;
			BYTE pckFileIndexBuf[MAX_INDEXTABLE_CLEARTEXT_LENGTH];

			decompress(pckFileIndexBuf, &dwFileBytesRead,
						lpFileBuffer, dwFileIndexTableCryptedDataLength[0]);


			m_PckAllInfo.lpDetectedPckVerFunc->PickIndexData(&lpPckIndexTable->cFileIndex, pckFileIndexBuf);

			lpFileBuffer += dwFileIndexTableCryptedDataLength[0];
			++lpPckIndexTable;

		}
	}

	delete lpRead;
	return TRUE;
}