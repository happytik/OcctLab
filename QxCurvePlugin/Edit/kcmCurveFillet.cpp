#include "StdAfx.h"
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ChFi2d_FilletAPI.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BopAlgo_Tools.hxx>
#include "kcEntity.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxCurveLib.h"
#include "kiOptionSet.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputPointOnCurve.h"
#include "kcEntityFactory.h"
#include "kcmCurveFillet.h"

kcmCurveFillet::kcmCurveFillet(void)
{
	m_strName = "倒圆角";
	m_strAlias = "CrvFillet";
	m_strDesc = "曲线到圆角";

	m_pPickEntityTool = NULL;
	m_pPickLocalShapeTool = NULL;
	m_pInputRadiusTool = NULL;
	m_dRadius = 1.0;
}

kcmCurveFillet::~kcmCurveFillet(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmCurveFillet::CanFinish()//命令是否可以完成
{
	if(!m_aVertexData.empty())
		return TRUE;
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmCurveFillet::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickEntityTool)
	{
		if(m_pPickEntityTool->IsInputDone())
		{
			kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
			m_pPickLocalShapeTool->SetGlobalAISObject (se.AISObject());

			NavToNextTool();
		}
	}
	else if(pTool == m_pPickLocalShapeTool)
	{
		if(m_pPickLocalShapeTool->IsInputDone())
		{
			NavToNextTool();
		}
	}
	else if(pTool == m_pInputRadiusTool)
	{
		if(m_pInputRadiusTool->IsInputDone())
		{
			//记录一个
			TopoDS_Shape aS = m_pPickLocalShapeTool->SelectedShape(0);
			if(!aS.IsNull())
			{
				if(!m_aVertexMap.Contains(aS))
				{
					m_aVertexMap.Add(aS);
					//
					axVertexData *pData = new axVertexData;
					pData->_aVertex = TopoDS::Vertex(aS);
					pData->_dRadius = m_dRadius;

					m_aVertexData.push_back(pData);
				}
				m_pPickLocalShapeTool->ClearSel();
			}
			AddInputTool(m_pPickLocalShapeTool);
			AddInputTool(m_pInputRadiusTool);

			NavToNextTool();
		}
	}
	return KSTA_CONTINUE;
}

int	kcmCurveFillet::OnExecute()
{
	m_aVertexMap.Clear();
	return KSTA_CONTINUE;
}

int		kcmCurveFillet::OnEnd(int nCode)
{
	ClearVertexData();
	return nCode;
}

int		kcmCurveFillet::OnApply()
{
	int iret = KSTA_CONTINUE;
	m_pPickLocalShapeTool->End();
	if(DoFillet())
	{
		Done();
		iret = KSTA_DONE;
	}
	else
	{
		EndCommand(KSTA_CANCEL);
		iret = KSTA_CANCEL;
	}
	return iret;
}

static TopoDS_Vertex axFindVertex(const TopoDS_Vertex& aV,const TopoDS_Face& aFace)
{
	TopoDS_Vertex aNV,aVertex;
	gp_Pnt vp,vp2;
	TopExp_Explorer ex;
	vp = BRep_Tool::Pnt(aV);
	for(ex.Init(aFace,TopAbs_VERTEX);ex.More();ex.Next())
	{
		aVertex = TopoDS::Vertex(ex.Current());
		vp2 = BRep_Tool::Pnt(aVertex);
		if(vp.Distance(vp2) < Precision::Confusion())
		{
			aNV = aVertex;
			break;
		}
	}

	return aNV;
}

