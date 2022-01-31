#include "StdAfx.h"
#include "kiInputPoint.h"
#include "kPoint.h"
#include "kVector.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kvCoordSystem.h"
#include "kcmUtils.h"
#include "kcPreviewObj.h"
#include "kiInputLength.h"
#include "kcPreviewText.h"
#include "kcmCircle.h"

#define CSTA_CIRCLE_CENTER		1
#define CSTA_CIRCLE_RADIUS		2

kcmCircle::kcmCircle(void)
{
	m_strName = "Circle";
	m_strAlias = "circ";
	m_strDesc = "圆";

	m_nState = CSTA_CIRCLE_CENTER;
	m_pInputCenter = NULL;
	m_pInputRadius = NULL;
	m_dRadius = 0.0;

	m_bPreviewInit = FALSE;
	m_pPreviewCircle = NULL;
}

kcmCircle::~kcmCircle(void)
{
	KC_SAFE_DELETE(m_pPreviewCircle);
}

int		kcmCircle::OnExecute()
{
	m_nState = CSTA_CIRCLE_CENTER;

	//创建预览对象
	m_pPreviewCircle = new kcPreviewCircle(GetAISContext(),GetDocContext());
	m_pPreviewCircle->SetColor(1.0,0.0,0.0);
	m_bPreviewInit = FALSE;

	return KSTA_CONTINUE;
}

int		kcmCircle::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewCircle);
	m_pPreviewCircle = FALSE;

	return nCode;
}

BOOL	kcmCircle::CanFinish()//命令是否可以完成
{
	if(m_nState == CSTA_CIRCLE_RADIUS && m_dRadius > 0.0001)
		return TRUE;
	return FALSE;
}

//输入工具结束时
int	kcmCircle::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == NULL)
		return KSTA_CONTINUE;

	if(pTool == m_pInputCenter)
	{
		if(m_pInputCenter->IsInputDone())
		{
			m_nState = CSTA_CIRCLE_RADIUS;
			if(m_pInputRadius)
			{
				double a[3];
				m_aCenter.get(a);
				m_pInputRadius->Set(a,true);
			}
			//
			NavToNextTool();
		}
		else
		{
			return KSTA_CANCEL;
		}
	}
	else if(pTool == m_pInputRadius)
	{
		MakeCircleEntity();

		if(m_pPreviewCircle) m_pPreviewCircle->Display(FALSE);
		m_bPreviewInit = FALSE;

		Redraw();

		m_pInputRadius->End();

		InitInputToolQueue();
		m_nState = CSTA_CIRCLE_CENTER;
		NavToNextTool();//允许连续的画圆。
	}

	return KSTA_CONTINUE;
}

//
bool	kcmCircle::MakeCircleEntity()
{
	double radius = m_pInputRadius->GetLength();
	if(radius < 0.0001)
		return false;

	Handle(Geom_Circle) aCirc = ulb_MakeCircle(kPoint3(m_aCenter),radius,GetCurrentBasePlane());
	if(aCirc.IsNull())
		return false;

	kcEdgeEntity *pEntity = new kcEdgeEntity;
	if(pEntity)
	{
		pEntity->SetCurve(aCirc);

		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		Redraw();
	}

	return true;
}

// 创建必要的输入工具
BOOL	kcmCircle::CreateInputTools()
{
	m_pInputCenter = new kiInputPoint(this,"输入圆心");
	m_pInputCenter->Init(&m_aCenter);

	m_pInputRadius = new kiInputLength(this,"输入变径");
	m_pInputRadius->SetDefault(20.0);

	return TRUE;
}

BOOL	kcmCircle::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCenter);
	KC_SAFE_DELETE(m_pInputRadius);

	return TRUE;
}

BOOL	kcmCircle::InitInputToolQueue()
{
	ASSERT(m_pInputCenter);
	ASSERT(m_pInputRadius);

	//先清空
	ClearInputToolList();

	AddInputTool(m_pInputCenter);
	AddInputTool(m_pInputRadius);

	return TRUE;
}

int		kcmCircle::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == CSTA_CIRCLE_RADIUS)
	{
		gp_Pnt pnt,pnt1;
		double radius = 0.0;
		kPoint3 endPnt = m_pInputRadius->GetEndPoint();

		endPnt.get(m_aCircPnt);
		pnt.SetCoord(m_aCircPnt[0],m_aCircPnt[1],m_aCircPnt[2]);
		pnt1.SetCoord(m_aCenter[0],m_aCenter[1],m_aCenter[2]);
		radius = pnt1.Distance(pnt);
		if(radius < 0.0001)
			return KSTA_CONTINUE;

		if(m_pPreviewCircle)
		{
			if(!m_bPreviewInit)
			{
				//构造圆
				Handle(Geom_Circle) aCirc = ulb_MakeCircle(kPoint3(m_aCenter),radius,GetCurrentBasePlane());
				if(!aCirc.IsNull())
				{
					m_pPreviewCircle->Init(aCirc);
					m_bPreviewInit = TRUE;
				}
				else
				{
					KC_SAFE_DELETE(m_pPreviewCircle);
				}
			}
			if(m_pPreviewCircle)
			{
				m_pPreviewCircle->Update(radius);
				m_pPreviewCircle->Display(TRUE);
			}
		}

		Redraw();
	}

	return KSTA_CONTINUE;
}

int		kcmCircle::OnLButtonUp(kuiMouseInput& mouseInput)
{
	kiCommand::OnLButtonUp(mouseInput);

	return KSTA_CONTINUE;
}
