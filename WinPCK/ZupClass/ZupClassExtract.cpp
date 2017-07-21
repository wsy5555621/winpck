//////////////////////////////////////////////////////////////////////
// ZupClassExtract.cpp: ���ڽ����������繫˾��zup�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2012.5.23
//////////////////////////////////////////////////////////////////////

#include "ZupClass.h"

#pragma warning ( disable : 4267 )

BOOL CZupClass::GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpZupFileIndexTable, char *buffer, size_t sizeOfBuffer)
{
	
	LPPCKINDEXTABLE		lpPckFileIndexTable;

	//"element\" = 0x6d656c65, 0x5c746e656d656c65
	if(0x6d656c65 == *(DWORD*)lpZupFileIndexTable->cFileIndex.szFilename){

		lpPckFileIndexTable = GetBaseFileIndex(lpZupFileIndexTable, m_lpZupIndexTable);

		DWORD	dwFileLengthDecompress2 = lpZupFileIndexTable->cFileIndex.dwFileClearTextSize;
		DWORD	dwFileLengthDecompress1 = lpPckFileIndexTable->cFileIndex.dwFileClearTextSize;

		if(0 != sizeOfBuffer && sizeOfBuffer < dwFileLengthDecompress2){
			dwFileLengthDecompress2 = sizeOfBuffer;
			if(sizeOfBuffer < dwFileLengthDecompress1)
				dwFileLengthDecompress1 = sizeOfBuffer;
		}

		char	*_cipherbuf = (char*) malloc (dwFileLengthDecompress1);

		if(NULL == _cipherbuf)
		{
			PrintLogE(TEXT_MALLOC_FAIL, __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}


		if(CPckClass::GetSingleFileData(lpvoidFileRead, lpPckFileIndexTable, _cipherbuf, dwFileLengthDecompress1))
		{

			if(PCK_BEGINCOMPRESS_SIZE < lpZupFileIndexTable->cFileIndex.dwFileClearTextSize)
			{

				if(decompress_part((BYTE*)buffer, &dwFileLengthDecompress2,
							(BYTE*)_cipherbuf + 4, dwFileLengthDecompress1 - 4, lpZupFileIndexTable->cFileIndex.dwFileClearTextSize))
				{
					if(lpZupFileIndexTable->cFileIndex.dwFileClearTextSize == lpZupFileIndexTable->cFileIndex.dwFileCipherTextSize)
					{
						memcpy(buffer, _cipherbuf + 4, dwFileLengthDecompress2);
					}
					else
					{
						char szPrintf[160];
						sprintf_s(szPrintf, 160, TEXT_UNCOMPRESSDATA_FAIL, lpZupFileIndexTable->cFileIndex.szFilename);
						PrintLogE(szPrintf);
					}
				}
			}else{
				memcpy(buffer, _cipherbuf + 4, dwFileLengthDecompress2);
			}

			free(_cipherbuf);

		}else{
			free(_cipherbuf);
			return FALSE;
		}
		

	}else{
		
		lpPckFileIndexTable = lpZupFileIndexTable;
		return CPckClass::GetSingleFileData(lpvoidFileRead, lpPckFileIndexTable, buffer, sizeOfBuffer);
	}

	return TRUE;

}