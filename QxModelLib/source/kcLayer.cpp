#include "StdAfx.h"
#include "kiSnapMngr.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcModel.h"
#include "kcHandleMgr.h"
#include "QxStgBlock.h"
#include "QxStgModelDef.h"
#include "kuiInterface.h"
#include "kcLayer.h"

kcLayer::kcLayer()
{
	_pModel = NULL;
	_strName = "";
	_nHandle = 0;//无效句柄
}

kcLayer::kcLayer(const char *pszName)
{
	_pModel = NULL;
	_strName = pszName;
	_nHandle = 0;
}

kcLayer::~kcLayer(void)
{
	Destroy();
}

int  kcLayer::Initialize(kcModel *pModel)
{
	ASSERT(pModel);
	_pModel = pModel;

	GenHandleAndName();

	return 1;
}

void  kcLayer::GenHandleAndName()
{
	ASSERT(_pModel);
	//获取句柄
	kcHandleMgr *pHandleMgr = _pModel->GetHandleMgr();
	ASSERT(pHandleMgr);
	_nHandle = pHandleMgr->AllocHandle(KCT_ENT_LAYER);
	ASSERT(KC_IS_VALID_HANDLE(_nHandle));

	if(_strName.empty()){
		char cbuf[64];
		int len = sprintf_s(cbuf,64,"Layer %d",_nHandle);
		cbuf[len] = '\0';
		_strName = cbuf;
	}
}

void  kcLayer::Clear()
{
	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		(*it)->Destroy();
		delete *it;
	}
	_entityList.clear();
}

void  kcLayer::Destroy()
{
	// 释放句柄
	kcHandleMgr *pHandleMgr = _pModel->GetHandleMgr();
	ASSERT(pHandleMgr);
	pHandleMgr->FreeHandle(KCT_ENT_LAYER,_nHandle);
	_nHandle = 0;

	Clear();
}

// 添加图元
// 可能是新创建的图元，可能是undo时重新添加的图元,也可能是
//
BOOL	kcLayer::AddEntity(kcEntity *pEntity)
{
	ASSERT(pEntity);
	ASSERT(!FindEntity(pEntity));//不需不能存在
	if(!pEntity)
		return FALSE;

	//设置图层
	kcLayer *pLayer = pEntity->GetLayer();
	if(pLayer == NULL)
		pEntity->SetLayer(this);
	else
	{//应当是undo添加的
		ASSERT(pLayer == this);
	}
	_entityList.push_back(pEntity);

	//添加到modeltree中
	kuiModelTreeInterface *pModelTreeIF = _pModel->GetModelTreeInterface();
	if(pModelTreeIF){
		pModelTreeIF->AddEntity(_strName.c_str(),pEntity->GetName(),pEntity);
	}

	return TRUE;
}

// 删除图元
// 通常将保存在undo项中，因此，不能轻易破坏其内部数据。
//
kcEntity* kcLayer::DelEntity(kcEntity *pEntity)
{
	BOOL bFound = FALSE;
	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		if(*it == pEntity){
			_entityList.erase(it);
			pEntity->SetLayer(NULL);
			//从modeltree中删除
			kuiModelTreeInterface *pModelTreeIF = _pModel->GetModelTreeInterface();
			if(pModelTreeIF){
				pModelTreeIF->RemoveEntity(_strName.c_str(),pEntity->GetName());
			}

			bFound = TRUE;
			break;
		}
	}
	
	return bFound ? pEntity : NULL;
}

BOOL	kcLayer::FindEntity(kcEntity *pEntity)
{
	BOOL bFound = FALSE;
	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		if(*it == pEntity){
			bFound = TRUE;
			break;
		}
	}
	return bFound;
}

kcEntity*  kcLayer::FindEntity(const char *pszName)//根据名称获取
{
	std::string sName(pszName),snam;
	if(!sName.empty()){
		kcEntityList::iterator ite;
		for(ite = _entityList.begin();ite != _entityList.end();++ite){
			snam = (*ite)->GetName();
			if(snam == sName){
				return (*ite);
			}
		}
	}
	return NULL;
}

void  kcLayer::GetAllEntity(kcEntityList& entList)
{
	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		entList.push_back(*it);
	}
}

void  kcLayer::GetAllEntity(int entType,kcEntityList& entList)
{
	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		if((*it)->GetType() == entType)
			entList.push_back(*it);
	}
}

BOOL  kcLayer::ShowAll(BOOL bUpdateView)
{
	kuiModelTreeInterface *pModelTreeIF = _pModel->GetModelTreeInterface();
	kcEntityList::iterator it;

	for(it = _entityList.begin();it != _entityList.end();it ++){
		(*it)->Display(true,bUpdateView);
		//
		if(pModelTreeIF){
			pModelTreeIF->ShowEntity((*it)->GetName(),TRUE);
		}
	}
	return TRUE;
}

