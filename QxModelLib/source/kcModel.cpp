#include "StdAfx.h"
#include <AIS_InteractiveObject.hxx>

#include "kcEntity.h"
#include "kiSnapMngr.h"
#include "kcLayer.h"
#include "kiUndoMgr.h"
#include "kiSelSet.h"
#include "kcHandleMgr.h"
#include "kcShapeAttribMgr.h"
#include "QxStgBlock.h"
#include "kuiInterface.h"
#include "kvGrid.h"
#include "kcBasePlane.h"
#include "kcModel.h"

kcModel::kcModel(const Handle(AIS_InteractiveContext)& aCtx)
{
	_pHandleMgr = NULL;
	_pCurLayer = NULL;
	_pCurBasePlane = NULL;

	_pShapeAttribMgr = NULL;

	_pUndoMgr = NULL;
	_pCurrUndoItem = NULL;

	pGlobalSelSet_ = NULL;
	pSnapMngr_ = NULL;

	_pModelTreeInterface = NULL;

	_hAISContext = aCtx;
	dwFileVersion_ = 0;
}

kcModel::~kcModel(void)
{
	Destroy();
}

//初始化
BOOL kcModel::Initialize()
{
	_pHandleMgr = new kcHandleMgr;
	_pHandleMgr->Initialize();

	//初始化Undo管理器
	_pUndoMgr = new kiUndoMgr(this);
	if(!_pUndoMgr)
		return FALSE;
	_pUndoMgr->Initialize();

	_pShapeAttribMgr = new kcShapeAttribMgr;

	//全局选择集对象
	pGlobalSelSet_ = new kiSelSet(this);
	
	_pCurrUndoItem = NULL;
		
	return TRUE;
}

//情况模型
void  kcModel::Clear()
{
	if(_pUndoMgr)
		_pUndoMgr->Clear();
	_pCurrUndoItem = NULL;

	// 删除所有图层数据
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++){
		(*it)->Clear();
		delete *it;
	}
	_layerList.clear();
	_pCurLayer = NULL;

	//清除所有基准面数据
	kcBasePlaneList::iterator bpite = _basePlaneList.begin();
	for(;bpite != _basePlaneList.end();bpite ++){
		delete *bpite;
	}
	_basePlaneList.clear();
	_pCurBasePlane = NULL;

	if(_pShapeAttribMgr){
		_pShapeAttribMgr->Clear();
	}

	//
	if(_pHandleMgr){
		_pHandleMgr->Clear();
	}

	if(pGlobalSelSet_){
		pGlobalSelSet_->Clear();
	}
}

void  kcModel::Destroy()
{
	Clear();

	KC_SAFE_DELETE(_pHandleMgr);
	//释放Undo管理器。
	KC_SAFE_DELETE(_pUndoMgr);
	//
	KC_SAFE_DELETE(_pShapeAttribMgr);
	KC_SAFE_DELETE(pGlobalSelSet_);
}

BOOL	kcModel::CreateDefaultLayer()
{
	kcLayer *pLayer = CreateNewLayer();
	if(pLayer == NULL)
		return FALSE;

	SetCurrLayer(pLayer);

	return TRUE;
}

//创建缺省的XY、YZ、XZ基准面
BOOL  kcModel::CreateDefaultBasePlane(const Handle(V3d_View)& aView)
{
	kvCoordSystem cs,cs2;
	kcBasePlane *pBasePlane = NULL,*pCurr = NULL;

	//xy基准面
	pCurr = pBasePlane = new kcBasePlane(aView);
	pBasePlane->Create(cs,this,"XY基准面");
	AddBasePlane(pBasePlane);

	//YZ基准面
	cs2.Set(cs.Org(),cs.Y(),cs.Z(),cs.X());
	pBasePlane = new kcBasePlane(aView);
	pBasePlane->Create(cs2,this,"YZ基准面");
	AddBasePlane(pBasePlane);

	//ZX基准面
	cs2.Set(cs.Org(),cs.Z(),cs.X(),cs.Y());
	pBasePlane = new kcBasePlane(aView);
	pBasePlane->Create(cs2,this,"ZX基准面");
	AddBasePlane(pBasePlane);

	SetCurrentBasePlane(pCurr);

	return TRUE;
}


