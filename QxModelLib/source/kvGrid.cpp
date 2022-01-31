#include "StdAfx.h"
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_StructureManager.hxx>
#include "kcBasePlane.h"
#include "kvCoordSystem.h"
#include "kiSnapMngr.h"
#include "kvGrid.h"

//
kvGrid::kvGrid(const Handle(AIS_InteractiveContext)& aCtx,kcBasePlane *pBasePlane)
{
	ASSERT(!aCtx.IsNull());
	ASSERT(pBasePlane);
	m_eType = KVT_GRID_LINE;
	m_bVisible = true;
	m_aisContext = aCtx;
	_pBasePlane = pBasePlane;

	m_dWidth = 100.0;
	m_dHeight = 100.0;
	m_dCellSize = 1.0;

	//创建显示对象
	Handle(Graphic3d_StructureManager) aStruMgr = 
		aCtx->CurrentViewer()->StructureManager();
	m_hStruFrm = new Graphic3d_Structure(aStruMgr);
	m_hStruIns = new Graphic3d_Structure(aStruMgr);

	m_hStruFrm->SetInfiniteState(true);
	m_hStruIns->SetInfiniteState(true);
}

kvGrid::~kvGrid(void)
{
	Destroy();
}

// 销毁
void	kvGrid::Destroy()
{
	DestroyStructure(m_hStruFrm);
	DestroyStructure(m_hStruIns);
}

void	kvGrid::DestroyStructure(Handle(Graphic3d_Structure)& aStru)
{
	if(!aStru.IsNull())
	{
		if(aStru->IsDisplayed())
			aStru->Erase();

		aStru->Clear(Standard_True);
		//aStru->Destroy();
	}
}

void	kvGrid::ClearStructure(Handle(Graphic3d_Structure)& aStru)
{
	if(!aStru.IsNull())
	{
		if(aStru->IsDisplayed())
			aStru->Erase();

		aStru->Clear(Standard_True);
	}
}

void	kvGrid::SetType(int type)
{
	ASSERT(type == KVT_GRID_LINE || type == KVT_GRID_DOT);
	m_eType = type;
}

// 是否可见，需要及时显示。
//
void	kvGrid::SetVisible(bool bVisible)
{
	if(m_bVisible != bVisible){
		m_bVisible = bVisible;
		//
		Update();
	}
}

void	kvGrid::SetWidth(double width)
{
	m_dWidth = width;
}

void	kvGrid::SetHeight(double height)
{
	m_dHeight = height;
}

void	kvGrid::SetCellSize(double size)
{
	m_dCellSize = size;
}

