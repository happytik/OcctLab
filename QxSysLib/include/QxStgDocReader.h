//////////////////////////////////////////////////////////////////////////
// 读取保存的数据
#ifndef _QX_STG_DOC_READER_H_
#define _QX_STG_DOC_READER_H_

#include "QxLibDef.h"

class kcDocContext;
class kcModel;
class CArchive;

// 负责Doc

class QXLIB_API QxStgDocReader
{
public:
	QxStgDocReader(kcDocContext *pDocCtx);
	~QxStgDocReader(void);

	// 读取
	bool				Load(CArchive &ar);

protected:
	//
	BOOL				ReadFileHeader(CArchive& ar);

	//
	BOOL				ReadGlobalBlock(CArchive& ar);

	//
	BOOL				ReadModel(CArchive& ar,kcModel *pModel);

protected:
	kcDocContext		*pDocContext_;
	UINT				dwReadVersion_;//读入的版本号
};

#endif