// 仅仅程序启动和结束时,调用一次.目前如此处理.
//
void	kcModel::SetModelTreeInterface(kuiModelTreeInterface *pModelTreeInterface)
{
	_pModelTreeInterface = pModelTreeInterface;
	if(pModelTreeInterface){
		_pModelTreeInterface->SetModel(this);
		_pModelTreeInterface->Initialize();

		// 将现有的项添加到modeltree中.可能已经有默认图层了.
		kcLayerList::iterator it = _layerList.begin();
		for(;it != _layerList.end();it ++){
			kcLayer *pLayer = *it;
			_pModelTreeInterface->AddLayer(pLayer->_strName.c_str(),pLayer);
		}
	}
}

kiSelSet* kcModel::GetGlobalSelSet() const
{
	return pGlobalSelSet_;
}

void kcModel::SetSnapManager(kcSnapMngr *pMngr)
{
	pSnapMngr_ = pMngr;
}

kcSnapMngr* kcModel::GetSnapManager() const
{
	return pSnapMngr_;
}

kcLayer* kcModel::CreateNewLayer()
{
	kcLayer *pLayer = new kcLayer;
	pLayer->Initialize(this);
	_layerList.push_back(pLayer);

	// 添加到modeltree中
	if(_pModelTreeInterface){
		_pModelTreeInterface->AddLayer(pLayer->GetLayerName().c_str(),pLayer);
	}

	return pLayer;
}

//创建和删除图层
kcLayer*	kcModel::CreateLayer(const char* szLayerName)
{
	if(FindLayer(szLayerName) != NULL){
		ASSERT(FALSE);
		return NULL;
	}

	kcLayer *pLayer = new kcLayer(szLayerName);
	pLayer->Initialize(this);
	_layerList.push_back(pLayer);

	// 添加到modeltree中
	if(_pModelTreeInterface){
		_pModelTreeInterface->AddLayer(pLayer->GetLayerName().c_str(),pLayer);
	}

	return pLayer;
}

BOOL	kcModel::DeleteLayer(kcLayer *pLayer)
{
	return TRUE;
}

kcLayer*	kcModel::DeleteLayer(const char* szLayerName)
{
	return NULL;
}

kcLayer*	kcModel::FindLayer(const char* szLayerName)
{
	std::string ssName = szLayerName;
	if(ssName.empty())
		return NULL;

	kcLayer *pLayer = NULL;
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++){
		if((*it)->HasName(szLayerName)){
			pLayer = *it;
			break;
		}
	}
	return pLayer;
}

BOOL	kcModel::SetCurrLayer(const char* szLayerName)
{
	kcLayer *pLayer = FindLayer(szLayerName);
	if(pLayer == NULL)
		return FALSE;
	
	return SetCurrLayer(pLayer);
}

BOOL	kcModel::SetCurrLayer(kcLayer *pLayer)
{
	if(!pLayer)
		return FALSE;

	if(pLayer == _pCurLayer)
		return TRUE;

	_pCurLayer = pLayer;
	if(_pModelTreeInterface){
		_pModelTreeInterface->UnBoldAllItem(eLayerItem);
		_pModelTreeInterface->BoldTreeItem(eLayerItem,pLayer->GetLayerName().c_str());
	}

	return TRUE;
}

kcLayer*	kcModel::GetCurrLayer() const
{
	return _pCurLayer;
}

const kcLayerList& kcModel::GetLayerList() const
{
	return _layerList;
}

int kcModel::GetLayerCount() const
{
	return (int)_layerList.size();
}

// 添加对象到当前图层
BOOL kcModel::AddEntity(kcEntity *pEntity,BOOL bUpdateView)
{
	if(!_pCurLayer)
		return FALSE;

	return AddEntity(pEntity,_pCurLayer,bUpdateView);
}

