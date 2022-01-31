// 定义一个基准面
//
#ifndef _KC_BASE_PLANE_H_
#define _KC_BASE_PLANE_H_

#include <V3d_View.hxx>
#include "QxLibDef.h"
#include "kvCoordSystem.h"


class kvGrid;
class kcModel;
class CArchive;

#define  KCT_BASE_PLANE			0x20000  //避免和Entity的冲突

class QXLIB_API kcBasePlane
{
public:
	kcBasePlane(const Handle(V3d_View)& aView);
	~kcBasePlane(void);

	//根据一个坐标系创建
	int						Create(kvCoordSystem& cs,kcModel *pModel,const char *pszName = NULL);
	void						Destroy();

public:
	void						SetActive(bool bActive);//是否设置为活动的基准面

	// 获取内部的Grid
	kvGrid*					GetGrid() const { return _pGrid; }
	Handle(V3d_View)			GetV3dView() const { return _hView; }
	kvCoordSystem*			GetCoordSystem() { return &_cs; }
	int						GetCoordSystem(kvCoordSystem& cs);
	const kVector3&			GetZDirection() const;

	void						SetDefaultView();//设定默认实体，目前是上视图。

	int						GetHandle() const { return _nHandle;}
	void						SetName(const char *szName);
	std::string				GetName() const;
	bool						HasName() const;

public:
	// 屏幕窗口坐标转到WCS空间点和线.
	// x,y:窗口坐标,vp:空间点 vdir:空间视向.
	//
	bool						PixelToModel(int x,int y,kPoint3& vp,kVector3& vdir);

	// WCS空间点到屏幕坐标的转换
	// wp:空间点,wcs坐标系下.x,y:返回的屏幕坐标
	bool						ModelToPixel(const kPoint3& wp,int& x,int& y);

	// 屏幕窗口坐标到工作平面上点.通过视向射线和平面求交得到
	// x,y:窗口坐标,wp:工作平面上的点,wcs坐标系下.
	bool						PixelToWorkPlane(int x,int y,kPoint3& wp);

protected:
	bool						InitGrid();

	// 计算直线和平面的交点
	bool						LinPlnIntersection(const kPoint3& linpnt,const kVector3& lindir,
									const kPoint3& plnpnt,const kVector3& plnnorm,kPoint3& intpt);

public:
	// 读取
	BOOL						Read(CArchive& ar,kcModel *pModel);
	// 写入
	BOOL						Write(CArchive& ar);

protected:
	int						_nHandle;//句柄
	std::string				_szName;//名称
	kvCoordSystem				_cs;//坐标系
	kcModel					*_pModel;//所在模型
	Handle(V3d_View)			_hView;//关联的view
	kvGrid					*_pGrid;//工作平面网格。
	bool						_bActive;//是否当前基准面
};

#endif