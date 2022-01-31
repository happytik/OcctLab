//////////////////////////////////////////////////////////////////////////
// 实现对一下字符串值的解析
#ifndef _KI_PARSE_STRING_H_
#define _KI_PARSE_STRING_H_

#include "kPoint.h"
#include "QxLibDef.h"

class QXLIB_API kiParseInteger{
public:
	kiParseInteger(const char *pString);

	//解析,结果返回在ival中
	bool					Parse(int& ival);

protected:
	const char				*_pString;
};

// 解析浮点数
class QXLIB_API kiParseDouble{
public:
	kiParseDouble(const char *pString);

	//解析，结果返回在dval中
	bool					Parse(double& dval);

protected:
	const char				*_pString;
};

//解析点
class QXLIB_API kiParseStringToPoint3{
public:
	kiParseStringToPoint3(const char *pString);

	//解析,结果返回在point中.
	bool					Parse(kPoint3& point);

protected:
	const char				*_pString;
};


class kiParseString
{
public:
	kiParseString(void);
	~kiParseString(void);
};

#endif