BOOL	kcModel::AddEntity(const std::vector<kcEntity *>& aEnt,BOOL bUpdateView)
{
	if(!_pCurLayer)
		return FALSE;

	int ix,isize = (int)aEnt.size();
	for(ix = 0;ix < isize;ix ++)
		AddEntity(aEnt[ix],bUpdateView);

	return TRUE;
}

// 实际的添加和显示实现
BOOL	kcModel::AddEntity(kcEntity *pEntity,kcLayer *pLayer,BOOL bUpdateView)
{
	if(!pEntity || !pLayer || !pEntity->IsValid())
		return FALSE;

	//判断图层是否存在和一致。
	const std::string& sname = pLayer->GetLayerName();
	kcLayer *pFoundLayer = FindLayer(sname.c_str());
	if(pFoundLayer == NULL || pFoundLayer != pLayer){
		KTRACE("\n addEntity layer %s not exist.",sname.c_str());
		ASSERT(FALSE);
		return FALSE;
	}

	// 添加到图层
	if(!pLayer->AddEntity(pEntity))
		return FALSE;

	// 这里，由于可能没有显示对象，因此显示对象和entity的绑定在
	// Display中进行。
	// 显示对象
	if(!pEntity->Display(true,bUpdateView)){
		DelEntity(pEntity);
		return FALSE;
	}

	//至此，添加成功,如果存在undo过程，记录。
	if(_pCurrUndoItem){
		_pCurrUndoItem->AddNewEntity(pEntity);
	}

	return TRUE;
}

BOOL	kcModel::DelEntity(kcEntity *pEntity,BOOL bUpdateView)
{
	if(!pEntity || !pEntity->IsValid())
		return FALSE;

	kcLayer *pLayer = pEntity->GetLayer();
	if(pLayer == NULL){
		KTRACE("\n DelEntity layer not exist.");
		ASSERT(FALSE);
		return FALSE;
	}

	pLayer->DelEntity(pEntity);
	// 从显示中删除
	pEntity->Display(false,bUpdateView);

	//是否undo过程
	if(_pCurrUndoItem){
		_pCurrUndoItem->AddDelEntity(pEntity);
	}

	return TRUE;
}

kcEntity*	kcModel::DelEntity(Handle(AIS_InteractiveObject)& aObj,BOOL bUpdateView)
{
	kcEntity *pEntity = GetEntity(aObj);
	if(pEntity == NULL)
		return NULL;

	DelEntity(pEntity,bUpdateView);

	return pEntity;
}

// 根据显示对象，查找对应的entity.
//
kcEntity*	kcModel::GetEntity(Handle(AIS_InteractiveObject)& aObj)
{
	return ksGetAisShapeEntity(aObj);
}

//BOOL	kcModel::SetAISContext(const Handle(AIS_InteractiveContext)& aCtx)
//{
//	if(aCtx.IsNull())
//		return FALSE;
//	_hAISContext = aCtx;
//	return TRUE;
//}

BOOL  kcModel::SetActiveView(const Handle(V3d_View)& aView)
{
	if(!aView.IsNull()){
		aActiveView_ = aView;
		return TRUE;
	}
	return FALSE;
}

//和undo相关接口。开始和结束undo，在两个函数成对调用。在这两个函数间
//AddEntity和DelEntity将自动由生成的Undo项保存。
//
BOOL	kcModel::BeginUndo(const char *pszUndoName)
{
	ASSERT(pszUndoName);
	ASSERT(_pCurrUndoItem == NULL);//必须没有其他的undo再做。

	_pCurrUndoItem = new kiNewDelUndoItem(pszUndoName,this);
	if(!_pCurrUndoItem)
		return FALSE;

	return TRUE;
}

BOOL	kcModel::EndUndo()
{
	if(!_pCurrUndoItem) 
		return FALSE;

	//记录到undo管理器中
	_pUndoMgr->Push(_pCurrUndoItem);
	_pCurrUndoItem = NULL;//标识没有undo存在。

	return TRUE;
}

