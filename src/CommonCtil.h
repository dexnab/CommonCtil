/**
* @file CommonCtil.h
* @brief CommonCtil 项目管理模块；包含该文件即可使用 CommonCtil 的所有功能
* @date 2022-02-22
* @author dexnab
* @author <李老师你把这里改成你的名字（github账号名）>
*/
#include <cmtType.h>

/**
* @typedef cmtConf
* @brief CommonCtil项目配置信息
* @see _CMTCONF
* @date 2022-02-22
* @author dexnab
*/
/**
* @brief CommonCtil项目配置信息
* @date 2022-02-22
* @author dexnab
*/
typedef struct _CMTCONF
{
	cmtUint8 aes : 1;
	cmtUint8 avx : 1;
	cmtUint8 avx2 : 1;
	cmtUint8 cmov : 1;
	cmtUint8 fpu : 1;
	cmtUint8 mmx : 1;
	cmtUint8 mmxext : 1;
	cmtUint8 movbe : 1;
	cmtUint8 msr : 1;
	cmtUint8 rdpid : 1;
	cmtUint8 rdpru : 1;
	cmtUint8 rdrand : 1;
	cmtUint8 rdtscp : 1;
	cmtUint8 rdseed : 1;
	cmtUint8 sha : 1;
	cmtUint8 sse1 : 1;
	cmtUint8 sse2 : 1;
	cmtUint8 sse3 : 1;
	cmtUint8 ssse3 : 1;
	cmtUint8 sse4a : 1;
	cmtUint8 sse41 : 1;
	cmtUint8 sse42 : 1;
	cmtUint8 rdtsc : 1;
}cmtConf;

/**
* @brief CommonCtil 初始配置器；请在使用 CommonCtil 之前调用该函数
* @pre @a locale 字符串必须有结束符
* @test cmtDemoANSI() 状态：PA
* @date 2021-09-23
* @author dexnab
*/
extern void cmtConfInit();