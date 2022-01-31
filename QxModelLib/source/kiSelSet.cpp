#include "StdAfx.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "AIS_EntityShape.h"
#include "kiSelSet.h"

//一个选择项
kiSelEntity::kiSelEntity()
{
	_pEntity = NULL;
	_bLocalShape = false;
}

kiSelEntity::kiSelEntity(kcEntity *pEntity,const Handle(AIS_InteractiveObject)& hObj)
{
	ASSERT(pEntity);
	ASSERT(!hObj.IsNull());
	_pEntity = pEntity;
	_bLocalShape = false;
	_aAISObj = hObj;
}

kiSelEntity::kiSelEntity(const kiSelEntity& other)
{
	_pEntity = NULL;
	*this = other;
}

//是否有效的拾取项
bool  kiSelEntity::IsValid() const
{
	if(_pEntity == NULL || _aAISObj.IsNull() || _aEntOwner.IsNull())
		return false;
	
	if(_bLocalShape && _aLocShape.IsNull())
		return false;

	return true;
}

void  kiSelEntity::Set(kcEntity *pEntity,const Handle(AIS_InteractiveObject)& hObj)
{
	ASSERT(pEntity);
	ASSERT(!hObj.IsNull());
	_pEntity = pEntity;
	_aAISObj = hObj;
}

void  kiSelEntity::SetEntityOwner(const Handle(SelectMgr_EntityOwner) &aOwner)
{
	ASSERT(!aOwner.IsNull());
	_aEntOwner = aOwner;
}

void  kiSelEntity::SetLocalShape(const TopoDS_Shape &aSLoc)
{
	ASSERT(!aSLoc.IsNull());
	_bLocalShape = true;
	_aLocShape = aSLoc;
}

kiSelEntity&  kiSelEntity::operator=(const kiSelEntity& other)
{
	_pEntity = other.Entity();
	_aAISObj = other.AISObject();
	_aEntOwner = other.EntityOwner();
	_bLocalShape = other.IsLocalShape();
	if(other.IsLocalShape()){
		_aLocShape = other.LocalShape();
	}

	return *this;
}

bool  kiSelEntity::IsLocalShape() const
{
	return _bLocalShape;
}

const TopoDS_Shape  kiSelEntity::EntityShape() const
{
	TopoDS_Shape aS;
	if(_pEntity != NULL){
		aS = _pEntity->GetShape();
	}
	return aS;
}

//拾取的对象，自然对象或局部对象
const TopoDS_Shape  kiSelEntity::SelectShape() const
{
	TopoDS_Shape aS = EntityShape();
	if(_bLocalShape){
		aS = _aLocShape;
	}
	return aS;
}

