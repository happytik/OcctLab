//////////////////////////////////////////////////////////////////////////
// 
#ifndef _KC_DXF_READER_H_
#define _KC_DXF_READER_H_

#include "QxLibDef.h"

class kcModel;

class QXLIB_API kcDXFReader{
public:
	kcDXFReader(kcModel *pModel);
	~kcDXFReader();

	//导入文件
	bool					DoImport();

protected:
	kcModel					*m_pModel;
};

#endif