#include "StdAfx.h"
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_StructureManager.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <Prs3d_Arrow.hxx>
#include <Graphic3d_Group.hxx>
#include "kcDocContext.h"
#include "kcDispArrow.h"

kcDispArrow::kcDispArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx)
{
	_bDataValid = false;
	_bChanged = false;
	pDocContext_ = pDocCtx;
	
	_arrowLen = 0.0;
	_aColor.set(1.0,1.0,0.0);
	
	if(!aCtx.IsNull() && !aCtx->CurrentViewer().IsNull()){
		Handle(Graphic3d_StructureManager) aStruMgr = 
			aCtx->CurrentViewer()->StructureManager();
	
		if(!aStruMgr.IsNull()){
			_hStructure = new Graphic3d_Structure(aStruMgr);
		}

		_aCtx = aCtx;//记录
	}
}

kcDispArrow::~kcDispArrow(void)
{
	Destroy();
}

//设置属性
void  kcDispArrow::SetColor(const kColor& color)
{
	if(_aColor != color){
		_aColor = color;
		_bChanged = true;
	}
}


//显示和隐藏
BOOL  kcDispArrow::Display(BOOL bShow)
{
	if(_hStructure.IsNull() || !_bDataValid)
		return FALSE;
	
	Standard_Boolean bDisplayed = _hStructure->IsDisplayed();
	//依然不显示
	if(!bShow && !bDisplayed)
		return TRUE;

	//隐藏
	if(!bShow && bDisplayed){
		_hStructure->Erase();
	}else{//显示对象
		//更新显示
		UpdateDisplay();

		if(!_hStructure->IsDisplayed()){
			_hStructure->Display();
		}
	}
	
	if(pDocContext_){
		pDocContext_->RefreshCurView();
	}

	return TRUE;
}

BOOL  kcDispArrow::IsDisplayed() const
{
	if(_hStructure.IsNull())
		return FALSE;
	return _hStructure->IsDisplayed();
}

//清除
void  kcDispArrow::Clear()
{
	if(!_hStructure.IsNull()){
		_hStructure->Erase();
		_hStructure->Clear(true);
	}
	
	_bChanged = false;
}

void  kcDispArrow::Destroy()
{
	if(!_hStructure.IsNull()){
		_hStructure->Erase();
		_hStructure->Remove();
		_hStructure.Nullify();
	}
}

void  kcDispArrow::UpdateDisplay()
{
	bool bNeedUpdate = false;

	if(!_bDataValid)	return;

	if(_hStructure.IsNull()){//be destroy
		Handle(Graphic3d_StructureManager) aStruMgr = 
			_aCtx->CurrentViewer()->StructureManager();
	
		if(!aStruMgr.IsNull()){
			_hStructure = new Graphic3d_Structure(aStruMgr);
			bNeedUpdate = true;
		}else{
			return;
		}
	}

	if(_bChanged) bNeedUpdate = true;
	if(!bNeedUpdate)
		return;
	
	//更新
	Standard_Boolean bDisplayed = _hStructure->IsDisplayed();
	if(bDisplayed){
		_hStructure->Erase();//先隐藏
	}
	
	//清除旧的数据，包括所有的group
	_hStructure->Clear(true);
	
	// 重新创建显示对象
	if(!BuildGroupDisplay())
		return;

	//重新显示
	if(bDisplayed){
		_hStructure->Display();
	}

	//更新了
	_bChanged = false;
}

//////////////////////////////////////////////////////////////////////////
// 线框显示
kcDispFrameArrow::kcDispFrameArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx)
	:kcDispArrow(aCtx,pDocCtx)
{
	m_eType = Aspect_TOL_SOLID;
	m_dWidth = 1.0;
}

kcDispFrameArrow::~kcDispFrameArrow()
{

}

void	kcDispFrameArrow::SetType(int type)
{
	m_eType = type;
	if(m_eType < Aspect_TOL_SOLID) m_eType = Aspect_TOL_SOLID;
	if(m_eType >= Aspect_TOL_USERDEFINED) m_eType = Aspect_TOL_SOLID;
}

void	kcDispFrameArrow::SetWidth(double width)
{
	m_dWidth = width;
	if(m_dWidth < 0.0)
		m_dWidth = 1.0;
}

