#include "StdAfx.h"
#include <BRepBuilderAPI_Transform.hxx>
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcEntityTrsf.h"

kcEntityTrsf::kcEntityTrsf(void)
{
}

kcEntityTrsf::~kcEntityTrsf(void)
{
}

// 移动对象,生成新的对象
kcEntity*	kcEntityTrsf::Move(kcEntity *pEntity,const kVector3& vec)
{
	ASSERT(pEntity && pEntity->IsValid());
	if(!pEntity || !pEntity->IsValid())
		return NULL;

	TopoDS_Shape aShape = pEntity->GetShape();
	if(aShape.IsNull())
		return NULL;

	kcEntity *pNewEntity = NULL;
	if(vec.length() < KDBL_MIN)
		pNewEntity = pEntity->Clone();
	else
	{
		TopoDS_Shape aNS;
		gp_Trsf trsf;
		trsf.SetTranslation(gp_Vec(vec[0],vec[1],vec[2]));
		try
		{
			BRepBuilderAPI_Transform trans(aShape,trsf,Standard_True);
			if(trans.IsDone())
				aNS = trans.Shape();
		}catch (Standard_Failure){
			return NULL;
		}
		if(aNS.IsNull())
			return NULL;

		kcEntityFactory efac;
		pNewEntity = efac.Create(aNS);
	}

	return pNewEntity;
}

// 旋转
// p:基点,dir:单位向量，ang：旋转角度，弧度。
kcEntity*	kcEntityTrsf::Rotate(kcEntity *pEntity,const kPoint3& p,const kVector3& dir,double ang)
{
	ASSERT(pEntity && pEntity->IsValid());
	if(!pEntity || !pEntity->IsValid())
		return NULL;

	TopoDS_Shape aShape = pEntity->GetShape();
	if(aShape.IsNull())
		return NULL;

	kcEntity *pNewEntity = NULL;
	if(fabs(ang) < KDBL_MIN)
		pNewEntity = pEntity->Clone();
	else
	{
		gp_Ax1 ax1;
		gp_Trsf trsf;
		ax1.SetLocation(gp_Pnt(p[0],p[1],p[2]));
		ax1.SetDirection(gp_Dir(dir[0],dir[1],dir[2]));
		trsf.SetRotation(ax1,ang);

		TopoDS_Shape aNS;
		try
		{
			BRepBuilderAPI_Transform trans(aShape,trsf,Standard_True);
			if(trans.IsDone())
				aNS = trans.Shape();
		}catch (Standard_Failure){
			return NULL;
		}
		if(aNS.IsNull())
			return NULL;

		kcEntityFactory efac;
		pNewEntity = efac.Create(aNS);
	}

	return pNewEntity;
}

// 缩放
kcEntity*	kcEntityTrsf::Scale(kcEntity *pEntity,const kPoint3& p,double scale)
{
	ASSERT(pEntity && pEntity->IsValid());
	if(!pEntity || !pEntity->IsValid())
		return NULL;

	TopoDS_Shape aShape = pEntity->GetShape();
	if(aShape.IsNull())
		return NULL;

	kcEntity *pNewEntity = NULL;
	if(fabs(scale - 1.0) < KDBL_MIN)
		pNewEntity = pEntity->Clone();
	else
	{
		TopoDS_Shape aNS;
		gp_Trsf trsf;
		trsf.SetScale(gp_Pnt(p[0],p[1],p[2]),scale);
		try
		{
			BRepBuilderAPI_Transform trans(aShape,trsf,Standard_True);
			if(trans.IsDone())
				aNS = trans.Shape();
		}catch (Standard_Failure){
			return NULL;
		}
		if(aNS.IsNull())
			return NULL;

		kcEntityFactory efac;
		pNewEntity = efac.Create(aNS);
	}

	return pNewEntity;
}