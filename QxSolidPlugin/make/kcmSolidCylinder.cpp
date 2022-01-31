#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kiInputPoint.h"
#include "kiInputLength.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcPreviewObj.h"
#include "QxCurveLib.h"
#include "kcEntityFactory.h"
#include "kcmSolidCylinder.h"

#define KC_STA_INPUT_CENTER		1
#define KC_STA_INPUT_RADIUS		2
#define KC_STA_INPUT_HEIGHT		3

kcmSolidCylinder::kcmSolidCylinder(void)
{
	m_strName = "圆柱体";
	m_strAlias = "Cyl";
	m_strDesc = "圆柱体";

	m_nState = KC_STA_INPUT_CENTER;
	m_pInputCenter = NULL;
	m_pInputRadius = NULL;
	m_dRadius = 5.0;
	m_pInputHeight = NULL;
	m_dAngle = 360.0;

	m_pPreviewCircle = NULL;
	m_pPreviewCircle2 = NULL;
	m_pPreviewLine = NULL;
	m_pPreviewLine2 = NULL;
}

kcmSolidCylinder::~kcmSolidCylinder(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidCylinder::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmSolidCylinder::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputCenter)
	{
		if(m_pInputCenter->IsInputDone())
		{
			double p[3];
			m_aCenter.get(p);
			m_pInputRadius->Set(p,true);
			//
			m_nState = KC_STA_INPUT_RADIUS;
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputRadius)
	{
		if(m_pInputRadius->IsInputDone())
		{
			m_dRadius = m_pInputRadius->GetLength();
			if(m_dRadius > 0.001)
			{
				kVector3 zdir = GetWorkPlaneZDir();
				m_pInputHeight->Set(m_aCenter,zdir,true);

				// 此时，这个可能没有值
				m_aXPoint = m_pInputRadius->GetEndPoint();
				kVector3 vec = m_aXPoint - m_aCenter;
				if(vec.iszero()){
					vec.set(1,0,0);
					kVector3 cs = zdir ^ vec;
					if(cs.iszero()){
						vec.set(0,1,0);
					}
				}else{
					vec.normalize();
					m_aXPoint = m_aCenter + m_dRadius * vec;
				}

				m_nState = KC_STA_INPUT_HEIGHT;
				NavToNextTool();
			}
		}
	}
	else if(pTool == m_pInputHeight)
	{
		if(m_pInputHeight->IsInputDone())
		{
			if(DoBuild())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				Redraw();

				return KSTA_FAILED;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int		kcmSolidCylinder::OnExecute()
{
	m_nState = KC_STA_INPUT_CENTER;
	//
	kcDocContext *pDocCtx = GetDocContext();

	m_pPreviewCircle = new kcPreviewCircle(GetAISContext(),pDocCtx);
	m_pPreviewCircle->SetColor(0.0,0.0,1.0);

	m_pPreviewCircle2 = new kcPreviewCircle(GetAISContext(),pDocCtx);
	m_pPreviewCircle2->SetColor(0.0,0.0,1.0);

	m_pPreviewLine = new kcPreviewLine(GetAISContext(),pDocCtx);
	m_pPreviewLine->SetColor(0.0,0.0,1.0);

	m_pPreviewLine2 = new kcPreviewLine(GetAISContext(),pDocCtx);
	m_pPreviewLine2->SetColor(0.0,0.0,1.0);

	return KSTA_CONTINUE;
}

int		kcmSolidCylinder::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewCircle);
	KC_SAFE_DELETE(m_pPreviewCircle2);
	KC_SAFE_DELETE(m_pPreviewLine);
	KC_SAFE_DELETE(m_pPreviewLine2);

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSolidCylinder::CreateInputTools()
{
	m_pInputCenter = new kiInputPoint(this,"输入地面圆心");
	m_pInputCenter->Init(&m_aCenter);

	m_pInputRadius = new kiInputLength(this,"输入半径");
	m_pInputRadius->SetDefault(5.0);

	m_optionSet.AddDoubleOption("角度",'A',m_dAngle);
	m_pInputHeight = new kiInputLength(this,"输入高度",&m_optionSet);
	m_pInputHeight->SetDefault(10.0);

	return TRUE;
}

BOOL	kcmSolidCylinder::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCenter);
	KC_SAFE_DELETE(m_pInputRadius);
	KC_SAFE_DELETE(m_pInputHeight);

	return TRUE;
}