// 显示特定类型的对象
BOOL	kcLayer::FilterDisplay(int nTypeFlag,BOOL bUpdateView)
{
	kcEntityList::iterator it;
	kcEntity *pEnt = NULL;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		pEnt = (*it);
		int typ = pEnt->GetType();
		//仅仅取总类型
		typ = (typ >> 8) << 8;
		if(typ & nTypeFlag)
			(*it)->Display(true,bUpdateView);
		else
			(*it)->Display(false,bUpdateView);
	}

	return TRUE;
}

bool  kcLayer::HasName(const char *pszName)
{
	std::string ssName = pszName;
	if(ssName != _strName)
		return false;
	return true;
}

void  kcLayer::SetColor(double r,double g,double b,BOOL bUpdateView)
{
	kcEntityList::iterator it;
	kcEntity *pEnt = NULL;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		pEnt = (*it);
		pEnt->SetColor(r,g,b,FALSE);
	}
	//统一更新
	if(bUpdateView){
		UpdateCurrentViewer();
	}
}

void  kcLayer::SetLineStyle(eLineStyle eStyle,BOOL bUpdateView)
{
	kcEntityList::iterator it;
	kcEntity *pEnt = NULL;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		pEnt = (*it);
		pEnt->SetLineStyle(eStyle,FALSE);
	}
	//统一更新
	if(bUpdateView){
		UpdateCurrentViewer();
	}
}

void  kcLayer::SetLineWidth(double width,BOOL bUpdateView)
{
	kcEntityList::iterator it;
	kcEntity *pEnt = NULL;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		pEnt = (*it);
		pEnt->SetLineWidth(width,FALSE);
	}
	//统一更新
	if(bUpdateView){
		UpdateCurrentViewer();
	}
}


//计算自动捕捉点
BOOL  kcLayer::CalcSnapPoint(kiSnapMngr *pSnapMgr)
{
	if(_entityList.empty())
		return TRUE;

	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		(*it)->CalcSnapPoint(*pSnapMgr);
	}
	return TRUE;
}

// 读取
BOOL	kcLayer::Read(CArchive& ar,kcModel *pModel,UINT dwFileVer)
{
	ASSERT(ar.IsLoading());
	ASSERT(pModel);
	dwFileVer;
	
	_pModel = pModel;

	QxStgBlock blk;
	if(!blk.Read(ar,KSTG_BLOCK_LAYER)){
		ASSERT(FALSE);
		return FALSE;
	}

	int nbChild = 0;
	if(!blk.GetValueByCode(KSTG_CV_CODE_HANDLE,_nHandle) ||
		!blk.GetValueByCode(KSTG_CV_CODE_NAME,_strName) ||
		!blk.GetValueByCode(7,nbChild))
	{
		ASSERT(FALSE);
	}
	//标记已经使用了
	kcHandleMgr *pHandleMgr = _pModel->GetHandleMgr();
	pHandleMgr->MaskHandle(KCT_ENT_LAYER,_nHandle);

	//添加到modeltree
	kuiModelTreeInterface *pModelTreeIF = _pModel->GetModelTreeInterface();
	if(pModelTreeIF){
		pModelTreeIF->AddLayer(_strName.c_str(),this);
	}

	int idx;
	for(idx = 0;idx < nbChild;idx ++){
		if(!ReadEntity(ar)){
			Clear();
			return FALSE;
		}
	}

	//读结尾块
	if(!blk.Read(ar,KSTG_BLOCK_LAYER_END)){
		return FALSE;
	}
	
	return TRUE;
}

// 读取entity
BOOL	kcLayer::ReadEntity(CArchive& ar)
{
	kcEntity *pEntity = kcEntity::ReadEntity(ar);
	if(!pEntity)
		return FALSE;

	kcHandleMgr *pHandleMgr = _pModel->GetHandleMgr();
	pHandleMgr->MaskHandle(pEntity->GetType(),pEntity->GetHandle());

	AddEntity(pEntity);

	//显示
	pEntity->Display(true,FALSE);

	return TRUE;
}

// 写入
BOOL  kcLayer::Write(CArchive& ar)
{
	ASSERT(ar.IsStoring());

	QxStgBlock blk;

	//写入layer数据
	blk.Initialize(KSTG_BLOCK_LAYER);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,_nHandle);//tag == handle

	blk.AddCodeValue(KSTG_CV_CODE_NAME,_strName.c_str());//图层名称
	blk.AddCodeValue(KSTG_CV_CODE_HANDLE,_nHandle);//图层句柄
	int nbEnt = (int)_entityList.size();
	blk.AddCodeValue(7,nbEnt);

	blk.Write(ar);

	kcEntityList::iterator it;
	for(it = _entityList.begin();it != _entityList.end();it ++){
		(*it)->Write(ar);
	}

	//写入结尾块
	blk.Initialize(KSTG_BLOCK_LAYER_END);
	blk.AddCodeValue(KSTG_CV_CODE_BLOCK_TAG,_nHandle);//tag == handle
	blk.Write(ar);

	return TRUE;
}

void  kcLayer::UpdateCurrentViewer()
{
	if(_pModel != NULL){
		Handle(AIS_InteractiveContext) aCtx = _pModel->GetAISContext();
		if(!aCtx.IsNull()){
			aCtx->UpdateCurrentViewer();
		}
	}
}