#include "StdAfx.h"
#include "kcModel.h"
#include "kcHandleMgr.h"
#include "QxStgBlock.h"
#include "kcDocContext.h"
#include "QxStgDocDef.h"
#include "QxStgModelDef.h"
#include "QxStgDocReader.h"

QxStgDocReader::QxStgDocReader(kcDocContext *pDocCtx)
{
	ASSERT(pDocCtx);
	pDocContext_ = pDocCtx;
	dwReadVersion_ = 0;
}

QxStgDocReader::~QxStgDocReader(void)
{
}

// ¶ÁÈ¡
bool QxStgDocReader::Load(CArchive &ar)
{
	ASSERT(ar.IsLoading());
	kcModel *pModel = pDocContext_->GetModel();
	if(!pModel)
		return false;

	if(!ReadFileHeader(ar))
		return false;

	if(!ReadGlobalBlock(ar))
		return false;

	return (ReadModel(ar,pModel) ? true : false);
}

//
BOOL QxStgDocReader::ReadFileHeader(CArchive& ar)
{
	QxStgBlock blk;
	if(!blk.Read(ar) || blk.GetType() != KSTG_BLOCK_DOC_HEADER){
		return FALSE;
	}

	QxStgCodeValue *pCV = NULL;
	UINT dwMagic = 0,dwVer = 0;
	//magic
	pCV = blk.GetCodeValueByCode(0);
	if(!pCV || !pCV->GetValue(dwMagic) ||
		dwMagic != KSTG_DOC_FILE_MAGIC)
	{
		return FALSE;
	}

	//ver
	pCV = blk.GetCodeValueByCode(1);
	if(!pCV || !pCV->GetValue(dwVer)){
		return FALSE;
	}
	if(dwVer > KSTG_DOC_FILE_VERSION){
		return FALSE;
	}
	dwReadVersion_ = dwVer;
	
	return TRUE;
}

//
BOOL QxStgDocReader::ReadGlobalBlock(CArchive& ar)
{
	QxStgBlock blk;
	if(!blk.Read(ar))
		return FALSE;

	if(blk.GetType() != KSTG_BLOCK_DOC_GLOBAL)
		return FALSE;

	//¶ÁÈ¡±³¾°ÑÕÉ«
	double dCol[3];
	blk.GetValueByCode(0,dCol);

	return TRUE;
}

//
BOOL QxStgDocReader::ReadModel(CArchive& ar,kcModel *pModel)
{
	return pModel->Read(ar,dwReadVersion_);
}