
//////////////////////////////////////////////////////////////////////
// PckControlCenterAttribute.cpp: 用于解析完美世界公司的pck文件中的数据，并显示在List中
// 头文件,界面与PCK类的数据交互，控制中心
//
// 此程序由 李秋枫/stsm/liqf 编写，pck结构参考若水的pck结构.txt，并
// 参考了其易语言代码中并于读pck文件列表的部分
//
// 此代码预计将会开源，任何基于此代码的修改发布请保留原作者信息
// 
// 2012.10.10
//////////////////////////////////////////////////////////////////////

#include "PckControlCenter.h"
#include "PckClass.h"



CONST	LPPCKINDEXTABLE		CPckControlCenter::GetPckIndexTable()
{

	return m_lpClassPck->GetPckIndexTable();
}

//CONST	LPPCK_PATH_NODE		CPckControlCenter::GetPckPathNode()
//{
//	return m_lpClassPck->GetPckPathNode();
//}

CONST	LPPCKHEAD			CPckControlCenter::GetPckHead()
{

	return m_lpClassPck->GetPckHead();
}

//LPCTSTR	CPckControlCenter::GetLastErrorString()
//{
//
//	return m_lpClassPck->m_lastErrorString;
//}

DWORD	CPckControlCenter::GetPckFileCount()
{

	return m_lpClassPck->GetPckFileCount();
}

PCKADDR	CPckControlCenter::GetPckSize()
{

	return m_lpClassPck->GetPckHead()->dwPckSize;
}


BOOL	CPckControlCenter::IsValidPck()
{

	return (NULL != m_lpClassPck);
}

BOOL	CPckControlCenter::GetCurrentNodeString(char* szCurrentNodePathString, LPPCK_PATH_NODE lpNode)
{
	return m_lpClassPck->GetCurrentNodeString(szCurrentNodePathString, lpNode);
}

PCKADDR	CPckControlCenter::GetPckRedundancyDataSize()
{

	return m_lpClassPck->GetPckRedundancyDataSize();
}

PCKADDR	CPckControlCenter::GetPckDataAreaSize()
{

	return m_lpClassPck->GetPckDataAreaSize();
}


BOOL	CPckControlCenter::GetSingleFileData(LPVOID lpvoidFileRead, LPPCKINDEXTABLE lpPckFileIndexTable, char *buffer, size_t sizeOfBuffer)
{

	return m_lpClassPck->GetSingleFileData(lpvoidFileRead, lpPckFileIndexTable, buffer, sizeOfBuffer);
}

char*	CPckControlCenter::GetAdditionalInfo()
{

	return m_lpClassPck->GetAdditionalInfo();
}

BOOL	CPckControlCenter::SetAdditionalInfo()
{

	return m_lpClassPck->SetAdditionalInfo();
}

LPCSTR	CPckControlCenter::GetCurrentVersionName()
{
	return cParams.lpPckVersion->getCurrentKey()->name;
}



