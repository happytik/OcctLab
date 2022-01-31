#include "StdAfx.h"
#include "kcPreviewObj.h"
#include "kiInputPicker.h"
#include "kiCommand.h"
#include "QxBaseUtils.h"
#include "kiInputPointOnCurve.h"

// 用于命令中的构造函数。
kiInputPointOnCurve::kiInputPointOnCurve(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	m_pPreviewPoint = NULL;
	m_bPicked = false;
}
	// 用于InputTool中的构造函数。
kiInputPointOnCurve::kiInputPointOnCurve(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	m_pPreviewPoint = NULL;
	m_bPicked = false;
}

kiInputPointOnCurve::~kiInputPointOnCurve(void)
{
}

bool	kiInputPointOnCurve::Init(const TopoDS_Edge& aEdge,int itol)
{
	if(aEdge.IsNull()) return false;
	m_aShape = aEdge;
	m_nPickTol = itol;
	return true;
}

bool	kiInputPointOnCurve::Init(const TopoDS_Wire& aWire,int itol)
{
	if(aWire.IsNull()) return false;
	m_aShape = aWire;
	m_nPickTol = itol;
	return true;
}

//
int	kiInputPointOnCurve::OnBegin()
{
	ASSERT(!m_aShape.IsNull());
	if(m_aShape.IsNull())
		return KSTA_ERROR;

	m_pPreviewPoint = new kcPreviewPoint(GetAISContext(),GetDocContext());
	m_pPreviewPoint->SetColor(1.0,1,1);
	m_pPreviewPoint->SetType(Aspect_TOM_X);

	m_bPicked = false;//没有拾取

	return KSTA_CONTINUE;
}

int	kiInputPointOnCurve::OnEnd()
{
	KC_SAFE_DELETE(m_pPreviewPoint);
	return KSTA_DONE;
}

//鼠标消息
int		kiInputPointOnCurve::OnLButtonUp(kuiMouseInput& mouseInput)
{
	if(m_aShape.IsNull())
		return KSTA_CONTINUE;

	kiCurvePicker crvPicker(GetDocContext());
	if(m_aShape.ShapeType() == TopAbs_EDGE)
		crvPicker.Init(TopoDS::Edge(m_aShape));
	else
		crvPicker.Init(TopoDS::Wire(m_aShape));
	if(crvPicker.Pick(mouseInput.m_x,mouseInput.m_y,
		m_nPickTol,m_dPickParam,m_aPickPoint,m_aPickEdge))
	{
		m_bPicked = true;
		SetInputState(KINPUT_DONE);
		DoNotify();

		return KSTA_DONE;
	}
	return KSTA_CONTINUE;
}

int		kiInputPointOnCurve::OnMouseMove(kuiMouseInput& mouseInput)
{
	if(!IsInputDone() && !m_aShape.IsNull())
	{
		kiCurvePicker crvPicker(GetDocContext());
		if(m_aShape.ShapeType() == TopAbs_EDGE)
			crvPicker.Init(TopoDS::Edge(m_aShape));
		else
			crvPicker.Init(TopoDS::Wire(m_aShape));
		if(crvPicker.Pick(mouseInput.m_x,mouseInput.m_y,
			m_nPickTol,m_dPickParam,m_aPickPoint,m_aPickEdge))
		{
			m_pPreviewPoint->Update(KCG_GP_PNT(m_aPickPoint));
			m_pPreviewPoint->Display(TRUE);
		}
	}
	return KSTA_CONTINUE;
}

int		kiInputPointOnCurve::OnRButtonUp(kuiMouseInput& mouseInput)
{
	if(m_bPicked)
	{
		SetInputState(KINPUT_DONE);
		if(m_pCommand)
		{
			m_pCommand->OnInputFinished(this);
		}
		return KSTA_DONE;
	}
	else
	{
		SetInputState(KINPUT_CANCEL);
		return KSTA_CANCEL;
	}
}
