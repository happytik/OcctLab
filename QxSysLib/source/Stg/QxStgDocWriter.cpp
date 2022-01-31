#include "StdAfx.h"
#include "kcModel.h"
#include "QxStgBlock.h"
#include "kcDocContext.h"
#include "QxStgModelDef.h"
#include "QxStgDocDef.h"
#include "QxStgDocWriter.h"

QxStgDocWriter::QxStgDocWriter(kcDocContext *pDocCtx)
{
	ASSERT(pDocCtx);
	pDocContext_ = pDocCtx;
}

QxStgDocWriter::~QxStgDocWriter(void)
{
}

//写入所有
bool QxStgDocWriter::Save(CArchive &ar)
{
	ASSERT(ar.IsStoring());
	kcModel *pModel = pDocContext_->GetModel();
	if(!pModel)
		return false;

	//写入文件头
	WriteHeaderBlock(ar);

	//写入全局数据
	WriteGlobalBlock(ar);

	//写入模型数据
	WriteModel(ar,pModel);

	return true;
}

//写入文件
BOOL QxStgDocWriter::WriteModel(CArchive& ar,kcModel *pModel)
{
	ASSERT(ar.IsStoring());
	ASSERT(pModel);
	
	//写入模型文件
	if(!pModel->Write(ar))
		return FALSE;

	return TRUE;
}

//创建头块,目前的头很简单
BOOL QxStgDocWriter::WriteHeaderBlock(CArchive& ar)
{
	//先清除。
	QxStgBlock blk;
	//
	blk.Initialize(KSTG_BLOCK_DOC_HEADER);
	blk.AddCodeValue(0,(UINT)KSTG_DOC_FILE_MAGIC);//magic
	blk.AddCodeValue(1,(UINT)KSTG_DOC_FILE_VERSION);//version
	//
	blk.AddCodeValue(2,"opencasecade org");//desc

	blk.Write(ar);
    
	return TRUE;
}

//创建全局块
BOOL QxStgDocWriter::WriteGlobalBlock(CArchive& ar)
{
	QxStgBlock blk;
	//
	blk.Initialize(KSTG_BLOCK_DOC_GLOBAL);

	//背景色
	double bkCol[3];
	bkCol[0] = 0.2;
	bkCol[1] = 0.3;
	bkCol[2] = 0.4;
	blk.AddCodeValue(0,bkCol);
	//
	blk.Write(ar);

	return TRUE;
}