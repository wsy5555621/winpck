//////////////////////////////////////////////////////////////////////
// PckStructs.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// ͷ�ļ�
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.5.13
//////////////////////////////////////////////////////////////////////

#include "PckConf.h"

#if !defined(_PCKSTRUCTS_H_)
#define _PCKSTRUCTS_H_


typedef enum _FMTPCK
{
	FMTPCK_PCK					= 0,
	FMTPCK_ZUP					= 1,
	FMTPCK_CUP					= 2,
	FMTPCK_UNKNOWN				= 0x7fffffff
}FMTPCK;

typedef enum _PCKVER
{
	PCK_V2020,
	PCK_V2030,
	PCK_V2031
}PCKVERSION;

typedef struct _PCK_KEYS
{
	int			id;
	TCHAR		name[64];
	PCKVERSION	VersionId;
	DWORD		Version;
	//head
	DWORD		HeadVerifyKey1;
	DWORD		HeadVerifyKey2;
	//tail
	DWORD		TailVerifyKey1;
	QWORD		IndexesEntryAddressCryptKey;
	DWORD		TailVerifyKey2;
	//index
	DWORD		IndexCompressedFilenameDataLengthCryptKey1;
	DWORD		IndexCompressedFilenameDataLengthCryptKey2;
	//pkx
	DWORD		dwMaxSinglePckSize;
}PCK_KEYS, *LPPCK_KEYS;

typedef struct _PCK_VERSION_FUNC {

	const		PCK_KEYS*	cPckXorKeys;
	//ͷ��size
	size_t		dwHeadSize;
	//β��size
	size_t		dwTailSize;
	//fileindex��size
	size_t		dwFileIndexSize;
	//��ȡͷ
	//��ȡβ
	//��ȡfileindex
	BOOL (*PickIndexData)(void*, void*);
	//����ͷ
	LPVOID (*FillHeadData)(void*);
	//����β
	LPVOID (*FillTailData)(void*);
	//����fileindex
	LPVOID (*FillIndexData)(void*, void*);
}PCK_VERSION_FUNC, *LPPCK_VERSION_FUNC;

//****** structures ******* 
#pragma pack(push, 4)

/*
** PCKHEAD PCK�ļ����ļ�ͷ�ṹ
** size = 12
** 
** dwHeadCheckHead
** ���ļ���������Ϸ��أ���ͬ��Ϸ��ֵ��ͬ
** 
** dwPckSize
** ����pck�ļ��Ĵ�С
** 
** dwHeadCheckTail
** ���ļ��汾��2.0.2ʱ���ã����汾��2.0.3ʱ���ϲ���dwPckSize
** ����Ϊ��λʹ��
** 
*/

typedef struct _PCK_HEAD_V2020 {
	DWORD		dwHeadCheckHead;
	DWORD		dwPckSize;
	DWORD		dwHeadCheckTail;
}PCKHEAD_V2020, *LPPCKHEAD_V2020;

typedef struct _PCK_HEAD_V2030 {
	DWORD		dwHeadCheckHead;
	union {
		QWORD		dwPckSize;
		struct {
			DWORD		dwPckSizeLow;
			DWORD		dwHeadCheckTail;
		};
	};
}PCKHEAD_V2030, *LPPCKHEAD_V2030, PCKHEAD_V2031, *LPPCKHEAD_V2031;

/*
** PCKTAIL PCK�ļ����ļ�β�����8�ֽڣ��ṹ
** size = 8
** 
** dwFileCount
** �ļ���ѹ�����ļ�����
** 
** dwVersion
** pck�ļ��İ汾
** 
*/

typedef struct _PCK_TAIL_V2020 {
	DWORD		dwIndexTableCheckHead;
	DWORD		dwVersion0;
	DWORD		dwCryptedFileIndexesAddr;
	DWORD		dwNullDword;	//a5 != 0 ? 0x80000000 : 0 ���� a5 === 0
	char		szAdditionalInfo[PCK_ADDITIONAL_INFO_SIZE];
	DWORD		dwIndexTableCheckTail;
	DWORD		dwFileCount;
	DWORD		dwVersion;
}PCKTAIL_V2020, *LPPCKTAIL_V2020;

typedef struct _PCK_TAIL_V2030 {
	DWORD		dwIndexTableCheckHead;
	union {
		QWORD		dwCryptedFileIndexesAddr;
	struct {
			DWORD		dwVersion0;
			DWORD		dwCryptedFileIndexesAddrHigh;
		};
	};
	DWORD		dwNullDword;
	char		szAdditionalInfo[PCK_ADDITIONAL_INFO_SIZE];
	DWORD		dwIndexTableCheckTail;
	DWORD		dwFileCount;
	DWORD		dwVersion;
}PCKTAIL_V2030, *LPPCKTAIL_V2030;

typedef struct _PCK_TAIL_V2031 {
	DWORD		dwIndexTableCheckHead;
	DWORD		dwVersion0;
	QWORD		dwCryptedFileIndexesAddr;
	DWORD		dwNullDword;
	char		szAdditionalInfo[PCK_ADDITIONAL_INFO_SIZE];
	QWORD		dwIndexTableCheckTail;
	DWORD		dwFileCount;
	DWORD		dwVersion;
}PCKTAIL_V2031, *LPPCKTAIL_V2031;

