#include "StdAfx.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "kcLayer.h"
#include "kiUndoMgr.h"

//////////////////////////////////////////////////////////////////////////
//
kiNewDelUndoItem::kiNewDelUndoItem(const char *lpszName,kcModel *pModel)
:kiUndoItem(lpszName,pModel)
{

}

kiNewDelUndoItem::~kiNewDelUndoItem()
{
	_aNewEntity.clear();
	_aDelEntity.clear();
}

bool	kiNewDelUndoItem::Undo()
{
	if(_pModel == NULL) return false;
	// 删除新创建的对象
	int ix,isize = (int)_aNewEntity.size();
	for(ix = 0;ix < isize;ix ++)
		_pModel->DelEntity(_aNewEntity[ix]);

	// 添加删除的对象
	kcLayer *pLayer = NULL;
	isize = (int)_aDelEntity.size();
	for(ix = 0;ix < isize;ix ++)
	{
		pLayer = _aDelEntity[ix]->GetLayer();
		ASSERT(pLayer);
		_pModel->AddEntity(_aDelEntity[ix],pLayer);
	}
		
	return true;
}

bool	kiNewDelUndoItem::Redo()
{
	if(_pModel == NULL) return false;
	// 删除被删除的对象
	int ix,isize = (int)_aDelEntity.size();
	for(ix = 0;ix < isize;ix ++)
		_pModel->DelEntity(_aDelEntity[ix]);

	// 添加新创建的对象
	kcLayer *pLayer = NULL;
	isize = (int)_aNewEntity.size();
	for(ix = 0;ix < isize;ix ++)
	{
		pLayer = _aNewEntity[ix]->GetLayer();
		ASSERT(pLayer);
		_pModel->AddEntity(_aNewEntity[ix],pLayer);
	}

	return true;
}

bool	kiNewDelUndoItem::AddNewEntity(kcEntity *pEntity)
{
	int ix,isize = (int)_aNewEntity.size();
	for(ix = 0;ix < isize;ix ++)
	{
		if(_aNewEntity[ix] == pEntity)
			return false;
	}
	_aNewEntity.push_back(pEntity);
	return true;
}

bool	kiNewDelUndoItem::AddDelEntity(kcEntity *pEntity)
{
	int ix,isize = (int)_aDelEntity.size();
	for(ix = 0;ix < isize;ix ++)
	{
		if(_aDelEntity[ix] == pEntity)
			return false;
	}
	_aDelEntity.push_back(pEntity);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
kiModifyUndoItem::kiModifyUndoItem(const char *lpszName,kcModel *pModel,kiCommand *pCommand)
:kiUndoItem(lpszName,pModel)
{
}

kiModifyUndoItem::~kiModifyUndoItem()
{

}

bool	kiModifyUndoItem::Undo()
{
	return true;
}

bool	kiModifyUndoItem::Redo()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
kiUndoMgr::kiUndoMgr(kcModel *pModel)
:m_pModel(pModel)
{
	ASSERT(m_pModel);

	m_nUndoLimit = 500;
	m_nRedoLimit = 500;
}

kiUndoMgr::~kiUndoMgr(void)
{
	UnInitialize();
}

bool	kiUndoMgr::Initialize()
{
	return true;
}

bool	kiUndoMgr::UnInitialize()
{
	ClearAllItems();

	return true;
}

// 压入一个undo项。如果满了，则去除最旧的，添加新的。
//
bool	kiUndoMgr::Push(kiUndoItem *pItem)
{
	ASSERT(pItem);
	if(!pItem) return false;

	int isize = (int)m_undoStack.size();
	if(isize >= m_nUndoLimit)//检查是否满
	{
		kiUndoItem *pOldItem = m_undoStack.back();
		m_undoStack.pop_back();
		delete pOldItem;
	}
	//压入到起始
	m_undoStack.push_front(pItem);

	return true;
}

bool	kiUndoMgr::PushRedo(kiUndoItem *pItem)
{
	ASSERT(pItem);
	int isize = (int)m_redoStack.size();
	if(isize >= m_nRedoLimit)
	{
		kiUndoItem *pOldItem = m_redoStack.back();
		m_redoStack.pop_back();
		delete pOldItem;
	}
	m_redoStack.push_front(pItem);
	return true;
}

// undo一步。
//
bool	kiUndoMgr::Undo()
{
	if(m_undoStack.empty())
		return true;

	kiUndoItem *pItem = m_undoStack.front();
	m_undoStack.pop_front();
	//执行undo
	if(pItem->Undo())
	{
		//作为redo
		PushRedo(pItem);
	}
	else
	{
		//执行失败，则删除
		delete pItem;
		return false;
	}
	
	return true;
}

// redo一步
//
bool	kiUndoMgr::Redo()
{
	if(m_redoStack.empty())
		return true;

	kiUndoItem *pItem = m_redoStack.front();
	m_redoStack.pop_front();
	//
	if(pItem->Redo())
	{
		Push(pItem);
	}
	else
	{
		delete pItem;
		return false;
	}

	return true;
}

bool	kiUndoMgr::CanUndo() const
{
	return (m_undoStack.empty() ? false : true);
}

bool	kiUndoMgr::CanRedo() const
{
	return (m_redoStack.empty() ? false : true);
}

//清除所有
void	kiUndoMgr::ClearAllItems()
{
	kiUndoItemStack::iterator ite;
	if(!m_undoStack.empty())
	{
		for(ite = m_undoStack.begin();ite != m_undoStack.end();ite ++)
			delete *ite;
		m_undoStack.clear();
	}
	
	if(!m_redoStack.empty())
	{
		for(ite = m_redoStack.begin();ite != m_redoStack.end();ite ++)
			delete *ite;
		m_redoStack.clear();
	}
}

void	kiUndoMgr::Clear()
{
	ClearAllItems();
}