//////////////////////////////////////////////////////////////////////////
// 显示各种类型的箭头,包括一段直线
#ifndef _KC_DISP_ARROW_H_
#define _KC_DISP_ARROW_H_

#include <AIS_InteractiveContext.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "kColor.h"
#include "QxLibDef.h"

class kcDocContext;

class QXLIB_API kcDispArrow
{
public:
	kcDispArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispArrow(void);

	//设置属性
	void								SetColor(const kColor& color);

	//显示和隐藏
	virtual BOOL						Display(BOOL bShow);
	virtual BOOL						IsDisplayed() const;
	virtual void						UpdateDisplay();//更新显示对象
	//清除
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay() = 0;

protected:
	Handle(AIS_InteractiveContext)		_aCtx;
	Handle(Graphic3d_Structure)		_hStructure;//显示对象
	kcDocContext						*pDocContext_;//
	bool								_bDataValid;
	bool								_bChanged;//参数发生了改变，需要重新生成显示

	kPoint3							_arrowOrg;
	kVector3							_axisDire;
	double							_arrowLen;//总长度
	kColor							_aColor;
};

// 线框显示
class QXLIB_API kcDispFrameArrow : public kcDispArrow{
public:
	kcDispFrameArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispFrameArrow();

	/*
	enum Aspect_TypeOfLine {
	Aspect_TOL_SOLID,
	Aspect_TOL_DASH,
	Aspect_TOL_DOT,
	Aspect_TOL_DOTDASH,
	Aspect_TOL_USERDEFINED
	};
	*/
	void								SetType(int type);
	void								SetWidth(double width);

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay();

protected:
	int									m_eType;//类型
	double								m_dWidth;//线宽
};

// 实体显示
class QXLIB_API kcDispSolidArrow : public kcDispArrow{
public:
	kcDispSolidArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispSolidArrow();

	//初始化
	int								Init(const kPoint3& arrowOrg,const kVector3& axisDir,
		                                double arrowLen,double coneRadius,double coneLen,double tubeRadius);
	int								Init(const kPoint3& arrowOrg,const kVector3& axisDir,double arrowLen);
	void								SetFacetNum(int num);//设定剖分面的个数

protected:
	// 具体的创建显示对象
	virtual bool						BuildGroupDisplay();

protected:
	double							_coneRadius;//锥底半径
	double							_coneLen;//锥长度
	double							_tubeRadius;//杆的半径
	int								_nbFacet;//剖分面的个数

protected:
	Handle(Prs3d_ShadingAspect)		_hShadingAspect;
};

#endif