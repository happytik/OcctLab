#include "StdAfx.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include "kiInputPoint.h"
#include "kPoint.h"
#include "kVector.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kiInputLength.h"
#include "kiInputAngle.h"
#include "kvCoordSystem.h"
#include "kcmUtils.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kcmArc.h"

#define CSTA_ARC_INPUT_CENTER		1
#define CSTA_ARC_INPUT_RADIUS		2
#define CSTA_ARC_INPUT_START_PNT	3
#define CSTA_ARC_INPUT_ANGLE		4

kcmArc::kcmArc(void)
{
	m_strName = "Arc";
	m_strAlias = "Arc";
	m_strDesc = "arc curve";

	m_pInputCenter = NULL;
	m_pInputRadius = NULL;
	m_pInputAngle = NULL;
	m_pInputStartPnt = NULL;

	m_dRadius = 20.0;
	m_dAngle = 0.0;
	m_pLocalCS = new kvCoordSystem;
}

kcmArc::~kcmArc(void)
{
	KC_SAFE_DELETE(m_pLocalCS);
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL kcmArc::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int kcmArc::OnInputFinished(kiInputTool *pTool)
{
	if(pTool->IsInputDone()){

		if(pTool == m_pInputCenter){
			m_nState = CSTA_ARC_INPUT_RADIUS;
			if(m_pInputRadius){
				double c[3];
				m_aCenter.get(c);
				m_pInputRadius->Set(c,TRUE);
			}
			NavToNextTool();
		}
		else if(pTool == m_pInputRadius)
		{
			m_nState = CSTA_ARC_INPUT_START_PNT;

			NavToNextTool();
		}
		else if(pTool == m_pInputStartPnt)
		{
			m_nState = CSTA_ARC_INPUT_ANGLE;
			//
			kcBasePlane *pWorkPlane = GetCurrentBasePlane();
			pWorkPlane->GetCoordSystem(*m_pLocalCS);
			kVector3 xv = m_aStartPnt - m_aCenter,zv = m_pLocalCS->Z();
			xv.normalize();
			m_pLocalCS->Set(m_aCenter,xv,zv);
			m_pInputAngle->Init(m_pLocalCS,&m_dAngle,false);

			NavToNextTool();
		}
		else if(pTool == m_pInputAngle)
		{
			if(BuildArcEntity())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	else
	{
		return KSTA_CANCEL;
	}
	return kiCommand::OnInputFinished(pTool);
}

BOOL kcmArc::BuildArcEntity()
{
	Handle(Geom_Circle) aCirc = ulb_MakeCircle(*m_pLocalCS,m_dRadius);
	if(aCirc.IsNull())
		return FALSE;

	double dAngle = DEG_TO_RAD(m_dAngle);
	TopoDS_Edge aEdge;
	try{
		BRepBuilderAPI_MakeEdge me(aCirc,0.0,dAngle);
		if(me.IsDone())
			aEdge = me.Edge();
	}catch(Standard_Failure){
	}

	if(aEdge.IsNull())
		return FALSE;

	kcModel *pModel = GetModel();
	kcEdgeEntity *pEntity = new kcEdgeEntity;
	if(pEntity)
	{
		pEntity->SetEdge(aEdge);

		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		Redraw();
	}

	return TRUE;
}

int kcmArc::OnExecute()
{
	m_nState = CSTA_ARC_INPUT_CENTER;

	m_pPreviewCircle = NULL;
	m_pPreviewLine = NULL;
	m_pPreviewArc = NULL;
	m_pPreviewText = NULL;

	kcDocContext *pDocCtx = GetDocContext();
	//创建预览对象
	m_pPreviewLine = new kcPreviewLine(GetAISContext(),pDocCtx);
	m_pPreviewLine->SetColor(1.0,0.5,0.0);

	m_pPreviewCircle = new kcPreviewCircle(GetAISContext(),pDocCtx);
	m_pPreviewCircle->SetColor(1.0,0.0,0.0);

	//m_pPreviewText = new kcPreviewText(m_pModel->GetAISContext());

	m_pPreviewArc = new kcPreviewArc(GetAISContext(),pDocCtx);
	m_pPreviewArc->SetColor(1.0,0.0,0.0);

	return KSTA_CONTINUE;
}

int		kcmArc::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewCircle);
	KC_SAFE_DELETE(m_pPreviewLine);
	KC_SAFE_DELETE(m_pPreviewArc);
	KC_SAFE_DELETE(m_pPreviewText);

	m_nState = CSTA_ARC_INPUT_CENTER;

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmArc::CreateInputTools()
{
	//
	m_pInputCenter = new kiInputPoint(this,"输入圆心");
	m_pInputCenter->Init(&m_aCenter);

	//
	m_pInputRadius = new kiInputLength(this,"输入变径");

	m_pInputStartPnt = new kiInputPoint(this,"输入起始参考点");
	m_pInputStartPnt->Init(&m_aStartPnt);
	m_pInputStartPnt->SetRealtimeUpdate(true);

	m_pInputAngle = new kiInputAngle(this,"输入角度");

	return TRUE;
}

BOOL	kcmArc::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputCenter);
	KC_SAFE_DELETE(m_pInputRadius);
	KC_SAFE_DELETE(m_pInputStartPnt);

	return TRUE;
}

