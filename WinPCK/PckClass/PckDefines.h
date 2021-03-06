//////////////////////////////////////////////////////////////////////
// PckDefines.h: 用于解析完美世界公司的pck文件中的数据，并显示在List中
// 头文件
//
// 此程序由 李秋枫/stsm/liqf 编写
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2015.5.13
//////////////////////////////////////////////////////////////////////

#include "PckConf.h"

#if !defined(_PCKDEFINES_H_)
#define _PCKDEFINES_H_



//#ifdef UNICODE
//	#define tcscpy_s	wcscpy_s
//	#define tcscat_s	wcscat_s
//#else
//	#define tcscpy_s	strcpy_s
//	#define tcscat_s	strcat_s
//#endif

typedef unsigned __int64	QWORD, *LPQWORD;
//typedef unsigned __int32	DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
#define MAX_PATH			260



//PckVersion
#define	PCK_VERSION_ZX		1
#define	PCK_VERSION_SDS		2

#define	PCK_VERSION_XAJH	1



#if defined PCKV202
	#define PCKADDR	DWORD
#elif defined PCKV203
	#define PCKADDR	QWORD
#elif defined PCKV203ZX
	#define PCKADDR	QWORD
#endif

#ifdef PCKV202
	#define	INDEXTABLE_CLEARTEXT_LENGTH	0x114
#elif defined PCKV203
	#define	INDEXTABLE_CLEARTEXT_LENGTH	0x118
#elif defined PCKV203ZX
	#define INDEXTABLE_CLEARTEXT_LENGTH	0x120
#endif

#define	PCK_BEGINCOMPRESS_SIZE			20
#define	MAX_PATH_PCK					256
#ifdef PCKV203ZX
	#define PCK_TAIL_OFFSET					288
#else
	#define PCK_TAIL_OFFSET					280
#endif
#define PCK_DATA_START_AT				12


//创建时，剩余文件的空间量不够时，添加量
#define	PCK_STEP_ADD_SIZE				0x4000000
//创建时，剩余文件的空间量
#define	PCK_SPACE_DETECT_SIZE			0x100000

#define	PCK_ADDITIONAL_INFO				"Angelica File Package"
#define	PCK_ADDITIONAL_INFO_STSM		"\r\nCreate by WinPCK"


//日志
#define	LOG_BUFFER						1024

#define	LOG_FLAG_ERROR					'E'
#define	LOG_FLAG_WARNING				'W'
#define	LOG_FLAG_INFO					'I'
#define	LOG_FLAG_DEBUG					'D'

#define LOG_IMAGE_INFO					0
#define LOG_IMAGE_WARNING				1
#define LOG_IMAGE_ERROR					2
#define LOG_IMAGE_DEBUG					3

//LOG INFO STRING
#define TEXT_LOG_OPENFILE				"打开文件 %s"
#define TEXT_LOG_CLOSEFILE				"关闭文件"
#define	TEXT_LOG_FLUSH_CACHE			"写入缓存中..."
#define	TEXT_LOG_RESTOR_OK				"数据恢复成功"

#define TEXT_LOG_WORKING_DONE			"操作完成"

#define	TEXT_LOG_LEVEL_THREAD			"压缩率=%d:线程=%d"

#define	TEXT_LOG_UPDATE_ADD				"更新模式"
#define	TEXT_LOG_UPDATE_NEW				"新建模式"

#define	TEXT_LOG_RENAME					"重命名(删除)包中文件..."
#define	TEXT_LOG_REBUILD				"重建PCK文件..."

#define	TEXT_LOG_CREATE					"新建PCK文件:%s..."
#define	TEXT_LOG_COMPRESSOK				"压缩完成，写入索引..."

#define	TEXT_LOG_EXTRACT				"解压文件..."



//ERROR STRING
#define	TEXT_ERROR						"错误"

#define	TEXT_MALLOC_FAIL				"申请内存失败！"
#define	TEXT_CREATEMAP_FAIL				"创建文件映射失败！"
#define	TEXT_CREATEMAPNAME_FAIL			"映射文件\"%s\"失败！"
#define	TEXT_OPENNAME_FAIL				"打开文件\"%s\"失败！"
#define	TEXT_VIEWMAP_FAIL				"创建映射视图失败！"
#define	TEXT_VIEWMAPNAME_FAIL			"文件\"%s\"创建映射视图失败！"

#define TEXT_READ_INDEX_FAIL			"文件索引表读取错误！"
#define TEXT_UNKNOWN_PCK_FILE			"无法识别的PCK文件！"

#define	TEXT_OPENWRITENAME_FAIL			"打开写入文件\"%s\"失败！"
#define	TEXT_READFILE_FAIL				"文件读取失败！"
#define	TEXT_WRITEFILE_FAIL				"文件写入失败！"
#define TEXT_WRITE_PCK_INDEX			"写入文件索引出错！"

#define	TEXT_USERCANCLE					"用户取消退出！"

#define	TEXT_COMPFILE_TOOBIG			"压缩文件过大！"
#define	TEXT_UNCOMP_FAIL				"解压文件失败！"

#define	TEXT_UNCOMPRESSDATA_FAIL		"文件 %s \r\n数据解压失败！"

#define	TEXT_ERROR_OPEN_AFTER_UPDATE	"打开失败，可能是上次的操作导致了文件的损坏。\r\n是否尝试恢复到上次打开状态？"
#define	TEXT_ERROR_GET_RESTORE_DATA		"获取恢复信息时出错"
#define	TEXT_ERROR_RESTORING			"恢复时出错"
#define	TEXT_ERROR_DUP_FOLDER_FILE		"存在（文件名=文件夹名），退出。"


//#define	TEXT_REBUILD_ENUMFAIL		"重建文件失败！"


#define	TEXT_EVENT_WRITE_PCK_DATA_FINISH			"WPckDataF%d"
#define	TEXT_EVENT_COMPRESS_PCK_DATA_FINISH			"CPckDataF%d"
#define	TEXT_EVENT_PCK_MAX_MEMORY					"PckMaxMem%d"

#define	TEXT_MAP_NAME_READ							"PckRead%d"
#define	TEXT_MAP_NAME_WRITE							"PckWrite%d"


#endif