// 模型的保存和读取实现
#include "StdAfx.h"
#include <AIS_InteractiveObject.hxx>

#include "kcEntity.h"
#include "kcLayer.h"
#include "kcHandleMgr.h"
#include "QxStgBlock.h"
#include "kuiInterface.h"
#include "QxStgModelDef.h"
#include "kcBasePlane.h"
#include "kcModel.h"

//说明：
//  为了方便解读，或者实现某些块的忽略，model中的保存，通常采用如下方式：
//  begin block 记录简单信息
//  一系列的子block，记录相应的数据
//  end block 结束记录
//  begin and end block都包含一个code值，通过该(blocktype,code)实现begin和end的配对,因此code
//  需要避免冲突，这个不是强制的。

// 保存和读取
//保存如下信息：
// 1、handleMgr信息 2、图层和图元信息 3、基准面信息
//
BOOL kcModel::Read(CArchive& ar,UINT dwFileVersion)
{
	ASSERT(ar.IsLoading());
	
	//清除旧的数据
	Clear();

	dwFileVersion_ = dwFileVersion;

	QxStgBlock blk;
	//读取起始块
	if(!blk.Read(ar,KSTG_BLOCK_MODEL)){
		ASSERT(FALSE);
		return FALSE;
	}
	UINT modelTag = 0;
	if(!blk.GetValueByCode(KSTG_CV_CODE_BLOCK_TAG,modelTag) || modelTag != 1){
		ASSERT(FALSE);
		return FALSE;
	}

	if(!ReadHandleMgr(ar) ||
		!ReadLayerData(ar) || 
		!ReadBasePlaneData(ar))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	//读取结束块
	if(!blk.Read(ar,KSTG_BLOCK_MODEL_END)){
		ASSERT(FALSE);
		return FALSE;
	}
	if(!blk.GetValueByCode(KSTG_CV_CODE_BLOCK_TAG,modelTag) || modelTag != 1){
		ASSERT(FALSE);
		return FALSE;
	}
		
	return TRUE;
}

// 写入文件。
BOOL	kcModel::Write(CArchive& ar)
{
	ASSERT(ar.IsStoring());

	UINT modelCode = 1;
	//写起始
	QxStgBlock blk;

	blk.Initialize(KSTG_BLOCK_MODEL);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,modelCode);
	blk.Write(ar);

	//1、HandleMgr信息
	WriteHandleMgr(ar);

	//2、图层信息
	WriteLayerData(ar);
	
	//3、基准面信息
	WriteBasePlaneData(ar);

	//写入结束块
	blk.Initialize(KSTG_BLOCK_MODEL_END);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,modelCode);
	blk.Write(ar);
	
	return TRUE;
}

BOOL kcModel::ReadHandleMgr(CArchive& ar)
{
	if(_pHandleMgr){
		if(!_pHandleMgr->Read(ar)){
			ASSERT(FALSE);
			return FALSE;
		}
	}else{
		ASSERT(FALSE);
	}

	return TRUE;
}

BOOL kcModel::WriteHandleMgr(CArchive& ar)
{
	ASSERT(_pHandleMgr);

	if(_pHandleMgr){
		_pHandleMgr->Write(ar);
	}

	return TRUE;
}

BOOL  kcModel::ReadLayerData(CArchive& ar)
{
	//图层信息
	QxStgBlock blk;

	if(!blk.Read(ar,KSTG_BLOCK_LAYERS)){
		TRACE("\n ReadLayerData False.");
		return FALSE;
	}

	int nCurLayerId = 0,nbLayer = 0;
	if(!blk.GetValueByCode(1,nbLayer) ||
		!blk.GetValueByCode(2, nCurLayerId))//获取当前图层的handle
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	//依次读取每个图层
	kcLayer *pLayer = NULL;
	int idx;

	for(idx = 0;idx < nbLayer;idx ++){
		pLayer = new kcLayer;
		if(!pLayer->Read(ar,this,dwFileVersion_)){
			Clear();
			//
			if(_pModelTreeInterface)
				_pModelTreeInterface->RemoveLayer(pLayer->GetLayerName().c_str());
			return FALSE;
		}
		_layerList.push_back(pLayer);

		if(pLayer->GetID() == nCurLayerId){
			SetCurrLayer(pLayer);
		}
	}

	//读取结尾块
	if(!blk.Read(ar,KSTG_BLOCK_LAYERS_END)){
		return FALSE;
	}

	return TRUE;
}

// LAYERS LAYER1 LAYER2... LAYERS_END
//
BOOL  kcModel::WriteLayerData(CArchive& ar)
{
	QxStgBlock blk;

	blk.Initialize(KSTG_BLOCK_LAYERS);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,1);//tag = 1

	// 图层个数
	int nbLayer = (int)_layerList.size();
	blk.AddCodeValue(1,nbLayer);
	//当前图层
	int nId = _pCurLayer->GetID();
	blk.AddCodeValue(2,nId);
	
	blk.Write(ar);

	//依次写入每个图层
	kcLayer *pLayer = NULL;
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++){
		(*it)->Write(ar);
	}

	//写入结束块
	blk.Initialize(KSTG_BLOCK_LAYERS_END);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,1);//tag = 1
	blk.Write(ar);

	return TRUE;
}

BOOL  kcModel::ReadBasePlaneData(CArchive& ar)
{
	QxStgBlock blk;

	if(!blk.Read(ar)){
		TRACE("\n ReadBasePlaneData failed.");
		return FALSE;
	}

	int ix,nbCnt = 0,nCurHandle = 0;
	if(blk.GetType() != KSTG_BLOCK_BASEPLANE || 
		!blk.GetValueByCode(1,nCurHandle) ||
		!blk.GetValueByCode(6,nbCnt))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	//依次读取每个基准面信息
	Handle(V3d_View) aView = GetActiveView();//;glb_GetCurrV3dView();
	kcBasePlane *pBasePlane = NULL;
	for(ix = 0;ix < nbCnt;ix ++){
		pBasePlane = new kcBasePlane(aView);
		if(!pBasePlane->Read(ar,this)){
			Clear();

			return FALSE;
		}
		AddBasePlane(pBasePlane);

		if(pBasePlane->GetHandle() == nCurHandle){
			SetCurrentBasePlane(pBasePlane);
		}
	}

	return TRUE;
}

BOOL  kcModel::WriteBasePlaneData(CArchive& ar)
{
	QxStgBlock basePlnBlk;
	int nbBasePlane = (int)_basePlaneList.size();

	basePlnBlk.Initialize(KSTG_BLOCK_BASEPLANE);
	int nHandle = _pCurBasePlane->GetHandle();
	basePlnBlk.AddCodeValue(1,nHandle);
	basePlnBlk.AddCodeValue(6,nbBasePlane);

	basePlnBlk.Write(ar);

	//一次写入每个基准面
	kcBasePlaneList::iterator bpit = _basePlaneList.begin();
	for(;bpit != _basePlaneList.end();bpit ++){
		(*bpit)->Write(ar);
	}

	return TRUE;
}