BOOL	kcmCurveFillet::DoFillet()
{
	if(m_pPickEntityTool->GetSelCount() != 1)
		return FALSE;
	kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Wire aWire = TopoDS::Wire(aS);
	BOOL bClosed = FALSE;
	bClosed = QxCurveLib::IsWireClosed(aWire);
	if(!bClosed)
	{
		//构建闭合的wire
	}
	//构造平面
	TopoDS_Face aFace;
	try{
		BRepBuilderAPI_MakeFace mf(aWire,Standard_True);
		mf.Build();
		if(mf.IsDone())
			aFace = mf.Face();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aFace.IsNull())
		return FALSE;

	TopoDS_Shape aNS;
	try{
		TopoDS_Vertex aV,aFV;
		BRepFilletAPI_MakeFillet2d mfl(aFace);
		int ix,isize = (int)m_aVertexData.size();
		for(ix = 0;ix < isize;ix ++)
		{
			aFV = axFindVertex(m_aVertexData[ix]->_aVertex,aFace);
			if(!aFV.IsNull())
				mfl.AddFillet(aFV,m_aVertexData[ix]->_dRadius);
		}
		mfl.Build();
		if(!mfl.IsDone())
			return FALSE;
		aNS = mfl.Shape();
	}catch(Standard_Failure){
		return FALSE;
	}
	//结果wire重新构建
	TopoDS_Wire aNW;
	try{
		BRepBuilderAPI_MakeWire MW;
		TopExp_Explorer exp (aNS, TopAbs_EDGE);
		for (; exp.More(); exp.Next())
			MW.Add(TopoDS::Edge(exp.Current()));
		MW.Build();
		if (!MW.IsDone())
			return FALSE;
		aNW = MW.Wire();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aNW.IsNull())
		return FALSE;

	kcModel *pModel = GetModel();
	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aNW);
	if(pEntity)
	{
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pEntity);
		pModel->EndUndo();

		return TRUE;
	}

	return FALSE;
}

void	kcmCurveFillet::ClearVertexData()
{
	int ix,isize = (int)m_aVertexData.size();
	for(ix = 0;ix < isize;ix ++)
		delete m_aVertexData[ix];
	m_aVertexData.clear();
}

// 创建必要的输入工具
BOOL	kcmCurveFillet::CreateInputTools()
{
	m_pPickEntityTool = new kiInputEntityTool(this,"拾取组合曲线");
	m_pPickEntityTool->SetOption(KCT_ENT_WIRE,1);

	m_pPickLocalShapeTool = new kiInputEntityTool(this,"拾取要倒角的节点");
	m_pPickLocalShapeTool->SetOption(KCT_ENT_POINT, 1);
	m_pPickLocalShapeTool->SetNaturalMode(false);
	
	m_pInputRadiusTool = new kiInputDoubleTool(this,"输入倒角半径");
	m_pInputRadiusTool->Init(&m_dRadius,1e-6,1000);

	return TRUE;
}

BOOL	kcmCurveFillet::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickEntityTool);
	KC_SAFE_DELETE(m_pPickLocalShapeTool);
	KC_SAFE_DELETE(m_pInputRadiusTool);

	return TRUE;
}

BOOL	kcmCurveFillet::InitInputToolQueue()
{
	AddInputTool(m_pPickEntityTool);
	AddInputTool(m_pPickLocalShapeTool);
	AddInputTool(m_pInputRadiusTool);

	return TRUE;
}

//////////////////////////////////////////////////////////////////
//
kcmTwoEdgeFillet::kcmTwoEdgeFillet(void)
{
	InitSet("TwoEdgeFillet","TEFillet","Two edge fillet");

	pOptionSet_ = NULL;
	pPickEdgeTool_ = NULL;
	dRadius_ = 2.0;
	bToWire_ = false;
	bCopy_ = false;
}

