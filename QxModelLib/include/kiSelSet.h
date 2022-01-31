//////////////////////////////////////////////////////////////////////////
// //选择集管理
#ifndef _KI_SELSET_H_
#define _KI_SELSET_H_

#include <vector>
#include <list>
#include <Quantity_NameOfColor.hxx>
#include "QxLibDef.h"

class kcEntity;
class kiSelEntity;
class kcModel;

// 这里直接使用对象，不是指针
typedef std::list<kiSelEntity>    kiSelEntityList;
typedef std::vector<kiSelEntity>  kiSelEntityArray;

//一个选择项，目前支持：
// 拾取一个自然拓扑对象
// 拾取一个局部对象，例如：实体中的一条边
//
class QXLIB_API kiSelEntity{
public:
	kiSelEntity();
	kiSelEntity(kcEntity *pEntity,const Handle(AIS_InteractiveObject)& hObj);
	kiSelEntity(const kiSelEntity& other);

	bool					IsValid() const;//是否有效的拾取项

	void					Set(kcEntity *pEntity,const Handle(AIS_InteractiveObject)& hObj);
	void					SetEntityOwner(const Handle(SelectMgr_EntityOwner) &aOwner);
	void					SetLocalShape(const TopoDS_Shape &aSLoc);

	kcEntity*				Entity() const { return _pEntity; }
	const Handle(AIS_InteractiveObject)  AISObject() const { return _aAISObj;}
	bool					IsLocalShape() const;
	const Handle(SelectMgr_EntityOwner)  EntityOwner() const { return _aEntOwner;}
	const TopoDS_Shape		LocalShape() const { return _aLocShape; }

	//kcEntity对应的shape对象
	const TopoDS_Shape		EntityShape() const;

	//拾取的对象，自然对象或局部对象，如果是局部对象，则是局部对象对应的shape对象
	const TopoDS_Shape		SelectShape() const;

	kiSelEntity&			operator=(const kiSelEntity& other);
	bool					operator==(const kiSelEntity& other);

public:
	// 选中对象对应的自然对象，如果不是局部对象，选中的就是自身
	kcEntity						*_pEntity;

protected:
	//拓扑对象对应的AIS显示对象，_pEntity,_aAISObj通常不为空
	Handle(AIS_InteractiveObject)	_aAISObj;
	//选中对象的owner,对局部对象，是局部对象的owner，对全局对象是全局对象的owner。
	Handle(SelectMgr_EntityOwner)	_aEntOwner;
	//是否局部对象,即拓扑对象中的子对象，例如：实体中的面等
	bool							_bLocalShape;
	TopoDS_Shape					_aLocShape;//选中局部对象的shape对象，对局部对象不为空，否则为空
};

// 一个选择集
// 记录一系列选择对象，并负责高亮显示
//
class QXLIB_API kiSelSet
{
public:
	//kiSelSet(void);
	kiSelSet(kcModel *pModel);
	~kiSelSet(void);

	//获取选中项,通常用于拓扑对象的选择
	bool					GetSelected();
	void					Clear();

	//添加选中项
	bool					AddSelected(const Handle(AIS_InteractiveObject)& aisObj,BOOL bUpdateView = FALSE);
	bool					RemoveSelected(const Handle(AIS_InteractiveObject)& aisObj,BOOL bUpdateView = FALSE);
	bool					AddSelected(const kiSelEntity& selEnt,BOOL bUpdateView = FALSE);
	bool					AddSelected(kiSelSet& selSet,BOOL bUpdateView = FALSE);

	//高亮和不高亮显示
	bool					Hilight(BOOL bUpdateView = TRUE);//高亮显示，使用默认颜色
	bool					Hilight(const Quantity_NameOfColor hicol,BOOL bUpdateView = TRUE);
	void					UnHilight(BOOL bUpdateView = TRUE);

	bool					IsEmpty() const;
	int					GetSelCount();
	void					InitSelected();
	bool					MoreSelected();
	void					NextSelected();
	kiSelEntity			CurSelected();
	//
	kiSelEntity			GetFirstSelected() const;
	kiSelEntity			GetSelEntity(int idx) const;

public:
	Handle(AIS_InteractiveContext)	m_hAISCtx;
	kcModel				*pModel_;//模型对象
	kiSelEntityList		m_selEntList;//选择对象列表

protected:
	kiSelEntityList::iterator	m_iteList;
};



#endif
