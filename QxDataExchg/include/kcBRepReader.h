//////////////////////////////////////////////////////////////////////////
// 导入BRep文件
#ifndef _KC_BREP_READER_H_
#define _KC_BREP_READER_H_

#include "kcBaseReader.h"

class kcModel;

class QXLIB_API kcBRepReader : public kcBaseReader
{
public:
	kcBRepReader(kcModel *pModel);
	~kcBRepReader(void);

	//导入
	virtual	BOOL			DoImport();
};

#endif