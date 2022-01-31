#include "StdAfx.h"
#include <BRepOffsetAPI_ThruSections.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxCurveLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSolidLoft.h"

kcmSolidLoft::kcmSolidLoft(void)
{
	m_strName = "实体放样";
	m_strAlias = "SolidLoft";
	m_strDesc = "实体放样";

	m_pInputStart = NULL;
	m_pInputThrough = NULL;
	m_pInputEnd = NULL;

	m_bBuildSolid = false;
}

kcmSolidLoft::~kcmSolidLoft(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidLoft::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSolidLoft::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputStart)
	{
		if(m_pInputStart->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputThrough)
	{
		NavToNextTool();
	}
	else if(pTool == m_pInputEnd)
	{
		if(m_pInputEnd->IsInputDone())
		{
			if(DoLoft())
			{
				Done();
				Redraw();

				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

static BOOL auxAddSection(BRepOffsetAPI_ThruSections& tsec,kiSelEntity se)
{
	if(IS_POINT_ENTITY(se._pEntity))
	{
		TopoDS_Vertex aV = TopoDS::Vertex(se._pEntity->GetShape());
		tsec.AddVertex(aV);
	}
	else if(IS_EDGE_ENTITY(se._pEntity))
	{
		TopoDS_Edge aEdge = TopoDS::Edge(se._pEntity->GetShape());
		TopoDS_Wire aWire = QxBRepLib::EdgeToWire(aEdge);
		if(aWire.IsNull())
			return FALSE;
		tsec.AddWire(aWire);
	}
	else
	{
		TopoDS_Wire aWire = TopoDS::Wire(se._pEntity->GetShape());
		tsec.AddWire(aWire);
	}
	return TRUE;
}

static TopoDS_Shape auxCheckSection(kiSelEntity se,BOOL& bClosed,BOOL& bVertex)
{
	TopoDS_Shape aShape;
	bClosed = FALSE;bVertex = FALSE;
	if(IS_POINT_ENTITY(se._pEntity))
	{
		aShape = se._pEntity->GetShape();
		bVertex = TRUE;
	}
	else if(IS_EDGE_ENTITY(se._pEntity))
	{
		TopoDS_Edge aEdge = TopoDS::Edge(se._pEntity->GetShape());
		TopoDS_Wire aWire = QxBRepLib::EdgeToWire(aEdge);
		if(aWire.IsNull())
			return aShape;
		aShape = aWire;
		//
		bClosed = QxCurveLib::IsEdgeClosed(aEdge);
	}
	else
	{
		TopoDS_Wire aWire = TopoDS::Wire(se._pEntity->GetShape());
		bClosed = QxCurveLib::IsWireClosed(aWire);
		aShape = aWire;
	}
	return aShape;
}

BOOL	kcmSolidLoft::DoLoft()
{
	if(m_pInputStart->GetSelCount() != 1 ||
		m_pInputEnd->GetSelCount() != 1)
		return FALSE;

	//依次检查并获取相应的对象
	int nbVertex = 0,nbShape = 0,nbClosed = 0;
	BOOL bClosed = FALSE,bVertex = FALSE;
	TopTools_ListOfShape shapeList;
	TopoDS_Shape aS;
	kiSelEntity se;
	// 起始
	se = m_pInputStart->GetFirstSelect();
	aS = auxCheckSection(se,bClosed,bVertex);
	if(!aS.IsNull())
	{
		shapeList.Append(aS);
		if(bClosed) nbClosed ++;
		if(bVertex) nbVertex ++;
		nbShape ++;
	}
	//中间经过曲线
	if(m_pInputThrough->GetSelCount() > 0)
	{
		kiSelSet *pSelSet = m_pInputThrough->GetSelSet();
		pSelSet->InitSelected();
		while(pSelSet->MoreSelected())
		{
			se = pSelSet->CurSelected();
			aS = auxCheckSection(se,bClosed,bVertex);
			if(!aS.IsNull())
			{
				shapeList.Append(aS);
				if(bClosed) nbClosed ++;
				if(bVertex) nbVertex ++;
				nbShape ++;
			}	

			pSelSet->NextSelected();
		}
	}

	//结束曲线
	se = m_pInputEnd->GetFirstSelect();
	aS = auxCheckSection(se,bClosed,bVertex);
	if(!aS.IsNull())
	{
		shapeList.Append(aS);
		if(bClosed) nbClosed ++;
		if(bVertex) nbVertex ++;
		nbShape ++;
	}

	if(nbVertex == 2 && nbShape == nbVertex)//两个点，无法构建
		return FALSE;

	BOOL bSolid = FALSE;
	if(nbVertex + nbClosed == nbShape)//都是点或闭合曲线，可以创建solid
		bSolid = TRUE;

	TopoDS_Shape aNS;
	try{
		BRepOffsetAPI_ThruSections tsec;
		if(bSolid && m_bBuildSolid)
			tsec.Init(Standard_True);

		TopTools_ListIteratorOfListOfShape lls;
		for(lls.Initialize(shapeList);lls.More();lls.Next())
		{
			aS = lls.Value();
			if(aS.ShapeType() == TopAbs_VERTEX)
				tsec.AddVertex(TopoDS::Vertex(aS));
			else
				tsec.AddWire(TopoDS::Wire(aS));
		}

		tsec.CheckCompatibility();
		tsec.Build();
		if(tsec.IsDone())
			aNS = tsec.Shape();
	}catch(Standard_Failure){}

	if(aNS.IsNull())
		return FALSE;

	kcEntityFactory entfac;
	kcEntity *pEnt = entfac.Create(aNS);
	if(pEnt)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEnt);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

int		kcmSolidLoft::OnExecute()
{
	m_pInputStart->ClearSel();
	m_pInputThrough->ClearSel();
	m_pInputEnd->ClearSel();

	return KSTA_CONTINUE;
}

int		kcmSolidLoft::OnEnd(int nCode)
{
	m_pInputStart->ClearSel();
	m_pInputThrough->ClearSel();
	m_pInputEnd->ClearSel();

	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSolidLoft::CreateInputTools()
{
	m_optionSet.AddBoolOption("实体",'S',m_bBuildSolid);

	m_pInputStart = new kiInputEntityTool(this,"拾取起始点或曲线",&m_optionSet);
	m_pInputStart->SetOption(KCT_ENT_POINT | KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	m_pInputThrough = new kiInputEntityTool(this,"拾取通过曲线");
	m_pInputThrough->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE);

	m_pInputEnd = new kiInputEntityTool(this,"拾取结束点或曲线");
	m_pInputEnd->SetOption(KCT_ENT_POINT | KCT_ENT_EDGE | KCT_ENT_WIRE,1);

	return TRUE;
}

BOOL	kcmSolidLoft::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputStart);
	KC_SAFE_DELETE(m_pInputThrough);
	KC_SAFE_DELETE(m_pInputEnd);

	return TRUE;
}

BOOL	kcmSolidLoft::InitInputToolQueue()
{
	AddInputTool(m_pInputStart);
	AddInputTool(m_pInputThrough);
	AddInputTool(m_pInputEnd);

	return TRUE;
}

