#include "StdAfx.h"
#include <BRepBuilderAPI_MakeWire.hxx>
#include "kcModel.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kiInputPoint.h"
#include "kcgTextLib.h"
#include "kcSysUtils.h"
#include "kiCommandMgr.h"
#include "kcmRect.h"

//状态
#define CSTA_RECT_POINT1		0
#define CSTA_RECT_POINT2		1
#define CSTA_RECT_POINT3		2

kcmRect::kcmRect(void)
{
	m_strName = "Rect";
	m_strAlias = "rct";
	m_strDesc = "绘制矩形";

	_pInputPoint1 = NULL;
	_pInputPoint2 = NULL;
	_pInputPoint3 = NULL;

	m_nState = CSTA_RECT_POINT1;

	_pPreviewRect = NULL;
	_pPreviewText = NULL;
	_pPreviewText2 = NULL;
}

kcmRect::~kcmRect(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmRect::CanFinish()//命令是否可以完成
{
	if(!_pInputPoint1 || !_pInputPoint1 || !_pInputPoint1)
		return FALSE;

	if(m_nState == CSTA_RECT_POINT2)
		return TRUE;

	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmRect::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == _pInputPoint1)
	{
		if(pTool->IsInputDone())
		{
			m_nState = CSTA_RECT_POINT2;
			NavToNextTool();
		}
	}
	else if(pTool == _pInputPoint2)
	{
		MakeRectEntity();

		Done();
		Redraw();

		return KSTA_DONE;
	}

	return kiCommand::OnInputFinished(pTool);
}

int		kcmRect::OnExecute()
{
	m_nState = CSTA_RECT_POINT1;
	return KSTA_CONTINUE;
}

int		kcmRect::OnEnd(int nCode)
{
	KC_SAFE_DELETE(_pPreviewRect);
	KC_SAFE_DELETE(_pPreviewText);
	KC_SAFE_DELETE(_pPreviewText2);

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmRect::CreateInputTools()
{
	_pInputPoint1 = new kiInputPoint(this,"输入第一个点:");
	_pInputPoint2 = new kiInputPoint(this,"输入第二个点:");

	_pInputPoint1->Init(&_aPoint1);
	_pInputPoint2->Init(&_aPoint2);
	_pInputPoint2->SetRealtimeUpdate(true);

	return TRUE;
}

BOOL	kcmRect::DestroyInputTools()
{
	KC_SAFE_DELETE(_pInputPoint1);
	KC_SAFE_DELETE(_pInputPoint2);

	return TRUE;
}

BOOL	kcmRect::InitInputToolQueue()
{
	AddInputTool(_pInputPoint1);
	AddInputTool(_pInputPoint2);

	return TRUE;
}

int		kcmRect::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == CSTA_RECT_POINT2)
	{
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);

		if(_pPreviewRect == NULL)
		{
			_pPreviewRect = new kcPreviewRect(GetAISContext(),GetDocContext());
			_pPreviewRect->SetColor(0.0,1.0,1.0);
			_pPreviewRect->Init(gp_Pnt(_aPoint1[0],_aPoint1[1],_aPoint1[2]),pWorkPlane);
		}
		if(_pPreviewRect)
		{
			_pPreviewRect->Update(gp_Pnt(_aPoint2[0],_aPoint2[1],_aPoint2[2]));
			_pPreviewRect->Display(TRUE);

			Redraw();
		}
	}

	return KSTA_CONTINUE;
}

//创建曲线对象
BOOL	kcmRect::MakeRectEntity()
{
	kvCoordSystem cs;
	kcBasePlane *pWorkPlane = GetCurrentBasePlane();
	ASSERT(pWorkPlane);
	pWorkPlane->GetCoordSystem(cs);

	double c[4][3];
	kPoint3 p1(_aPoint1),p2(_aPoint2);
	if(p1.distance(p2) < KC_MIN_DIST)
		return FALSE;

	kcg_CalcRectCorner(cs,p1,p2,c);
	//
	TopoDS_Wire aWire;
	try{
		BRepBuilderAPI_MakeWire mw;
		int ix,inx;
		for(ix = 0;ix < 4;ix ++)
		{
			inx = (ix + 1) % 4;
			BRepBuilderAPI_MakeEdge me(gp_Pnt(c[ix][0],c[ix][1],c[ix][2]),
										gp_Pnt(c[inx][0],c[inx][1],c[inx][2]));
			if(me.IsDone())
				mw.Add(me.Edge());
		}
		if(mw.IsDone())
			aWire = mw.Wire();

	}catch(Standard_Failure){
		return FALSE;
	}

	if(aWire.IsNull())
		return FALSE;

	kcWireEntity *pEntity = new kcWireEntity;
	if(pEntity)
	{
		pEntity->SetWire(aWire);

		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();
	}

	return TRUE;
}
