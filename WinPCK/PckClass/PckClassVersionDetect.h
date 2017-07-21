//////////////////////////////////////////////////////////////////////
// PckClassVersionDetect.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �汾��صļ���Լ����ݵ�д�롢��ȡ
//
// �˳����� �����/stsm/liqf ��д
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.6.27
//////////////////////////////////////////////////////////////////////

//���pck�İ汾��д�뵽iDetectedPckID��ͬʱ��ȡ�ļ�ͷ��β
BOOL	DetectPckVerion(LPCTSTR lpszPckFile, LPPCK_ALL_INFOS pckAllInfo);
//void	PrintInvalidVersionDebugInfo();

TCHAR		szSaveDlgFilterString[1024];

const static PCK_KEYS			cPckKeys[PCK_VERSION_NUMS];
const static PCK_VERSION_FUNC	cPckVersionFunc[PCK_VERSION_NUMS];

void	BuildSaveDlgFilterString();

//�ļ�ͷ��β��������д������д��
static LPVOID	FillHeadData_V2020(void *param);
static LPVOID	FillHeadData_V2030(void *param);

static LPVOID	FillTailData_V2020(void *param);
static LPVOID	FillTailData_V2030(void *param);
static LPVOID	FillTailData_V2031(void *param);

static LPVOID FillIndexData_V2020(void *param, void *pckFileIndexBuf);
static LPVOID FillIndexData_V2030(void *param, void *pckFileIndexBuf);
static LPVOID FillIndexData_V2031(void *param, void *pckFileIndexBuf);

//���ݴ�lpIndex -> param
static BOOL PickIndexData_V2020(void *param, void* lpIndex);
static BOOL PickIndexData_V2030(void *param, void* lpIndex);
static BOOL PickIndexData_V2031(void *param, void* lpIndex);