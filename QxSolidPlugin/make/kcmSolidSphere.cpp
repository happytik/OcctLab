#include "StdAfx.h"
#include <Graphic3d_AspectLine3d.hxx>
#include "kiInputPoint.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kiInputValueTool.h"
#include "kcPreviewObj.h"
#include "kcBasePlane.h"
#include "kcmUtils.h"
#include "kcmSolidSphere.h"

#define STA_SPHERE_INPUT_CENTER		1
#define STA_SPHERE_INPUT_RADIUS		2

kcmSolidSphere::kcmSolidSphere(void)
{
	m_strName = "SphereSolid";
	m_strAlias = "SSph";
	m_strDesc = "球";

	m_pInputPoint = NULL;
	m_pInputRadius = NULL;
	m_pPreviewCircle = NULL;

	m_dCenter[0] = m_dCenter[1] = m_dCenter[2] = 0.0;
	m_dRadius = 20;
}

kcmSolidSphere::~kcmSolidSphere(void)
{
}

int		kcmSolidSphere::Destroy()
{
	KC_SAFE_DELETE(m_pInputPoint);
	KC_SAFE_DELETE(m_pInputRadius);
	KC_SAFE_DELETE(m_pPreviewCircle);

	return kiCommand::Destroy();
}

//执行和结束接口
int	kcmSolidSphere::Execute()
{
	if(KSTA_ERROR == kiCommand::Execute())
		return KSTA_ERROR;

	ASSERT(m_pPreviewCircle == NULL);
	m_pPreviewCircle = NULL;
	m_nState = STA_SPHERE_INPUT_CENTER;

	return KSTA_CONTINUE;
}

int	kcmSolidSphere::End(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewCircle);

	return kiCommand::End(nCode);
}


BOOL	kcmSolidSphere::CanFinish()//命令是否可以完成
{
	return FALSE;
}

//输入工具结束时
int	kcmSolidSphere::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputPoint && m_nState == STA_SPHERE_INPUT_CENTER)
	{
		m_nState = STA_SPHERE_INPUT_RADIUS;
		//
		NavToNextTool();
	}
	else if(pTool == m_pInputRadius && m_nState == STA_SPHERE_INPUT_RADIUS)
	{
		//生成球。
		if(MakeSphere())
		{
			Done();
			Redraw();

			return KSTA_DONE;
		}
	}

	return KSTA_CONTINUE;
}

//
BOOL	kcmSolidSphere::MakeSphere()
{
	kcBasePlane *pWorkPlane = GetCurrentBasePlane();
	if(!pWorkPlane)
		return FALSE;

	//构造圆
	kvCoordSystem cs;
	pWorkPlane->GetCoordSystem(cs);

	gp_Ax2 axis(gp_Pnt(m_dCenter[0],m_dCenter[1],m_dCenter[2]),
		gp_Dir(gp_Vec(cs.Z().x(),cs.Z().y(),cs.Z().z())),
		gp_Dir(gp_Vec(cs.X().x(),cs.X().y(),cs.X().z())));

	TopoDS_Solid aSolid;
	try{
		BRepPrimAPI_MakeSphere ms(axis,m_dRadius);
		ms.Build();
		if(!ms.IsDone())
			return FALSE;

		aSolid = TopoDS::Solid(ms.Shape());
	}
	catch(Standard_Failure){
		return FALSE;
	}

	kcSolidEntity *pEntity = new kcSolidEntity;
	if(pEntity)
	{
		pEntity->SetSolid(aSolid);
		GetModel()->AddEntity(pEntity);

		Redraw();
	}
	
	KC_SAFE_DELETE(m_pPreviewCircle);

	return TRUE;
}

// 创建必要的输入工具
BOOL	kcmSolidSphere::CreateInputTools()
{
	m_pInputPoint = new kiInputPoint(this,"输入圆心:");
	m_pInputPoint->Init(&m_dCenter);
	m_pInputRadius = new kiInputDoubleTool(this,"输入半径:");
	m_pInputRadius->Init(&m_dRadius,0.0,1000000.0);

	return TRUE;
}

BOOL	kcmSolidSphere::InitInputToolQueue()
{
	AddInputTool(m_pInputPoint);
	AddInputTool(m_pInputRadius);

	return TRUE;
}

int		kcmSolidSphere::OnMouseMove(kuiMouseInput& mouseInput)
{
	if(m_nState == STA_SPHERE_INPUT_RADIUS)
	{
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);
		kPoint3 p,pc(m_dCenter);
		pWorkPlane->PixelToWorkPlane(mouseInput.m_x,mouseInput.m_y,p);
		double radius = p.distance(pc);
		if(radius < 1e-6)
			return kiCommand::OnMouseMove(mouseInput);

		if(m_pPreviewCircle == NULL)
		{
			m_pPreviewCircle = new kcPreviewCircle(GetAISContext(),GetDocContext());
			m_pPreviewCircle->SetColor(1.0,0.0,1.0);
			m_pPreviewCircle->SetType((int)Aspect_TOL_DASH);

			Handle(Geom_Circle) aCirc = ulb_MakeCircle(pc,radius,pWorkPlane);
			if(!aCirc.IsNull())
				m_pPreviewCircle->Init(aCirc);
			else
			{
				KC_SAFE_DELETE(m_pPreviewCircle);
			}
		}
		if(m_pPreviewCircle)
		{
			m_pPreviewCircle->Update(radius);
			m_pPreviewCircle->Display(TRUE);

			Redraw();
		}
	}
	return kiCommand::OnMouseMove(mouseInput);
}

int		kcmSolidSphere::OnLButtonUp(kuiMouseInput& mouseInput)
{
	if(m_nState == STA_SPHERE_INPUT_RADIUS)
	{//根据两点来判断
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);
		kPoint3 p,pc(m_dCenter);
		pWorkPlane->PixelToWorkPlane(mouseInput.m_x,mouseInput.m_y,p);
		//
		if(p.distance(pc) > 1e-6)
		{
			m_dRadius = p.distance(pc);

			if(MakeSphere())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return kiCommand::OnLButtonUp(mouseInput);
}