// 进行更新,主要是更新显示
void	kvGrid::Update()
{
	//先因此当前显示的.
	if(m_hStruFrm.IsNull()) return;

	//清除就数据
	ClearStructure(m_hStruFrm);
	ClearStructure(m_hStruIns);

	if(!m_bVisible)//不显示
		return;

	// 创建新的数据.
	// 绘制粗网格
	int nbXCell = (int)(m_dWidth / m_dCellSize);
	int nbYCell = (int)(m_dHeight / m_dCellSize);
	int nbFrmVertex = (nbXCell / 10 + 1 + nbYCell / 10 + 1) * 2;
	int nbInsVertex = (nbXCell + 1) * 2 + (nbYCell + 1) * 2 - nbFrmVertex;
	int nbFrmEdge = nbFrmVertex;
	int nbInsEdge = nbInsVertex;
	double x0 = -m_dWidth * 0.5,x1 = m_dWidth * 0.5;
	double y0 = -m_dHeight * 0.5,y1 = m_dHeight * 0.5;

	Handle(Graphic3d_Group) aGroupFrm = m_hStruFrm->NewGroup();//new Graphic3d_Group(m_hStruFrm);
	Handle(Graphic3d_Group) aGroupIns = m_hStruIns->NewGroup();//new Graphic3d_Group(m_hStruIns);

	Handle(Graphic3d_ArrayOfSegments) aSegFrm,aSegIns;
	Handle(Graphic3d_ArrayOfPoints)	aPnts;

	if(m_eType == KVT_GRID_LINE)
	{
		aSegFrm = new Graphic3d_ArrayOfSegments(nbFrmVertex,nbFrmEdge);
		aSegIns = new Graphic3d_ArrayOfSegments(nbInsVertex,nbInsEdge);
	}
	else
	{
		aSegFrm = new Graphic3d_ArrayOfSegments(nbFrmVertex,nbFrmEdge);
		// 重新计算点的个数
		nbInsVertex = (nbXCell - nbXCell / 10) * (nbYCell - nbYCell / 10);
		aPnts = new Graphic3d_ArrayOfPoints(nbInsVertex);
	}

	//添加点
	int ix,jx;
	double x,y;
	kvCoordSystem cs;
	_pBasePlane->GetCoordSystem(cs);

	//x向的点
	for(ix = 0;ix <= nbXCell;ix ++)
	{
		x = x0 + ix * m_dCellSize;
		kPoint3 bp(x,y0,0.0),ep(x,y1,0.0);
		cs.UCSToWCS(bp);
		cs.UCSToWCS(ep);

		if(ix % 10 == 0)
		{
			aSegFrm->AddVertex(bp[0],bp[1],bp[2]);
			aSegFrm->AddVertex(ep[0],ep[1],ep[2]);
		}
		else
		{
			if(m_eType == KVT_GRID_LINE)
			{
				aSegIns->AddVertex(bp[0],bp[1],bp[2]);
				aSegIns->AddVertex(ep[0],ep[1],ep[2]);
			}
			else
			{
				for(jx = 0;jx <= nbYCell;jx ++)
				{
					y = y0 + jx * m_dCellSize;
					if(jx % 10 == 0)
						continue;

					kPoint3 tp(x,y,0.0);
					cs.UCSToWCS(tp);

					aPnts->AddVertex(tp[0],tp[1],tp[2]);
				}
			}
		}
	}
	//y向的点
	for(ix = 0;ix <= nbYCell;ix ++)
	{
		y = y0 + ix * m_dCellSize;
		kPoint3 bp(x0,y,0.0),ep(x1,y,0.0);
		//
		cs.UCSToWCS(bp);
		cs.UCSToWCS(ep);

		if(ix % 10 == 0)
		{
			aSegFrm->AddVertex(bp[0],bp[1],bp[2]);
			aSegFrm->AddVertex(ep[0],ep[1],ep[2]);
		}
		else
		{
			if(m_eType == KVT_GRID_LINE)
			{
				aSegIns->AddVertex(bp[0],bp[1],bp[2]);
				aSegIns->AddVertex(ep[0],ep[1],ep[2]);
			}
		}
	}
	//添加edge
	// x向
	int ie = 1,iie = 1;
	for(ix = 0;ix <= nbXCell;ix ++)
	{
		if(ix % 10 == 0)
		{
			aSegFrm->AddEdge(ie ++);
			aSegFrm->AddEdge(ie ++);
		}
		else
		{
			if(m_eType == KVT_GRID_LINE)
			{
				aSegIns->AddEdge(iie ++);
				aSegIns->AddEdge(iie ++);
			}
		}
	}
	// y向
	for(ix = 0;ix <= nbYCell;ix ++)
	{
		if(ix % 10 == 0)
		{
			aSegFrm->AddEdge(ie ++);
			aSegFrm->AddEdge(ie ++);
		}
		else
		{
			if(m_eType == KVT_GRID_LINE)
			{
				aSegIns->AddEdge(iie ++);
				aSegIns->AddEdge(iie ++);
			}
		}
	}

	//aGroupFrm->BeginPrimitives();
	aGroupFrm->AddPrimitiveArray(aSegFrm);
	//aGroupFrm->EndPrimitives();

	//aGroupIns->BeginPrimitives();
	if(m_eType == KVT_GRID_LINE)
		aGroupIns->AddPrimitiveArray(aSegIns);
	else
		aGroupIns->AddPrimitiveArray(aPnts);
	//aGroupIns->EndPrimitives();

	//设置对应属性
	Handle(Graphic3d_AspectLine3d) aspLineFrm = new Graphic3d_AspectLine3d;
	aspLineFrm->SetColor(Quantity_Color(0.5,0.5,0.5,Quantity_TOC_RGB));
	if(m_eType == KVT_GRID_LINE)
		aspLineFrm->SetType(Aspect_TOL_SOLID);
	else
		aspLineFrm->SetType(Aspect_TOL_DOT);
	aspLineFrm->SetWidth(0.5);
	aGroupFrm->SetPrimitivesAspect(aspLineFrm);
	//m_hStruFrm->SetPrimitivesAspect(aspLineFrm);

	Handle(Graphic3d_AspectLine3d) aspLineIns = new Graphic3d_AspectLine3d;
	aspLineIns->SetColor(Quantity_Color(0.3,0.3,0.3,Quantity_TOC_RGB));
	if(m_eType == KVT_GRID_LINE)
		aspLineIns->SetType(Aspect_TOL_DOT);
	aspLineIns->SetWidth(0.2);
	aGroupIns->SetPrimitivesAspect(aspLineIns);
	//m_hStruIns->SetPrimitivesAspect(aspLineIns);


	//显示
	m_hStruIns->Display();
	m_hStruFrm->Display();
}

//计算自动捕捉点
void	kvGrid::CalcSnapPoint(kiSnapMngr& snapMgr)
{
	snapMgr.ClearGridSnapPoint();
	//
	int ix,jx;
	kvCoordSystem cs;
	int nbXCell = (int)(m_dWidth / m_dCellSize);
	int nbYCell = (int)(m_dHeight / m_dCellSize);
	double x0 = -m_dWidth * 0.5,x1 = m_dWidth * 0.5;
	double y0 = -m_dHeight * 0.5,y1 = m_dHeight * 0.5;
	kPoint3 lp;
	int xsc,ysc;

	_pBasePlane->GetCoordSystem(cs);

	for(ix = 0;ix <= nbXCell;ix ++)
	{
		for(jx = 0;jx <= nbYCell;jx ++)
		{
			lp.set(x0 + ix * m_dCellSize,y0 + jx * m_dCellSize,0.0);
			//
			cs.UCSToWCS(lp);

			_pBasePlane->ModelToPixel(lp,xsc,ysc);

			kiSnapPoint sp;
			sp._point = lp;
			sp._scx = xsc;
			sp._scy = ysc;
			snapMgr.AddGridSnapPoint(sp);
		}
	}
}