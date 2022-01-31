//////////////////////////////////////////////////////////////////////////
//
#ifndef _KC_SNAP_MGR_H_
#define _KC_SNAP_MGR_H_

#include <vector>
#include <Graphic3d_Structure.hxx>
#include <Graphic3d_Group.hxx>
#include "kPoint.h"
#include "kiSnapMngr.h"

class CRect;
class kcModel;
class kcBasePlane;

// 记录一系列点
class QXLIB_API kiSnapPointSet{
public:
	kiSnapPointSet();

	bool				AddSnapPoint(const kiSnapPoint& sp);
	void				Clear();

	int					GetCount() const;
	const kiSnapPoint&	GetSnapPoint(int idx) const;

	bool				CalcScreenPoint(const Handle(V3d_View) &aView);

	void				DoSnap(int x,int y,int scope,std::vector<kiSnapPoint>& ss);

protected:
	std::vector<kiSnapPoint>	_spArray;//捕捉点列表
};



// 管理一个或多个捕捉点信息.
//
class QXLIB_API kcSnapMngr : public kiSnapMngr
{
public:
	kcSnapMngr(kcModel *pModel);
	~kcSnapMngr(void);

	//添加点
	bool					AddSnapPoint(int snapType,const kiSnapPoint& sp);
	bool					AddSnapPointSet(int snapType,const kiSnapPointSet& ss);
	void					Clear();
	//清除临时点
	void					ClearTempPoint();

	// 网格点捕捉相关函数
	bool					AddGridSnapPoint(const kiSnapPoint& sp);
	void					ClearGridSnapPoint();

	// 合并其他mgr
	bool					Merge(const kcSnapMngr& smgr);

	void					SetView(const Handle(V3d_View) &aView);

public:
	//计算一个曲线的自动捕捉点
	bool					CalcSnapPoint(const Handle(Geom_Curve)& aCurve);
	bool					CalcSnapPoint(const Handle(Geom_Curve)& aCurve,double t0,double t1);
	bool					CalcSnapPoint(const TopoDS_Edge& aEdge);
	bool					CalcSnapPoint(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace);

	//计算一个曲面的自动捕捉点
	bool					CalcSnapPoint(const TopoDS_Face& aFace);
	bool					CalcSnapPoint(const TopoDS_Shape& aShape);

public:
	void					SetSnapScope(int scope);
	bool					EnableSnap(int nFlag);
	bool					DisableSnap(int nFlag);
	bool					EnableAll();
	bool					DisableAll();

	// 计算屏幕坐标
	bool					CalcScreenPoint();

	// 进行捕捉,x,y是屏幕上两个点。
	void					DoSnap(int x,int y);
	bool					HasSnapPoint() const { return _bSnaped; }
	const kiSnapPoint&		GetSnapPoint();

public:
	void					HideSnapMarker();
	void					ShowSnapMarker();

protected:
	int						_nScope;//自动捕捉的范围大小。屏幕坐标，小于该距离认为捕捉成功。
	int						_nSnapFlag;//捕捉标志，表示当前允许捕捉的范围

public:
	kiSnapPointSet*			GetPointSet(int snapType) ;
	const kiSnapPointSet&	ExistPointSet() const;
	const kiSnapPointSet&	TermPointSet() const;
	const kiSnapPointSet&	MidPointSet() const;
	const kiSnapPointSet&	CenPointSet() const;
	const kiSnapPointSet&	TempPointSet() const;
	const kiSnapPointSet& QuaPointSet() const;

protected:
	kcBasePlane*				GetCurrentBasePlane();

protected:
	kiSnapPoint				_org;//原点
	kiSnapPointSet			_pntSet;//存在点列表
	kiSnapPointSet			_termSet;//端点列表
	kiSnapPointSet			_midSet;//中点列表
	kiSnapPointSet			_cenSet;//中心列表
	kiSnapPointSet			_tmpSet;//临时点列表,用于命令中的临时点.
	kiSnapPointSet			_quaSet;//四分点列表
	kiSnapPointSet			_gridSet;//网格上的捕捉点。

protected:
	bool					_bSnaped;//是否捕捉到。
	kiSnapPoint				_curSnapPoint;//当前捕捉点。

protected:
	Handle(Graphic3d_Structure)	_hSnapPntStru;
	Handle(Graphic3d_Group)		_hSnapPntGrp;

	kcModel						*pModel_;//关联的模型对象
	Handle(V3d_View)				aV3dView_;//记录所在的视图，用于坐标转换
};

inline const kiSnapPointSet&	kcSnapMngr::ExistPointSet() const
{
	return _pntSet;
}

inline const kiSnapPointSet&	kcSnapMngr::TermPointSet() const
{
	return _termSet;
}

inline const kiSnapPointSet&  kcSnapMngr::MidPointSet() const
{
	return _midSet;
}

inline const kiSnapPointSet&  kcSnapMngr::CenPointSet() const
{
	return _cenSet;
}

inline const kiSnapPointSet&  kcSnapMngr::TempPointSet() const
{
	return _tmpSet;
}

inline const kiSnapPointSet&  kcSnapMngr::QuaPointSet() const
{
	return _quaSet;
}

#endif