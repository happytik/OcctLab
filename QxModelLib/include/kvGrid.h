//////////////////////////////////////////////////////////////////////////
// 工作平面上的网格表示
//
#ifndef _KV_GRID_H_
#define _KV_GRID_H_

#include "QxLibDef.h"

// 网格的类型
#define KVT_GRID_LINE		1 //直线型的网格
#define KVT_GRID_DOT		2 //点型网格。

class kcBasePlane;
class kvWorkPlane;
class kiSnapMngr;

class QXLIB_API kvGrid
{
public:
	kvGrid(const Handle(AIS_InteractiveContext)& aCtx,kcBasePlane *pBasePlane);
	~kvGrid(void);

	void				SetType(int type);
	void				SetVisible(bool bVisible);
	void				SetWidth(double width);
	void				SetHeight(double height);
	void				SetCellSize(double size);

	bool				IsVisible() const { return m_bVisible; }

	// 进行更新,主要是更新显示
	void				Update();

	void				Destroy();

public:
	//计算自动捕捉点
	void				CalcSnapPoint(kiSnapMngr& snapMgr);

protected:
	//
	void				DestroyStructure(Handle(Graphic3d_Structure)& aStru);
	void				ClearStructure(Handle(Graphic3d_Structure)& aStru);

protected:
	int					m_eType;//类型
	bool				m_bVisible;//是否可见
	Handle(AIS_InteractiveContext)	m_aisContext;
	kcBasePlane			*_pBasePlane;//关联的基准面
	double				m_dWidth,m_dHeight;//宽和高
	double				m_dCellSize;//网格的大小.

protected:
	Handle(Graphic3d_Structure)		m_hStruFrm;//框架显示对象
	Handle(Graphic3d_Structure)		m_hStruIns;//内部显示对象
};

#endif