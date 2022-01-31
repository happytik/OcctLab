#ifndef _KC_DISPOBJ_H_
#define _KC_DISPOBJ_H_

#include "kPoint.h"
#include "kVector.h"
#include <Graphic3d_StructureManager.hxx>
#include "QxLibDef.h"

//显示一个对象的简单框架，用于交互中的过程显示
class QXLIB_API kcDispFrmShape
{
public:
	kcDispFrmShape(const Handle_AIS_InteractiveContext& aCtx);
	virtual ~kcDispFrmShape();

	//设置属性
	void								SetIsoNum(int UIsoNum,int VIsoNum);
	void								SetColor(int color);

	//添加对象
	bool								AddShape(const TopoDS_Shape& aShape);
	void								Destroy();

	bool								Show();
	bool								Hide();

	//移动
	bool								Move(double dx,double dy,double dz);
	// 旋转
	// p:基点,dir:单位向量，ang：旋转角度，弧度。
	bool								Rotate(const kPoint3& p,const kVector3& dir,double ang);

	// 缩放
	bool								Scale(const kPoint3& p,double scale);


protected:
	Handle(Graphic3d_StructureManager)	m_aStruMgr;
	Handle(Prs3d_Presentation)			m_aPrs;

	int								m_nUIsoNum,m_nVIsoNum;
	int								m_color;
};


#endif