kcmTwoEdgeFillet::~kcmTwoEdgeFillet(void)
{
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int kcmTwoEdgeFillet::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == pPickEdgeTool_){
		if(pPickEdgeTool_->IsInputDone()){
			if(DoFillet()){
				Done();
				return KSTA_DONE;
			}

			// 再次拾取
			pPickEdgeTool_->End();
			NavToInputTool(pPickEdgeTool_);
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int kcmTwoEdgeFillet::OnExecute()
{
	return kiCommand::OnExecute();
}

int kcmTwoEdgeFillet::OnEnd(int nCode)
{
	return kiCommand::OnEnd(nCode);
}

int kcmTwoEdgeFillet::OnApply()
{
	return KSTA_CONTINUE;
}

// 创建必要的输入工具
BOOL kcmTwoEdgeFillet::CreateInputTools()
{
	pOptionSet_ = new kiOptionSet(this);
	kiOptionDouble *pOptDbl = new kiOptionDouble("倒角半径",'R',dRadius_);
	pOptDbl->EnablePopup(true);
	pOptionSet_->AddOption(pOptDbl);
	pOptionSet_->AddBoolOption("是否组环",'L',bToWire_);
	pOptionSet_->AddBoolOption("是否拷贝",'P',bCopy_);
	pOptionSet_->AddQuitButtonOption();

	pPickEdgeTool_ = new kiInputEntityTool(this,"拾取两个Edge",pOptionSet_);
	pPickEdgeTool_->SetOption(KCT_ENT_EDGE,2);
	pPickEdgeTool_->SetNaturalMode(true);

	return TRUE;
}

BOOL kcmTwoEdgeFillet::DestroyInputTools()
{
	KC_SAFE_DELETE(pPickEdgeTool_);
	KC_SAFE_DELETE(pOptionSet_);

	return TRUE;
}

BOOL kcmTwoEdgeFillet::InitInputToolQueue()
{
	AddInputTool(pPickEdgeTool_);
	return TRUE;
}

//
void kcmTwoEdgeFillet::ClearVertexData()
{
}

static bool uxGetCommonPoint(const TopoDS_Edge &aE1,
						  const TopoDS_Edge &aE2,gp_Pnt& aPnt)
{
	BRepAdaptor_Curve aC1(aE1),aC2(aE2);
	gp_Pnt p11,p12,p21,p22;
	aC1.D0(aC1.FirstParameter(),p11);
	aC1.D0(aC1.LastParameter(),p12);
	aC2.D0(aC2.FirstParameter(),p21);
	aC2.D0(aC2.LastParameter(),p22);

	double tol = Precision::Confusion();
	if(p11.Distance(p21) < tol || p11.Distance(p22) < tol){
		aPnt = p11;
	}else if(p12.Distance(p21) < tol || p12.Distance(p22) < tol){
		aPnt = p12;
	}else{
		return false;
	}
	return true;
}

//
BOOL kcmTwoEdgeFillet::DoFillet()
{
	kiSelSet *pSelSet = pPickEdgeTool_->GetSelSet();
	kiSelEntity se1 = pSelSet->GetFirstSelected();
	kiSelEntity se2 = pSelSet->GetSelEntity(1);
	TopoDS_Shape aS1 = se1.SelectShape(),aS2 = se2.SelectShape();
	if(aS1.IsNull() || aS2.IsNull())
		return FALSE;

	TopoDS_Compound aComp,aComp2;
	BRep_Builder aBB;
	aBB.MakeCompound(aComp);
	aBB.Add(aComp,aS1);
	aBB.Add(aComp,aS2);

	BRepBuilderAPI_FindPlane findPln(aComp);
	if(!findPln.Found())
		return FALSE;

	TopoDS_Edge aE1 = TopoDS::Edge(aS1);
	TopoDS_Edge aE2 = TopoDS::Edge(aS2);
	Handle(Geom_Plane) aPln = findPln.Plane();
	gp_Pnt aCommPnt;

	if(!uxGetCommonPoint(aE1,aE2,aCommPnt))
		return FALSE;

	ChFi2d_FilletAPI filletAPI;
	filletAPI.Init(aE1,aE2,aPln->Pln());
	if(!filletAPI.Perform(dRadius_)){
		AfxMessageBox("倒角失败.");
		return FALSE;
	}

	TopoDS_Edge aER1,aER2,aFltEdge,aWires;
	aFltEdge = filletAPI.Result(aCommPnt,aER1,aER2);
	
	if(bToWire_){
		aBB.MakeCompound(aComp2);
		aBB.Add(aComp2,aER1);
		aBB.Add(aComp2,aFltEdge);
		aBB.Add(aComp2,aER2);
		int rc = BOPAlgo_Tools::EdgesToWires(aComp2,aWires);
		if(rc != 0){
			return FALSE;
		}
	}

	TopExp_Explorer aExp;
	kcEntity *pEnt = NULL;
	kcEntityFactory entFac;
	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	{
		if(!bCopy_){
			pModel->DelEntity(se1.Entity());
			pModel->DelEntity(se2.Entity());
		}
		//
		if(bToWire_){
			for(aExp.Init(aWires,TopAbs_WIRE);aExp.More();aExp.Next()){
				pEnt = entFac.Create(aExp.Current());
				if(pEnt){
					pModel->AddEntity(pEnt);
				}
			}
		}else{
			pEnt = entFac.Create(aER1);
			if(pEnt){
				pModel->AddEntity(pEnt);
			}
			pEnt = entFac.Create(aFltEdge);
			if(pEnt){
				pModel->AddEntity(pEnt);
			}
			pEnt = entFac.Create(aER2);
			if(pEnt){
				pModel->AddEntity(pEnt);
			}
		}
	}
	pModel->EndUndo();
	
	Redraw();

	return TRUE;
}