typedef struct _PCK_FILE_INDEX_V2020 {
	char		szFilename[MAX_PATH_PCK_256];
	DWORD		dwUnknown1;
	DWORD		dwAddressOffset;
	DWORD		dwFileClearTextSize;
	DWORD		dwFileCipherTextSize;
	DWORD		dwUnknown2;
}PCKFILEINDEX_V2020, *LPPCKFILEINDEX_V2020;

typedef struct _PCK_FILE_INDEX_V2030 {
	char		szFilename[MAX_PATH_PCK_256];
	DWORD		dwUnknown1;
	QWORD		dwAddressOffset;
	DWORD		dwFileClearTextSize;
	DWORD		dwFileCipherTextSize;
	DWORD		dwUnknown2;
}PCKFILEINDEX_V2030, *LPPCKFILEINDEX_V2030;

typedef struct _PCK_FILE_INDEX_V2031 {
	char		szFilename[MAX_PATH_PCK_260];
	DWORD		dwUnknown1;
	QWORD		dwAddressOffset;
	DWORD		dwFileClearTextSize;
	DWORD		dwFileCipherTextSize;
	DWORD		dwUnknown2;
}PCKFILEINDEX_V2031, *LPPCKFILEINDEX_V2031;

typedef struct _PCK_FILE_INDEX {
	char		szFilename[MAX_PATH_PCK_260];
	QWORD		dwAddressOffset;
	DWORD		dwFileClearTextSize;
	DWORD		dwFileCipherTextSize;
}PCKFILEINDEX, *LPPCKFILEINDEX;
#pragma pack(pop)

typedef struct _PCK_INDEX_TABLE {
	PCKFILEINDEX	cFileIndex;
	BOOL			bSelected;			//�����ϱ�ѡ��ʱ��1��һ����ɾ���ڵ�ʱʹ��
	BOOL			isRecompressed;		//ѹ���ؽ�ʱʹ��
	BOOL			isInvalid;
}PCKINDEXTABLE, *LPPCKINDEXTABLE;


typedef struct _PCK_PATH_NODE {
	char			szName[MAX_PATH_PCK_260];
	DWORD			dwFilesCount;
	DWORD			dwDirsCount;
	QWORD			qdwDirClearTextSize;
	QWORD			qdwDirCipherTextSize;
	LPPCKINDEXTABLE	lpPckIndexTable;
	_PCK_PATH_NODE	*parentfirst;
	_PCK_PATH_NODE	*parent;
	_PCK_PATH_NODE	*child;
	_PCK_PATH_NODE	*next;
}PCK_PATH_NODE, *LPPCK_PATH_NODE;


typedef struct _FILES_TO_COMPRESS {
	DWORD			dwCompressedflag;
	DWORD			dwFileSize;
	char			*lpszFileTitle;
	DWORD			nBytesToCopy;
	//char			szBase64Name[MAX_PATH_PCK];
	char			szFilename[MAX_PATH];
	_FILES_TO_COMPRESS	*next;
	_PCK_INDEX_TABLE	*samePtr;
}FILES_TO_COMPRESS, *LPFILES_TO_COMPRESS;


typedef struct _PCK_INDEX_TABLE_COMPRESS {
	DWORD			dwIndexValueHead;
	DWORD			dwIndexValueTail;
	BYTE			buffer[MAX_INDEXTABLE_CLEARTEXT_LENGTH];
	DWORD			dwIndexDataLength;					//�ļ�����ѹ����Ĵ�С
	DWORD			dwCompressedFilesize;				//ѹ������ļ���С
	DWORD			dwMallocSize;						//����ռ�ʹ�õĴ�С��>=ѹ������ļ���С��
	QWORD			dwAddressFileDataToWrite;			//ѹ���������д���ļ��ĵ�ַ
	DWORD			dwAddressOfDuplicateOldDataArea;	//���ʹ���������������ַ
	BOOL			bInvalid;							//���ģʽʱ���ļ�������ظ���ʹ���Ѵ��ڵ��ļ��������������
	QWORD			dwAddressAddStep;					//д���ļ���dwAddress��ָ��Ӧ�ü��ϵ�����
}PCKINDEXTABLE_COMPRESS, *LPPCKINDEXTABLE_COMPRESS;


typedef struct _PCK_ALL_INFOS {
	QWORD			qwPckSize;
	DWORD			dwFileCount;
	QWORD			dwAddressName;
	char			szAdditionalInfo[PCK_ADDITIONAL_INFO_SIZE];
	TCHAR			szFilename[MAX_PATH];

	const PCK_VERSION_FUNC*	lpDetectedPckVerFunc;
	const PCK_VERSION_FUNC*	lpSaveAsPckVerFunc;

}PCK_ALL_INFOS, *LPPCK_ALL_INFOS;

typedef struct _PCK_WRITING_VARS {

	QWORD	dwAddress;		//ѹ���㷨��ʹ�õĲ�������ǰд���ַ���ļ���С
	void*	lpFileRead;
	void*	lpFileWrite;
	int		level;

}PCK_WRITING_VARS, *LPPCK_WRITING_VARS;


#endif