// 具体的创建显示对象
bool	kcDispFrameArrow::BuildGroupDisplay()
{
	//Handle(Graphic3d_Group) hGroup = Prs3d_Root::CurrentGroup(m_aPrs);
	//if(hGroup.IsNull())
	//	return false;

	////set attrib
	//Handle(Graphic3d_AspectLine3d) hAspLin = new Graphic3d_AspectLine3d;
	//hAspLin->SetColor(Quantity_Color(m_aColor.r(),m_aColor.g(),m_aColor.b(),Quantity_TOC_RGB));
	//hAspLin->SetType((Aspect_TypeOfLine)m_eType);
	//hAspLin->SetWidth(m_dWidth);
	////m_aPrs->SetPrimitivesAspect(hAspLin);

	////计算实际长度
	//double dLinLen = m_dLineLength,dArrowLen = m_dArrowLength;
	//if(!m_bSpaceLen)
	//{
	//	double dscale = glb_GetViewToWindowScale();
	//	dLinLen = m_nLineLength * dscale,dArrowLen = m_nArrowLength * dscale;
	//}
	//kPoint3 lep = m_aPoint + m_aDir * dLinLen;
/*
	//create line
	Graphic3d_Array1OfVertex V2(1,2);
	V2(1).SetCoord(m_aPoint[0],m_aPoint[1],m_aPoint[2]);
	V2(2).SetCoord(lep[0],lep[1],lep[2]);
	//hGroup->Polyline(V2);

	//create arrow
	kPoint3 ac = lep,apex = ac + m_aDir * dArrowLen;
	int nbPoints = 15;
	Graphic3d_Array1OfVertex VN(1,nbPoints + 1);
	Graphic3d_Array1OfVertex VNC(1,nbPoints + 1);
	Graphic3d_Array1OfVertex VC2(1,2);
	double tg = tan(m_dAngle * K_PI / 180.0),radius = dArrowLen * tg;
	kVector3 kx,ky;
	//// Construction of i,j mark for the circle:
	RandomAxis(kx,ky);

	double x,y,z;
	double cosinus,sinus;
	int ix;
	V2(1).SetCoord(apex[0],apex[1],apex[2]);
	VC2(1).SetCoord(ac[0],ac[1],ac[2]);
	for(ix = 1;ix <= nbPoints;ix ++)
	{
		cosinus = cos(2 * K_PI / nbPoints * (ix - 1));
		sinus = sin(2 * K_PI / nbPoints * (ix - 1));

		x = ac[0] + (cosinus * kx[0] + sinus * ky[0]) * radius; 
		y = ac[1] + (cosinus * kx[1] + sinus * ky[1]) * radius;
		z = ac[2] + (cosinus * kx[2] + sinus * ky[2]) * radius;
	
		V2(2).SetCoord(x,y,z);
		VC2(2).SetCoord(x,y,z);
//		hGroup->Polyline(V2);
//		hGroup->Polyline(VC2);
		VN(ix).SetCoord(x,y,z);
		if(ix == 1) VN(nbPoints + 1).SetCoord(x,y,z);
	}
//	hGroup->Polyline(VN);
*/
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 实体显示
kcDispSolidArrow::kcDispSolidArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx)
	:kcDispArrow(aCtx,pDocCtx)
{
	_nbFacet = 20;//默认剖分个数
}

kcDispSolidArrow::~kcDispSolidArrow()
{

}

//初始化
int  kcDispSolidArrow::Init(const kPoint3& arrowOrg,const kVector3& axisDir,
                         double arrowLen,double coneRadius,double coneLen,double tubeRadius)
{
	_arrowOrg = arrowOrg;
	_axisDire = axisDir;
	_arrowLen = arrowLen;
	_coneRadius = coneRadius;
	_coneLen = coneLen;
	_tubeRadius = tubeRadius;

	_bChanged = true;
	_bDataValid = true;

	return 1;
}

int  kcDispSolidArrow::Init(const kPoint3& arrowOrg,const kVector3& axisDir,double arrowLen)
{
	_arrowOrg = arrowOrg;
	_axisDire = axisDir;
	_arrowLen = arrowLen;

	_coneRadius = arrowLen * 0.05;
	_coneLen = arrowLen * 0.2;
	_tubeRadius = arrowLen * 0.01;

	_bChanged = true;
	_bDataValid = true;

	return 1;
}

void  kcDispSolidArrow::SetFacetNum(int num)//设定剖分面的个数
{
	ASSERT(num > 0);
	_nbFacet = num;
}

// 具体的创建显示对象
bool  kcDispSolidArrow::BuildGroupDisplay()
{
	if(_hShadingAspect.IsNull()){
		_hShadingAspect = new Prs3d_ShadingAspect;
	}

	_hShadingAspect->SetColor(Quantity_Color(_aColor.r(),
		_aColor.g(),_aColor.b(),Quantity_TOC_RGB));

	//生成显示对象
	Handle(Graphic3d_Group) aGroup = _hStructure->NewGroup();

	gp_Ax1 ax1;
	ax1.SetLocation(gp_Pnt(_arrowOrg.x(),_arrowOrg.y(),_arrowOrg.z()));
	ax1.SetDirection(gp_Dir(_axisDire.x(),_axisDire.y(),_axisDire.z()));

	Handle(Graphic3d_ArrayOfTriangles) aArrayTria = Prs3d_Arrow::DrawShaded(ax1,_tubeRadius,
		_arrowLen,_coneRadius,_coneLen,_nbFacet);
	if(!aArrayTria.IsNull()){
		aGroup->SetGroupPrimitivesAspect(_hShadingAspect->Aspect());
		aGroup->AddPrimitiveArray(aArrayTria);
	}

	return true;
}
