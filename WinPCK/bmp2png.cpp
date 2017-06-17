//////////////////////////////////////////////////////////////////////
// bmp2png.cpp: WinPCK �����̲߳���
// bmp32->png
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2015.6.2
//////////////////////////////////////////////////////////////////////

#include "miscdlg.h"
#include "png.h"
#include <process.h>
//#include "pngstruct.h"

#define CHANNELS 4
#define WM_TIMER_PROGRESS_100		(WM_USER + 1)
#define	TIMER_PROGRESS				100

png_uint_32	iRow;

char szPngFile[MAX_PATH];

void row_callback( png_structp png_ptr, png_uint_32 row, int pass )
{
	iRow = row;
}


BOOL TPicDlg::PrepareToSavePng(char *lpFile)
{
	memcpy(szPngFile, lpFile, MAX_PATH);
	_beginthread(SavePng, 0, this);
	return TRUE;
}


VOID TPicDlg::SavePng(VOID *pParam)
{

	TPicDlg	*pThis = (TPicDlg*)pParam;

	png_structp png_ptr;
	png_infop info_ptr;

	png_uint_32         ulRowBytes;

	static png_byte   **ppbRowPointers = NULL;

	png_byte *pDiData = NULL;

	int iWidth = pThis->picWidth;

	int iHeight = pThis->picHeight;


	// ���ñ�׼PNG�ṹ
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL,
	                                     (png_error_ptr)NULL, (png_error_ptr)NULL );
	if( png_ptr==NULL ){
		pThis->MessageBoxA(TEXT_PNG_OUTOFMEMORY, NULL, MB_OK | MB_ICONERROR);
		_endthread();
		//return FALSE;
	}

	info_ptr = png_create_info_struct( png_ptr );
	if( info_ptr==NULL ){
		png_destroy_write_struct( &png_ptr, NULL );
		pThis->MessageBoxA(TEXT_PNG_OUTOFMEMORY, NULL, MB_OK | MB_ICONERROR);
		_endthread();
		//return FALSE;
	}

	FILE *fp;
	if(NULL == (fp = fopen(szPngFile,"wb")))
	{
		png_destroy_write_struct( &png_ptr, &info_ptr );
		pThis->MessageBoxA(TEXT_SAVE_DDS_ERROR, NULL, MB_OK | MB_ICONERROR);
		_endthread();
		//return FALSE;
	}
	//fwrite(hG, 1, dwDdsToBmpSize, fp);
	
	//png_set_write_fn(png_ptr, 
	png_init_io( png_ptr, fp );
	png_set_compression_level( png_ptr, 9 );

	png_set_compression_mem_level( png_ptr, 9 );

	// ���õ�ɫ��
	png_set_IHDR( png_ptr, info_ptr, iWidth, iHeight, 8,
	              PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	              PNG_FILTER_TYPE_DEFAULT );

	png_write_info( png_ptr, info_ptr );

	// ��BGRA���ARGB
	png_set_bgr( png_ptr );

	// һ�е��ֽ���
	ulRowBytes = iWidth * CHANNELS;

	// Ϊָ������ڴ�
	if ((ppbRowPointers = (png_bytepp) malloc(iHeight * sizeof(png_bytep))) == NULL)
	{
		png_destroy_write_struct( &png_ptr, &info_ptr );

		pThis->MessageBoxA(TEXT_MALLOC_FAIL, NULL, MB_OK | MB_ICONERROR);
		fclose(fp);
		_endthread();
		//return FALSE;
	}

	pDiData = (png_byte*)pThis->cleanimage;

	// ����ָ��
	for (int i = 0; i < iHeight; ++i){
		ppbRowPointers[i] = pDiData + i * iWidth * 4;
	}

	//����
	png_set_write_status_fn( png_ptr, row_callback );

	pThis->SetTimer(WM_TIMER_PROGRESS_100, TIMER_PROGRESS, NULL);

	// һ����д��
	png_write_image( png_ptr, ppbRowPointers );

	// ��д���ӿ飬���Ǳ���
	png_write_end( png_ptr, info_ptr );

	// ɨβ
	png_destroy_write_struct( &png_ptr, &info_ptr );

	pThis->KillTimer(WM_TIMER_PROGRESS_100);

	fclose(fp);

	free(ppbRowPointers);

	pThis->SetWindowTextA(pThis->szTitle);

	_endthread();
	//return TRUE;
}

BOOL TPicDlg::EvTimer(WPARAM timerID, TIMERPROC proc)
{
	char		szString[MAX_PATH];
	switch(timerID)
	{
	case WM_TIMER_PROGRESS_100:

		sprintf(szString, "%s (������...%.1f%%)", szTitle, iRow * 100.0 / picHeight);
		SetWindowTextA(szString);

		break;
	}
	return	FALSE;
}