BOOL	kcmSolidCylinder::InitInputToolQueue()
{
	AddInputTool(m_pInputCenter);
	AddInputTool(m_pInputRadius);
	AddInputTool(m_pInputHeight);

	return TRUE;
}

int		kcmSolidCylinder::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == KC_STA_INPUT_RADIUS)
	{
		m_dRadius = m_pInputRadius->GetLength();
		if(!m_pPreviewCircle->HasInited())
		{
			kVector3 zdir = GetWorkPlaneZDir();
			m_aXPoint = m_pInputRadius->GetEndPoint();
			Handle(Geom_Circle) aCirc = QxCurveLib::MakeCircle(m_aCenter,m_aXPoint,zdir,m_dRadius);
			if(!aCirc.IsNull())
				m_pPreviewCircle->Init(aCirc);
		}
		else
		{
			m_pPreviewCircle->Update(m_dRadius);
			m_pPreviewCircle->Display(TRUE);
		}
	}
	else if(m_nState == KC_STA_INPUT_HEIGHT)
	{
		if(mouseInput.m_nFlags & MK_MBUTTON)
		{
			int i = 0;
		}
		double dHeight = m_pInputHeight->GetLength();
		kVector3 zdir = GetWorkPlaneZDir();
		kPoint3 ac = m_aCenter + zdir * dHeight;
		kPoint3 xp = m_aXPoint + zdir * dHeight;

		Handle(Geom_Circle) aCirc = QxCurveLib::MakeCircle(ac,xp,zdir,m_dRadius);
		if(!aCirc.IsNull())
		{
			m_pPreviewCircle2->Init(aCirc);
			m_pPreviewCircle2->Update(m_dRadius);
			m_pPreviewCircle2->Display(TRUE);
		}

		//
		if(m_pPreviewLine)
		{
			m_pPreviewLine->Update(gp_Pnt(m_aXPoint[0],m_aXPoint[1],m_aXPoint[2]),gp_Pnt(xp[0],xp[1],xp[2]));
			m_pPreviewLine->Display(TRUE);
		}
	}

	return kiCommand::OnMouseMove(mouseInput);
}

int		kcmSolidCylinder::OnLButtonUp(kuiMouseInput& mouseInput)
{
	return kiCommand::OnLButtonUp(mouseInput);
}


BOOL	kcmSolidCylinder::DoBuild()
{
	kVector3 zdir = GetWorkPlaneZDir();
	kVector3 x = m_aXPoint - m_aCenter,y;
	if(!x.normalize())
		return FALSE;

	y = zdir ^ x;
	if(!y.normalize()){
		x.set(1,0,0);
		y = zdir ^ x;
		if(!y.normalize()){
			return FALSE;
		}
	}

	double dHeight = m_pInputHeight->GetLength();
	if(fabs(dHeight) < KDBL_MIN)
		return FALSE;

	gp_Ax2 ax(gp_Pnt(m_aCenter[0],m_aCenter[1],m_aCenter[2]),
		gp_Dir(zdir[0],zdir[1],zdir[2]),
		gp_Dir(x[0],x[1],x[2]));

	if(dHeight < 0)
	{
		kPoint3 ac = m_aCenter + zdir * dHeight;
		ax.SetLocation(gp_Pnt(ac[0],ac[1],ac[2]));
		dHeight = -dHeight;
	}

	TopoDS_Shape aNS;
	try
	{
		if(fabs(m_dAngle - 360.0) < KDBL_MIN)
		{
			BRepPrimAPI_MakeCylinder mc(ax,m_dRadius,dHeight);
			mc.Build();
			if(mc.IsDone())
				aNS = mc.Shape();
		}
		else
		{
			BRepPrimAPI_MakeCylinder mc(ax,m_dRadius,dHeight,m_dAngle * K_PI / 180.0);
			mc.Build();
			if(mc.IsDone())
				aNS = mc.Shape();
		}
	}
	catch (Standard_Failure){
		return FALSE;
	}
	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory fac;
	kcEntity *pEntity = fac.Create(aNS);
	if(pEntity)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		return TRUE;
	}
	

	return FALSE;
}

