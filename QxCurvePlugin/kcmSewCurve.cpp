#include "StdAfx.h"
#include <BOPAlgo_Tools.hxx>
#include "kiInputEntityTool.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcModel.h"
#include "QxCurveLib.h"
#include "kcmSewCurve.h"

kcmSewCurve::kcmSewCurve(void)
{
	m_strName = "Sew Curve";
	m_strAlias = "SewCur";
	m_strDesc = "组合曲线";

	m_pInputEntity = NULL;
	m_bCopy = false;
}

kcmSewCurve::~kcmSewCurve(void)
{
}

//命令是否可以完成
BOOL	kcmSewCurve::CanFinish()
{
	if(!m_pInputEntity ||
	    m_pInputEntity->GetSelCount() <= 0)
		return FALSE;

	return TRUE;
}

//输入工具结束时
int		kcmSewCurve::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity){
		//if(SewCurve())
		if(DoSewingCurve()){
			m_pInputEntity->ClearSel();

			Done();
			Redraw();

			return KSTA_DONE;
		}
	}

	return kiCommand::OnInputFinished(pTool);
}

//具体的初始化和释放代码,每个子命令可以做各自的初始化和销毁操作。
int		kcmSewCurve::OnInitialize()
{
	return KSTA_CONTINUE;
}

int		kcmSewCurve::OnDestroy()
{
	return KSTA_CONTINUE;
}

// 执行函数
int		kcmSewCurve::OnExecute()
{
	if(m_pInputEntity){
		m_pInputEntity->AddInitSelected(false);
	}
	return KSTA_CONTINUE;
}

int		kcmSewCurve::OnEnd(int nCode)
{
	if(m_pInputEntity)
		m_pInputEntity->ClearSel();

	return KSTA_CONTINUE;
}

int		kcmSewCurve::OnApply()
{
	if(SewCurve())
	{
		m_pInputEntity->ClearSel();

		Done();
		Redraw();

		return KSTA_DONE;
	}

	return KSTA_ERROR;
}

// 创建必要的输入工具
BOOL	kcmSewCurve::CreateInputTools()
{
	kiOptionBool *pOptBool = new kiOptionBool("是否拷贝对象?",'C',m_bCopy);
	m_optionSet.AddOption(pOptBool);

	m_pInputEntity = new kiInputEntityTool(this,"选择曲线:",&m_optionSet);
	m_pInputEntity->SetOption(KCT_ENT_EDGE | KCT_ENT_WIRE);
	m_pInputEntity->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmSewCurve::InitInputToolQueue()
{
	if(!m_pInputEntity) return FALSE;

	AddInputTool(m_pInputEntity);
	return TRUE;
}

bool	kcmSewCurve::SewCurve()
{
	if(!m_pInputEntity || m_pInputEntity->GetSelCount() <= 1)
		return false;
	
	kcgSewCurve sewCur;
	if(!sewCur.Init())
		return false;

	kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
	kiSelEntity selEnt;
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected())
	{
		selEnt = pSelSet->CurSelected();
		//
		if(IS_EDGE_ENTITY(selEnt._pEntity))
		{
			kcEdgeEntity *pCurEnt = (kcEdgeEntity *)selEnt._pEntity;
			//sewCur.AddCurve(pCurEnt->GetCurve());
			sewCur.AddEdge(pCurEnt->GetEdge());
		}
		else
		{
			ASSERT(IS_WIRE_ENTITY(selEnt._pEntity));
			kcWireEntity *pWireEnt = (kcWireEntity *)selEnt._pEntity;
			sewCur.AddWire(pWireEnt->GetWire());
		}

		pSelSet->NextSelected();
	}

	if(!sewCur.Sew(false))
	{
		sewCur.Clear();
		return false;
	}

	TopoDS_Wire aWire = sewCur.Wire();
	kcWireEntity *pWireEntity = new kcWireEntity;
	pWireEntity->SetWire(aWire);

	kcModel *pModel = GetModel();
	//删除旧的
	for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected())
	{
		selEnt = pSelSet->CurSelected();
		pModel->DelEntity(selEnt._pEntity);
	}

	pModel->AddEntity(pWireEntity);

	Redraw();

	return true;
}

// 将多条edge和wire缝合成wire
//
bool  kcmSewCurve::DoSewingCurve()
{
	if(!m_pInputEntity || m_pInputEntity->GetSelCount() <= 1)
		return false;

	BRep_Builder BB;
	TopoDS_Compound aEComp;

	BB.MakeCompound(aEComp);

	int cnt = 0;
	kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
	pSelSet->InitSelected();
	while(pSelSet->MoreSelected()){
		kiSelEntity se = pSelSet->CurSelected();
		TopoDS_Shape aS = se.SelectShape();//Edge or wire

		if(aS.ShapeType() == TopAbs_EDGE){
			BB.Add(aEComp,aS);
			cnt ++;
		}else if(aS.ShapeType() == TopAbs_WIRE){
			TopoDS_Iterator ite;
			for(ite.Initialize(aS);ite.More();ite.Next()){
				BB.Add(aEComp,ite.Value());
				cnt ++;
			}
		}

		pSelSet->NextSelected();
	}

	if(cnt == 0){
		return false;
	}
	
	try{
		BRepBuilderAPI_FindPlane findPlane(aEComp);
		if(!findPlane.Found()){
			return false;
		}
	}catch(Standard_Failure){
		return false;
	}

	TopoDS_Shape aWires;
	int rc = BOPAlgo_Tools::EdgesToWires(aEComp,aWires);
	if(rc != 0){
		return false;
	}

	kcEntity *pEnt = NULL;
	kcEntityFactory efac;
	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		TopExp_Explorer exp;
		exp.Init(aWires,TopAbs_WIRE);
		for(;exp.More();exp.Next()){
			pEnt = efac.Create(exp.Current());
			pModel->AddEntity(pEnt);
		}
	}
	pModel->EndUndo();

	Redraw();

	return true;
}