bool  kiSelEntity::operator==(const kiSelEntity& other)
{
	if(_pEntity == other.Entity() && 
		_aAISObj == other.AISObject() &&
		(_aEntOwner == other.EntityOwner()) &&
		_bLocalShape == other.IsLocalShape())
	{
		if(_bLocalShape){//局部对象
			if(_aLocShape.IsSame(other.LocalShape()))
				return true;
		}else{
			return true;
		}
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//kiSelSet::kiSelSet(void)
//{
//	m_hAISCtx = glb_GetAISContext();
//	ASSERT(!m_hAISCtx.IsNull());
//	pModel_ = NULL;
//}

kiSelSet::kiSelSet(kcModel *pModel)
{
	ASSERT(pModel);
	pModel_ = pModel;
	m_hAISCtx = pModel->GetAISContext();
}

kiSelSet::~kiSelSet(void)
{
	Clear();
}

// 从Ctx中获取选中项.这里主要还是自然对象的选择，没有具体的拾取模式
//
bool	kiSelSet::GetSelected()
{
	//清除旧的。
	m_selEntList.clear();
	//
	if(m_hAISCtx.IsNull())
		return false;

	kcEntity *pEntity = NULL;
	Handle(AIS_InteractiveObject) hObj;

	m_hAISCtx->InitSelected();
	while(m_hAISCtx->MoreSelected()){
		hObj = m_hAISCtx->SelectedInteractive();
		//
		if(!hObj.IsNull()){
			pEntity = pModel_->GetEntity(hObj);
			if(pEntity){
				kiSelEntity se(pEntity,hObj);
				se.SetEntityOwner(m_hAISCtx->SelectedOwner());
				m_selEntList.push_back(se);
			}
		}

		m_hAISCtx->NextSelected();
	}

	return true;
}

//清楚选择集和显示
void  kiSelSet::Clear()
{
	if(!m_selEntList.empty()){
		m_selEntList.clear();
	}

	//if(!m_hAISCtx.IsNull()){
	//	m_hAISCtx->ClearSelected(true);
	//}
}

// 添加一个对象到选择集
// 这个通常是添加自然对象。
//
bool  kiSelSet::AddSelected(const Handle(AIS_InteractiveObject)& aisObj,BOOL bUpdateView)
{
	if(m_hAISCtx.IsNull() || aisObj.IsNull())	return false;

	//如果没有选中，添加到选择集中
	if(!m_hAISCtx->IsSelected(aisObj)){
		m_hAISCtx->AddOrRemoveSelected(aisObj,bUpdateView ? true : false);
	}

	//添加到列表中
	kcEntity *pEntity = ksGetAisShapeEntity(aisObj);
	if(pEntity){
		kiSelEntityList::iterator it = m_selEntList.begin();
		for(;it != m_selEntList.end();it ++){
			if((*it).Entity() == pEntity){//已经添加
				return true;
			}
		}
		kiSelEntity se(pEntity,aisObj);
		se.SetEntityOwner(aisObj->GlobalSelOwner());

		m_selEntList.push_back(se);
	}
	
	return true;
}

bool	kiSelSet::RemoveSelected(const Handle(AIS_InteractiveObject)& aisObj,BOOL bUpdateView)
{
	if(m_hAISCtx.IsNull() || aisObj.IsNull()) return false;

	//如果在选择集中，去选择
	if(m_hAISCtx->IsSelected(aisObj)){
		m_hAISCtx->AddOrRemoveSelected(aisObj,bUpdateView ? true : false);
	}

	//从list中删除
	kcEntity *pEntity = ksGetAisShapeEntity(aisObj);
	if(pEntity){
		kiSelEntityList::iterator it = m_selEntList.begin();
		for(;it != m_selEntList.end();it ++){
			if((*it).Entity() == pEntity)//已经添加
				break;
		}
		if(it != m_selEntList.end())
			m_selEntList.erase(it);
	}
	
	return true;
}

// 添加选中项
// 注意：这里自然对象和局部对象的都基于owner处理
//
bool  kiSelSet::AddSelected(const kiSelEntity& selEnt,BOOL bUpdateView)
{
	if(!selEnt.IsValid()){
		ASSERT(FALSE);
		return false;
	}

	//局部和全局对象都基于owner处理
	if(!m_hAISCtx->IsSelected(selEnt.EntityOwner())){
		m_hAISCtx->AddOrRemoveSelected(selEnt.EntityOwner(),bUpdateView ? true : false);
	}
	
	kiSelEntityList::iterator it = m_selEntList.begin();
	for(;it != m_selEntList.end();it ++)
	{
		if((*it) == selEnt )//已经添加
			return true;
	}
	m_selEntList.push_back(selEnt);
	
	return true;
}

bool	kiSelSet::AddSelected(kiSelSet& selSet,BOOL bUpdateView)
{
	selSet.InitSelected();
	while(selSet.MoreSelected())
	{
		AddSelected(selSet.CurSelected(),FALSE);
		selSet.NextSelected();
	}

	if(bUpdateView){
		m_hAISCtx->UpdateCurrentViewer();
	}

	return true;
}

//高亮和不高亮显示
bool  kiSelSet::Hilight(BOOL bUpdateView)
{
	if(m_selEntList.empty() || m_hAISCtx.IsNull())
		return false;

	//如果对象没有在选择集中，需要添加对象
	kiSelEntityList::iterator it = m_selEntList.begin();
	for(;it != m_selEntList.end();it ++){
		//全局或局部对象的处理都基于owner
		Handle(SelectMgr_EntityOwner) aOwner = (*it).EntityOwner();
		if(!aOwner.IsNull() && !m_hAISCtx->IsSelected(aOwner)){
			m_hAISCtx->AddOrRemoveSelected(aOwner,false);
		}
	}
	
	//高亮显示
	m_hAISCtx->HilightSelected(bUpdateView ? true : false);

	return true;
}

//高亮和不高亮显示
bool	kiSelSet::Hilight(const Quantity_NameOfColor hicol,BOOL bUpdateView)
{
	if(m_selEntList.empty() || m_hAISCtx.IsNull())
		return false;

	//如果对象没有在选择集中，需要添加对象
	kiSelEntityList::iterator it = m_selEntList.begin();
	for(;it != m_selEntList.end();it ++){
		//全局或局部对象的处理都基于owner
		Handle(SelectMgr_EntityOwner) aOwner = (*it).EntityOwner();
		if(!aOwner.IsNull() && !m_hAISCtx->IsSelected(aOwner)){
			m_hAISCtx->AddOrRemoveSelected(aOwner,false);
		}
	}
	
	//高亮显示
	m_hAISCtx->HilightSelected(bUpdateView ? true : false);

	return true;
}

void  kiSelSet::UnHilight(BOOL bUpdateView)
{
	if(m_selEntList.empty() || m_hAISCtx.IsNull())
		return;

	m_hAISCtx->UnhilightSelected(bUpdateView ? true : false);
}

bool	kiSelSet::IsEmpty() const
{
	return m_selEntList.empty();
}

int		kiSelSet::GetSelCount()
{
	int nCount = 0;
	kiSelEntityList::iterator it = m_selEntList.begin();
	for(;it != m_selEntList.end();it ++)
		nCount ++;
	return nCount;
}

void  kiSelSet::InitSelected()
{
	m_iteList = m_selEntList.begin();
}

bool  kiSelSet::MoreSelected()
{
	return (m_iteList != m_selEntList.end() ? true : false);
}
void  kiSelSet::NextSelected()
{
	m_iteList ++;
}

kiSelEntity  kiSelSet::CurSelected()
{
	return (*m_iteList);
}

kiSelEntity kiSelSet::GetFirstSelected() const
{
	ASSERT(!m_selEntList.empty());
	return m_selEntList.front(); 
}

kiSelEntity kiSelSet::GetSelEntity(int idx) const
{
	kiSelEntity se;
	kiSelEntityList::const_iterator ite = m_selEntList.begin();
	int ix = 0;
	for(;ite != m_selEntList.end();++ite,ix++){
		if(ix == idx){
			se = (*ite);
			break;
		}
	}
	return se;
}

////////////////////////////////////////////////////////