//计算所有捕捉点
BOOL	kcModel::CalcSnapPoint(kiSnapMngr *pSnapMgr)
{
	ASSERT(pSnapMgr);
	pSnapMgr->Clear();

	kcLayer *pLayer = NULL;
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++)
	{
		(*it)->CalcSnapPoint(pSnapMgr);
	}
	return TRUE;
}

void  kcModel::GetAllEntity(kcEntityList& entList)
{
	kcLayerList::iterator it = _layerList.begin();
	for( ;it != _layerList.end();it ++){
		(*it)->GetAllEntity(entList);
	}
}

void  kcModel::GetAllEntity(int entType,kcEntityList& entList)
{
	kcLayerList::iterator it = _layerList.begin();
	for( ;it != _layerList.end();it ++){
		(*it)->GetAllEntity(entType,entList);
	}
}

//显示所有对象
BOOL  kcModel::ShowAll(BOOL bUpdateView)
{
	kcLayer *pLayer = NULL;
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++)
	{
		(*it)->ShowAll(bUpdateView);
	}
	return TRUE;
}

// 显示特定类型的对象
BOOL  kcModel::FilterDisplay(int nTypeFlag,BOOL bUpdateView)
{
	kcLayer *pLayer = NULL;
	kcLayerList::iterator it = _layerList.begin();
	for(;it != _layerList.end();it ++)
	{
		(*it)->FilterDisplay(nTypeFlag,bUpdateView);
	}
	return TRUE;
}

/////////////////////////////////////
//
BOOL  kcModel::AddBasePlane(kcBasePlane *pBasePlane)
{
	ASSERT(pBasePlane);
	ASSERT(pBasePlane->HasName());
	
	_basePlaneList.push_back(pBasePlane);
	
	if(_pModelTreeInterface){
		std::string szName;
		szName = pBasePlane->GetName();
		_pModelTreeInterface->AddBasePlane(szName.c_str(),pBasePlane);
	}

	return TRUE;
}

BOOL  kcModel::SetCurrentBasePlane(kcBasePlane *pBasePlane)
{
	ASSERT(pBasePlane);
	if(_pCurBasePlane == pBasePlane)
		return TRUE;

	if(_pCurBasePlane)
		_pCurBasePlane->SetActive(false);

	pBasePlane->SetActive(true);
	_pCurBasePlane = pBasePlane;
	//
	if(_pModelTreeInterface){//添加处理
		std::string szName = pBasePlane->GetName();
		_pModelTreeInterface->UnBoldAllItem(eBasePlaneItem);

 		_pModelTreeInterface->BoldTreeItem(eBasePlaneItem,szName.c_str());
	}

	return TRUE;
}

kcBasePlane*  kcModel::GetCurrentBasePlane() const
{
	return _pCurBasePlane;
}

// 显示所以基准面的Grid
void kcModel::ShowAllBasePlaneGrid(bool bShow)
{
	kvGrid *pGrid = NULL;
	kcBasePlaneList::iterator ite = _basePlaneList.begin();
	for(;ite != _basePlaneList.end();++ite){
		pGrid = (*ite)->GetGrid();
		if(pGrid){
			pGrid->SetVisible(bShow);
		}
	}
}

//shape属性管理
BOOL kcModel::AddShapeAttrib(const TopoDS_Shape &aS,const Quantity_Color &aCol)
{
	if(_pShapeAttribMgr){
		if(_pShapeAttribMgr->AddColorAttrib(aS,aCol)){
			return TRUE;
		}
	}
	return FALSE;
}

//查找颜色属性
BOOL  kcModel::FindShapeAttrib(const TopoDS_Shape &aS,Quantity_Color &aCol)
{
	if(_pShapeAttribMgr){
		if(_pShapeAttribMgr->FindColorAttrib(aS,aCol)){
			return TRUE;
		}
	}
	return FALSE;
}
//
void  kcModel::ClearAllShapeAttrib()
{
	if(_pShapeAttribMgr){
		_pShapeAttribMgr->Clear();
	}
}