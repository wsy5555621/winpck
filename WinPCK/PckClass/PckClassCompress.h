//////////////////////////////////////////////////////////////////////
// PckClassCompress.h: ���ڽ����������繫˾��pck�ļ��е����ݣ�����ʾ��List��
// �ļ�ͷ��β�Ƚṹ��Ķ�ȡ
//
// �˳����� �����/stsm/liqf ��д��pck�ṹ�ο���ˮ��pck�ṹ.txt����
// �ο����������Դ����в��ڶ�pck�ļ��б�Ĳ���
//
// �˴���Ԥ�ƽ��ῪԴ���κλ��ڴ˴�����޸ķ����뱣��ԭ������Ϣ
// 
// 2017.7.3
//////////////////////////////////////////////////////////////////////
#define Z_BEST_COMPRESSION 9


protected:

struct COMPRESS_FUNC {
	unsigned long (*compressBound)(unsigned long);
	int (*compress)(void *dest, unsigned long *destLen, const void *source, unsigned long sourceLen, int level);
}m_PckCompressFunc;

public:

void	init_compressor();

protected:
unsigned long compressBound(unsigned long sourceLen);
int	compress(void *dest, unsigned long *destLen, const void *source, unsigned long sourceLen, int level);
int decompress(void *dest, unsigned long  *destLen, const void *source, unsigned long sourceLen);
int decompress_part(void *dest, unsigned long  *destLen, const void *source, unsigned long sourceLen, unsigned long  fullDestLen);

static unsigned long compressBound_zlib(unsigned long sourceLen);
static int	compress_zlib(void *dest, unsigned long *destLen, const void *source, unsigned long sourceLen, int level);
static unsigned long compressBound_libdeflate(unsigned long sourceLen);
static int	compress_libdeflate(void *dest, unsigned long *destLen, const void *source, unsigned long sourceLen, int level);