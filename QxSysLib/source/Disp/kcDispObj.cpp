#include "StdAfx.h"
//#include <Graphic3d_Array1OfVertex.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_Color.hxx>
#include <Prs3d_IsoAspect.hxx>
#include <StdPrs_WFShape.hxx>
#include "QxBaseUtils.h"
#include "kcDispobj.h"

//////////////////////////////////////////////////////////////////////////
//显示一个对象的简单框架，用于交互中的过程显示
kcDispFrmShape::kcDispFrmShape(const Handle_AIS_InteractiveContext& aCtx)
{
	m_aStruMgr = NULL;
	m_aPrs = NULL;
	m_nUIsoNum = m_nVIsoNum = 5;
	m_color = RGB(0,128,255);

	if(!aCtx.IsNull() && !aCtx->CurrentViewer().IsNull())
		m_aStruMgr = aCtx->CurrentViewer()->StructureManager();
}
kcDispFrmShape::~kcDispFrmShape()
{
	Destroy();
}

//设置属性
void	kcDispFrmShape::SetIsoNum(int UIsoNum,int VIsoNum)
{
	m_nUIsoNum = UIsoNum;
	m_nVIsoNum = VIsoNum;
}

void	kcDispFrmShape::SetColor(int color)
{
	m_color = color;
}

//添加对象
bool	kcDispFrmShape::AddShape(const TopoDS_Shape& aShape)
{
	if(m_aStruMgr.IsNull() || aShape.IsNull())
		return false;

	if(m_aPrs.IsNull())
	{
		m_aPrs = new Prs3d_Presentation(m_aStruMgr);
		if(m_aPrs.IsNull())
			return false;
	}

	//准备添加
	Handle(Prs3d_Drawer)  aDrawer = new Prs3d_Drawer;

	double r,g,b;
	r = GetRValue(m_color) / 256.0;g = GetGValue(m_color) / 256.0;b = GetBValue(m_color) / 256.0;
	Quantity_Color col(r,g,b,Quantity_TOC_RGB);

	aDrawer->UIsoAspect()->SetNumber(m_nUIsoNum);
	aDrawer->UIsoAspect()->SetColor(col);
	aDrawer->VIsoAspect()->SetNumber(m_nVIsoNum);
	aDrawer->VIsoAspect()->SetColor(col);

	aDrawer->UnFreeBoundaryAspect()->SetColor(col);
	aDrawer->FreeBoundaryAspect()->SetColor(col);
	aDrawer->WireAspect()->SetColor(col);

	StdPrs_WFShape::Add(m_aPrs,aShape,aDrawer);

	return true;
}

void	kcDispFrmShape::Destroy()
{
	if(!m_aPrs.IsNull())
	{
		if(m_aPrs->IsDisplayed())
			m_aPrs->Erase();
		
		m_aPrs.Nullify();
	}
}

bool	kcDispFrmShape::Show()
{
	if(!m_aPrs->IsDisplayed())
		m_aPrs->Display();
	return true;
}

bool	kcDispFrmShape::Hide()
{
	if(m_aPrs->IsDisplayed())
		m_aPrs->Erase();

	return true;
}

//移动
bool	kcDispFrmShape::Move(double dx,double dy,double dz)
{
	//TColStd_Array2OfReal mat(0,3,0,3);
	//int ix,jx;
	//for(ix = 0;ix < 4;ix ++)
	//	for(jx = 0;jx < 4;jx ++)
	//		mat(ix,jx) = 0.0;
	//mat(0,0) = mat(1,1) = mat(2,2) = mat(3,3) = 1.0;
	//mat(0,3) = dx;mat(1,3) = dy;mat(2,3) = dz;

	//m_aPrs->GraphicTransform(mat);

	//m_aViewMgr->Update();

	return true;
}

// 旋转
// p:基点,dir:单位向量，ang：旋转角度，弧度。
bool	kcDispFrmShape::Rotate(const kPoint3& p,const kVector3& dir,double ang)
{
	//TColStd_Array2OfReal mat(0,3,0,3);
	//gp_Ax1 ax(KCG_GP_PNT(p),KCG_GP_DIR(dir));
	//gp_Trsf tsf;
	//tsf.SetRotation(ax,ang);

	//gp_XYZ xyz = tsf.TranslationPart();
	//gp_Mat vmat = tsf.VectorialPart();
	//int ix,jx;
	//for(ix = 0;ix < 3;ix ++)
	//	for(jx = 0;jx < 3;jx ++)
	//		mat(ix,jx) = vmat.Value(ix + 1,jx + 1);
	//mat(3,0) = mat(3,1) = mat(3,2) = 0.0;
	//mat(0,3) = xyz.X();mat(1,3) = xyz.Y();
	//mat(2,3) = xyz.Z();mat(3,3) = 1.0;

	//m_aPrs->GraphicTransform(mat);

	//m_aViewMgr->Update();

	return true;
}

// 缩放
bool	kcDispFrmShape::Scale(const kPoint3& p,double scale)
{
	//TColStd_Array2OfReal mat(0,3,0,3);
	//gp_Trsf tsf;
	//tsf.SetScale(KCG_GP_PNT(p),scale);

	//gp_XYZ xyz = tsf.TranslationPart();
	//gp_Mat vmat = tsf.VectorialPart();
	//int ix,jx;
	//for(ix = 0;ix < 3;ix ++)
	//	for(jx = 0;jx < 3;jx ++)
	//		mat(ix,jx) = vmat.Value(ix + 1,jx + 1);
	//mat(3,0) = mat(3,1) = mat(3,2) = 0.0;
	//mat(0,3) = xyz.X();mat(1,3) = xyz.Y();
	//mat(2,3) = xyz.Z();mat(3,3) = 1.0;
	//
	//m_aPrs->GraphicTransform(mat);

	//m_aViewMgr->Update();
	
	return true;
}