BOOL	kcmArc::InitInputToolQueue()
{
	AddInputTool(m_pInputCenter);
	AddInputTool(m_pInputRadius);
	AddInputTool(m_pInputStartPnt);
	AddInputTool(m_pInputAngle);

	return TRUE;
}

int kcmArc::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == CSTA_ARC_INPUT_RADIUS){
		gp_Pnt pnt,pnt1;

		double radius = m_pInputRadius->GetLength();
		kPoint3 endpt = m_pInputRadius->GetEndPoint();
		if(radius < 0.0001)
			return KSTA_CONTINUE;
		m_dRadius = radius;

		pnt.SetCoord(endpt[0],endpt[1],endpt[2]);
		pnt1.SetCoord(m_aCenter[0],m_aCenter[1],m_aCenter[2]);
		
		if(m_pPreviewLine){
			m_pPreviewLine->Update(pnt1,pnt);
			m_pPreviewLine->Display(TRUE);
		}

		if(m_pPreviewCircle)
		{
			if(!m_pPreviewCircle->HasInited())
			{
				//构造圆
				Handle(Geom_Circle) aCirc = ulb_MakeCircle(kPoint3(m_aCenter),radius,GetCurrentBasePlane());
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
			}
		}
	}
	else if(m_nState == CSTA_ARC_INPUT_START_PNT)
	{
		if(m_pPreviewCircle)
		{
			m_pPreviewCircle->Update(m_dRadius);
			m_pPreviewCircle->Display(TRUE);
		}
		//
		gp_Pnt pnt,pnt1;

		pnt.SetCoord(m_aStartPnt[0],m_aStartPnt[1],m_aStartPnt[2]);
		pnt1.SetCoord(m_aCenter[0],m_aCenter[1],m_aCenter[2]);

		if(m_pPreviewLine)
		{
			m_pPreviewLine->Update(pnt1,pnt);
			m_pPreviewLine->Display(TRUE);
		}
	}
	else if(m_nState == CSTA_ARC_INPUT_ANGLE)
	{
		if(m_pPreviewCircle){
			m_pPreviewCircle->Display(FALSE);
		}
		
		if(m_pPreviewArc){
			if(!m_pPreviewArc->HasInited()){
				Handle(Geom_Circle) aCirc = ulb_MakeCircle(*m_pLocalCS,m_dRadius);
				if(!aCirc.IsNull()){
					m_pPreviewArc->Init(aCirc);
				}else{
					KC_SAFE_DELETE(m_pPreviewArc);
				}
			}
			if(m_pPreviewArc){
				//m_dAngle是角度
				m_pPreviewArc->Update(0.0,DEG_TO_RAD(m_dAngle));
				m_pPreviewArc->Display(TRUE);
			}
		}
	}

	return KSTA_CONTINUE;
}