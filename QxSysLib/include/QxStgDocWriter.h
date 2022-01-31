//////////////////////////////////////////////////////////////////////////
// 保存数据
#ifndef _QX_STG_DOC_WRITER_H_
#define _QX_STG_DOC_WRITER_H_

#include "QxLibDef.h"

class kcDocContext;
class kcModel;
class CArchive;

class QXLIB_API QxStgDocWriter
{
public:
	QxStgDocWriter(kcDocContext *pDocCtx);
	~QxStgDocWriter(void);

	//写入所有
	bool				Save(CArchive &ar);

protected:
	//创建头块
	BOOL				WriteHeaderBlock(CArchive& ar);
	//创建全局块
	BOOL				WriteGlobalBlock(CArchive& ar);

	//写入文件
	BOOL				WriteModel(CArchive& ar,kcModel *pModel);

protected:
	kcDocContext		*pDocContext_;
};

#endif