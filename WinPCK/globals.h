//////////////////////////////////////////////////////////////////////
// globals.h: WinPCK 全局头文件
// 界面类的全局头文件
//
// 此程序由 李秋枫/stsm/liqf 编写
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.4.10
//////////////////////////////////////////////////////////////////////

//#define _WIN32_WINNT 0x0501


#ifndef _GLOBALS_H_
#define _GLOBALS_H_

//#include "../tlib/tlib.h"
#include <windows.h>
//#include "resource.h"


//****** some defines *******
//#define _USE_CUSTOMDRAW_


#define	THIS_NAME			"WinPCK "

#define	THIS_VERSION		"v1.20.8d "
//#define	THIS_DESC			"完美世界系列游戏PCK文件包综合查看器 " 
#ifdef PCKV202
#define	THIS_DESC			"完美世界系列游戏PCK文件包综合查看器 (完美世界+诛仙+武林外传+圣斗士等)" 
							//"(完美世界+诛仙+武林外传+圣斗士等)"
#elif defined PCKV203
#define	THIS_DESC			"完美世界系列游戏PCK文件包综合查看器 (笑傲江湖)" 
							//"(笑傲江湖)"
#elif defined PCKV203ZX
#define	THIS_DESC			"完美世界系列游戏PCK文件包综合查看器 (诛仙203)" 

#endif
#define	THIS_AUTHOR			"程序编写 stsm/liqf/李秋枫\r\nE-Mail: stsm85@126.com\r\n\r\n诛仙十年兼容修改：https://github.com/wzv5/winpck"


#define TEXT_PROCESS_ERROR	"操作未能正常结束"

#ifdef _WIN64
	#define	THIS_MAIN_CAPTION	THIS_NAME \
								"64bit " \
								THIS_VERSION
#else
	#define	THIS_MAIN_CAPTION	THIS_NAME \
								THIS_VERSION
#endif
							

//#define	SHELL_LISTVIEW_PARENT_CLASS		TEXT("SHELLDLL_DefView")
#define	SHELL_LISTVIEW_ROOT_CLASS1		TEXT("ExploreWClass")
#define	SHELL_LISTVIEW_ROOT_CLASS2		TEXT("CabinetWClass")
//#define	SHELL_EXENAME					TEXT("\\explorer.exe")

#define	TEXT_INVALID_PATHCHAR			"\\/:*?\"<>|"

#define	FILE_EXT_PIC					".dds.tga.bmp.jpg.png.gif.jpeg.tif.tiff.emf"

#define	TEXT_FILE_FILTER				TEXT(	"PCK压缩文件(*.pck;*.zup)\0*.pck;*.zup\0")	\
										TEXT(	"所有文件\0*.*\0\0")



#define WM_FRESH_MAIN_CAPTION		(WM_USER + 1)
#define WM_TIMER_PROGRESS_100		(WM_USER + 1)

#define	TIMER_PROGRESS				100

//#define	FMT_RAW			0	//bmp,jpg,png,gif,emf.....
//#define	FMT_DDS			1
//#define	FMT_TGA			2

#define	MAX_BUFFER_SIZE_OFN			0xFFFFF



#define	MT_MAX_MEMORY				0x40000000	//1024MB


#ifndef SE_CREATE_SYMBOLIC_LINK_NAME
#define SE_CREATE_SYMBOLIC_LINK_NAME  TEXT("SeCreateSymbolicLinkPrivilege")
#endif

#ifdef _USE_CUSTOMDRAW_

#define	HB_GREEN					0
#define	HB_GRAY						1
#define	HB_SELECTED					2
//#define	HB_NOFOCUS					3
#define	HB_COUNT					3

//ListView Colors
#define FILL_COLOR_SELECTED			RGB(200,240,180)
//#define FRAME_COLOR_SELECTED		RGB(000,000,000)
//#define FILL_COLOR_NOFOCUS			RGB(200,240,255)
//#define FRAME_COLOR_NOFOCUS			RGB(100,100,100)
//#define FILL_COLOR_RED				RGB(255,220,220)
//#define FILL_COLOR_YELLOW			RGB(255,255,200)
#define FILL_COLOR_GRAY				RGB(237,238,250)
#define FILL_COLOR_GREEN			RGB(232,254,255)

#endif

//BOOL GetWndPath(HWND hWnd, TCHAR * szPath);

#endif