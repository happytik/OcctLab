#include "StdAfx.h"
#include <BRepCheck_Face.hxx>
#include "kcEntity.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "QxCurveLib.h"
#include "kcPreviewObj.h"
#include "kiInputEntityTool.h"
#include "kiInputValueTool.h"
#include "kiInputPointOnCurve.h"
#include "kcEntityFactory.h"
#include "QxFacePointPicker.h"
#include "kcmFaceCurve.h"

//曲面的边界
kiFaceBoundCurve::kiFaceBoundCurve()
{
	m_pPickEntityTool = NULL;
	m_pPickLocalShapeTool = NULL;

	m_strPrompt = "选择边界曲线";
	m_nSelModel = 0;
}

kiFaceBoundCurve::~kiFaceBoundCurve()
{

}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kiFaceBoundCurve::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kiFaceBoundCurve::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickEntityTool)
	{
		if(m_pPickEntityTool->IsInputDone())
		{
			kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
			//
			m_pPickLocalShapeTool->SetGlobalAISObject(se.AISObject());
			
			NavToNextTool();
		}
	}
	else if(pTool == m_pPickLocalShapeTool)
	{
		if(m_pPickLocalShapeTool->IsInputDone())
		{
			m_pPickLocalShapeTool->End();

			if(DoBuildBoundCurve())
			{
				Done();
				return KSTA_DONE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
		else
		{
			EndCommand(KSTA_CANCEL);
			return KSTA_CANCEL;
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int		kiFaceBoundCurve::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kiFaceBoundCurve::OnEnd(int nCode)
{
	return nCode;
}

// 创建必要的输入工具
BOOL	kiFaceBoundCurve::CreateInputTools()
{
	m_pPickEntityTool = new kiInputEntityTool(this,"选择对象");
	m_pPickEntityTool->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL | KCT_ENT_SOLID | KCT_ENT_COMPOUND,1);

	m_pPickLocalShapeTool = new kiInputEntityTool(this,m_strPrompt.c_str());
	m_pPickLocalShapeTool->SetNaturalMode(false);
	m_pPickLocalShapeTool->SetOption(m_nSelModel, -1);

	return TRUE;
}

BOOL	kiFaceBoundCurve::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pPickEntityTool);
	KC_SAFE_DELETE(m_pPickLocalShapeTool);

	return TRUE;
}

BOOL	kiFaceBoundCurve::InitInputToolQueue()
{
	AddInputTool(m_pPickEntityTool);
	AddInputTool(m_pPickLocalShapeTool);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//边界曲线
kcmFaceBoundEdge::kcmFaceBoundEdge()
{
	m_strName = "边界曲线";
	m_strAlias = "BndEdge";
	m_strDesc = "曲面边界曲线";

	m_strPrompt = "选择边界曲线";
	m_nSelModel = KCT_ENT_EDGE;
}

//
BOOL	kcmFaceBoundEdge::DoBuildBoundCurve()
{
	if(m_pPickEntityTool->GetSelCount() != 1)
		return FALSE;
	kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Edge aEdge,aNE;
	TopoDS_Face aFace;
	TopTools_IndexedDataMapOfShapeListOfShape shapeMap;
	BOOL bNeedFace = FALSE;

	if(IS_FACE_ENTITY(se._pEntity))
		aFace = TopoDS::Face(aS);
	else
	{
		TopExp::MapShapesAndAncestors(aS,TopAbs_EDGE,TopAbs_FACE,shapeMap);
		bNeedFace = TRUE;
	}

	std::vector<kcEntity *> aEnt;
	kcEntity *pEntity = NULL;
	kcEntityFactory efac;
	int ix,nbShape = m_pPickLocalShapeTool->GetSelCount();
	for(ix = 0;ix < nbShape;ix ++)
	{
		aEdge = TopoDS::Edge(m_pPickLocalShapeTool->SelectedShape(ix));
		if(bNeedFace)
		{
			const TopTools_ListOfShape& shapeList = shapeMap.FindFromKey(aEdge);
			if(shapeList.IsEmpty())
				continue;
			aFace = TopoDS::Face(shapeList.First());
		}
		if(aFace.IsNull())
			continue;
		aNE = QxCurveLib::BuildBoundEdge(aEdge,aFace);
		if(!aNE.IsNull())
		{
			pEntity = efac.Create(aNE);
			if(pEntity)
				aEnt.push_back(pEntity);
		}
		//创建曲线方向标识（圆)
		aNE = QxCurveLib::BuildEdgeDirFlag(aEdge,aFace);
		if(!aNE.IsNull())
		{
			pEntity = efac.Create(aNE);
			if(pEntity)
				aEnt.push_back(pEntity);
		}
	}

	if(aEnt.empty())
		return FALSE;

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	pModel->AddEntity(aEnt);
	pModel->EndUndo();

	Redraw();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//边界曲线方向
kcmFaceBoundEdgeDire::kcmFaceBoundEdgeDire()
{
	m_strName = "边界环方向";
	m_strAlias = "BndWireDire";
	m_strDesc = "曲面边界环的方向";

	m_bTopolDire = false;
	m_nSelModel = KCT_ENT_WIRE;
}

kcmFaceBoundEdgeDire::~kcmFaceBoundEdgeDire()
{
	ClearTempDispObj();
}

int		kcmFaceBoundEdgeDire::OnEnd(int nCode)
{
	ClearTempDispObj();
	return nCode;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmFaceBoundEdgeDire::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pPickEntityTool)
	{
		if(m_pPickEntityTool->IsInputDone())
		{
			kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
			m_pPickLocalShapeTool->SetGlobalAISObject(se.AISObject());

			NavToNextTool();
		}
	}
	else if(pTool == m_pPickLocalShapeTool)
	{
		if(m_pPickLocalShapeTool->IsInputDone())
		{
			m_pPickLocalShapeTool->End();

			if(DoShowWireDire())
			{
				AddInputTool(m_pPickLocalShapeTool);
				NavToNextTool();

				return KSTA_CONTINUE;
			}
			else
			{
				EndCommand(KSTA_FAILED);
				return KSTA_FAILED;
			}
		}
		else
		{
			EndCommand(KSTA_CANCEL);
			return KSTA_CANCEL;
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

// 创建必要的输入工具
BOOL	kcmFaceBoundEdgeDire::CreateInputTools()
{
	m_optionSet.AddBoolOption("拓扑方向",'T',m_bTopolDire);
	m_pPickEntityTool = new kiInputEntityTool(this,"选择对象",&m_optionSet);
	m_pPickEntityTool->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL | KCT_ENT_SOLID | KCT_ENT_COMPOUND,1);

	m_pPickLocalShapeTool = new kiInputEntityTool(this,m_strPrompt.c_str());
	m_pPickLocalShapeTool->SetOption(m_nSelModel, -1);
	m_pPickLocalShapeTool->SetNaturalMode(false);

	return TRUE;
}

//
BOOL	kcmFaceBoundEdgeDire::DoBuildBoundCurve()
{
	return FALSE;
}

bool	kcmFaceBoundEdgeDire::DoShowWireDire()
{
	if(m_pPickEntityTool->GetSelCount() != 1)
		return FALSE;
	kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Edge aEdge,aNE;
	TopoDS_Face aFace;
	TopTools_IndexedDataMapOfShapeListOfShape shapeMap;
	BOOL bNeedFace = FALSE;

	if(IS_FACE_ENTITY(se._pEntity))
		aFace = TopoDS::Face(aS);
	else
	{
		TopExp::MapShapesAndAncestors(aS,TopAbs_WIRE,TopAbs_FACE,shapeMap);
		bNeedFace = TRUE;
	}

	TopoDS_Wire aWire;
	int ix,nbShape = m_pPickLocalShapeTool->GetSelCount();
	for(ix = 0;ix < nbShape;ix ++)
	{
		aWire = TopoDS::Wire(m_pPickLocalShapeTool->SelectedShape(ix));
		if(bNeedFace)
		{
			const TopTools_ListOfShape& shapeList = shapeMap.FindFromKey(aWire);
			if(shapeList.IsEmpty())
				continue;
			aFace = TopoDS::Face(shapeList.First());
		}
		if(aFace.IsNull())
			continue;

		TopExp_Explorer exp;
		for(exp.Init(aWire,TopAbs_EDGE);exp.More();exp.Next()){
			aEdge = TopoDS::Edge(exp.Current());

			DoGenEdgeDireDisp(aEdge,aFace);
		}
	}

	Redraw();

	return true;
}

bool	kcmFaceBoundEdgeDire::DoGenEdgeDireDisp(const TopoDS_Edge& aEdge,const TopoDS_Face& aFace)
{
	double dF,dL,pF,pL,mt;
	double dLen,dLen2;
	gp_Pnt pnt,tp,p2,p3;
	gp_Pnt2d p2d;
	gp_Vec tang,N,D1U,D1V,offdir;

	Handle(Geom_Curve) aCurve = BRep_Tool::Curve(aEdge,dF,dL);
	Handle(Geom2d_Curve) aCrv2d = BRep_Tool::CurveOnSurface(aEdge,aFace,pF,pL);
	Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace);
	if(aCurve.IsNull() || aCrv2d.IsNull() || aSurf.IsNull())
		return false;

	dLen = QxCurveLib::CurveLength(aCurve,dF,dL);
	dLen *= 0.05;
	if(dLen > 3)	dLen = 3;
	dLen2 = dLen / 5;

	mt = (dF + dL) / 2;
	aCurve->D1(mt,pnt,tang);
	if(m_bTopolDire){
		if(aEdge.Orientation() == TopAbs_REVERSED)
			tang.Reverse();
	}
	tang.Normalize();
	tp.SetCoord(pnt.X() - tang.X() * dLen,pnt.Y() - tang.Y() * dLen,pnt.Z() - tang.Z() * dLen);

	mt = (pF + pL) / 2;
	aCrv2d->D0(mt,p2d);
	aSurf->D1(p2d.X(),p2d.Y(),pnt,D1U,D1V);
	N = D1U.Crossed(D1V);
	offdir = N.Crossed(tang);
	offdir.Normalize();
	p2.SetCoord(tp.X() - offdir.X() * dLen2,tp.Y() - offdir.Y() * dLen2,tp.Z() - offdir.Z() * dLen2);
	p3.SetCoord(tp.X() + offdir.X() * dLen2,tp.Y() + offdir.Y() * dLen2,tp.Z() + offdir.Z() * dLen2);

	kcPreviewCurve *pPrevCrv = new kcPreviewCurve(GetAISContext(),GetDocContext());
	pPrevCrv->Update(aEdge);
	pPrevCrv->SetColor(0.0,0.0,1.0);
	pPrevCrv->Display(TRUE);
	m_prevObjArray.push_back(pPrevCrv);

	kcPreviewTriangle *pPrevTri = new kcPreviewTriangle(GetAISContext(),GetDocContext());
	pPrevTri->Set(pnt,p2,p3);
	pPrevTri->SetColor(1.0,0,0);
	pPrevTri->SetWidth(2.0);
	pPrevTri->Display(TRUE);

	m_prevObjArray.push_back(pPrevTri);

	return true;
}

void	kcmFaceBoundEdgeDire::ClearTempDispObj()
{
	std::vector<kcPreviewObj *>::size_type ix,isize = m_prevObjArray.size();
	for(ix = 0;ix < isize;ix ++){
		delete m_prevObjArray[ix];
	}
	m_prevObjArray.clear();
}

//////////////////////////////////////////////////////////////////////////
//
kcmFaceBoundWire::kcmFaceBoundWire()
{
	m_strName = "边界环";
	m_strAlias = "BndWire";
	m_strDesc = "曲面边界环";

	m_strPrompt = "选择边界环";
	m_nSelModel = KCT_ENT_WIRE;
}

//
BOOL	kcmFaceBoundWire::DoBuildBoundCurve()
{
	if(m_pPickEntityTool->GetSelCount() != 1)
		return FALSE;
	kiSelEntity se = m_pPickEntityTool->GetFirstSelect();
	TopoDS_Shape aS = se._pEntity->GetShape();
	TopoDS_Face aFace;
	TopTools_IndexedDataMapOfShapeListOfShape shapeMap;
	BOOL bNeedFace = FALSE;

	if(IS_FACE_ENTITY(se._pEntity)){
		aFace = TopoDS::Face(aS);
	}
	else
	{
		TopExp::MapShapesAndAncestors(aS,TopAbs_WIRE,TopAbs_FACE,shapeMap);
		bNeedFace = TRUE;
	}

	std::vector<kcEntity *> aEnt;
	kcEntity *pEntity = NULL;
	TopoDS_Wire aWire,aNW;
	kcEntityFactory efac;
	int ix,nbShape = m_pPickLocalShapeTool->GetSelCount();
	for(ix = 0;ix < nbShape;ix ++)
	{
		aWire = TopoDS::Wire(m_pPickLocalShapeTool->SelectedShape(ix));
		if(bNeedFace)
		{
			const TopTools_ListOfShape& shapeList = shapeMap.FindFromKey(aWire);
			if(shapeList.IsEmpty())
				continue;
			aFace = TopoDS::Face(shapeList.First());
		}
		if(aFace.IsNull())
			continue;

		DoBuildEntity(aWire,aFace,aEnt);
	}

	if(aEnt.empty())
		return FALSE;

	kcModel *pModel = GetModel();
	pModel->BeginUndo(GetName());
	pModel->AddEntity(aEnt);
	pModel->EndUndo();

	Redraw();

	return TRUE;
}

BOOL	kcmFaceBoundWire::DoBuildEntity(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,
									    std::vector<kcEntity *>& aEnt)
{
	TopoDS_Wire aNW;
	kcEntity *pEntity = NULL;
	kcEntityFactory efac;

	aNW = QxCurveLib::BuildBoundWire(aWire,aFace);
	if(!aNW.IsNull())
	{
		pEntity = efac.Create(aNW);
		if(pEntity){
			aEnt.push_back(pEntity);
			return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////
kcmFaceRawWire::kcmFaceRawWire()
{
	m_strName = "原始环";
	m_strAlias = "RawWireCurve";
	m_strDesc = "Face的原始顺序环";
}

BOOL	kcmFaceRawWire::DoBuildEntity(const TopoDS_Wire& aWire,const TopoDS_Face& aFace,
									  std::vector<kcEntity *>& aEnt)
{
	TopoDS_Wire aTW;
	TopoDS_Face aTF = aFace;
	aTF.Orientation(TopAbs_FORWARD);

	TopoDS_Iterator ite;
	for(ite.Initialize(aTF,Standard_False);ite.More();ite.Next()){
		aTW = TopoDS::Wire(ite.Value());
		if(aTW.IsSame(aWire))
			break;
	}

	kcEntity *pEntity = NULL;
	kcEntityFactory efac;
	TopoDS_Edge aEdge,aNE;
	TopoDS_Iterator eit;
	double r = 0.1;
	for(eit.Initialize(aTW,Standard_False);eit.More();eit.Next()){
		aEdge = TopoDS::Edge(eit.Value());
		aNE = QxCurveLib::BuildBoundEdge(aEdge,aFace);
		if(!aNE.IsNull()){
			pEntity = efac.Create(aNE);
			if(pEntity)
				aEnt.push_back(pEntity);
		}
		//创建曲线方向标识（圆)
		aNE = QxCurveLib::BuildEdgeDirFlag(aEdge,aFace,r);
		if(!aNE.IsNull()){
			pEntity = efac.Create(aNE);
			if(pEntity)
				aEnt.push_back(pEntity);
		}

		r += 0.02;//逐步增大
	}

	return TRUE;
}

//-----------------------------------------------------------
kcmFaceIsoCurve::kcmFaceIsoCurve()
{
	m_strName = "曲面流线";
	m_strAlias = "FaceIsoCurve";
	m_strDesc = "生成Face上的曲面流线";

	pPickEntityTool_ = NULL;
	pPrvwCurve_ = NULL;
	nIsoDir_ = 0;
	dParam_ = 0.0;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmFaceIsoCurve::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == pPickEntityTool_){
		if(pTool->IsInputDone()){

			if(DoGetSelFace()){
				NavToNextTool();
				PromptMessage("点击曲面上一点创建流线");
			}
			//if(DoBuildIsoCurve()){

			//	Done();
			//	Redraw();

			//	return KSTA_DONE;
			//}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

int  kcmFaceIsoCurve::OnLButtonUp(kuiMouseInput& mouseInput)
{
	if(!aSelFace_.IsNull()){
		QxFacePointPicker picker(aSelFace_,mouseInput.aView_);
		double pnt[3],uv[2];
		if(picker.PickPoint(mouseInput.m_x,mouseInput.m_y,pnt,uv)){
			TRACE("\n pick point (%f,%f,%f),uv (%f,%f).\n",pnt[0],pnt[1],pnt[2],uv[0],uv[1]);
			Handle(Geom_Curve) aCurve;
			if(nIsoDir_ == 0){//U向,OCC计算的是等V参数线，即V固定，因此是U向
				aCurve = DoGetIsoCurve(uv[1]);
			}else{
				aCurve = DoGetIsoCurve(uv[0]);
			}
			if(!aCurve.IsNull()){
				BRepBuilderAPI_MakeEdge me(aCurve);
				if(me.IsDone()){
					TopoDS_Edge aE = me.Edge();

					kcEntityFactory fac;
					kcEntity *pEnt = fac.Create(aE);
					if(pEnt != NULL){

						kcModel *pModel = GetModel();
						pModel->BeginUndo(GetName());
						pModel->AddEntity(pEnt);
						pModel->EndUndo();
					}
				}
			}
		}
	}

	return kiCommand::OnLButtonUp(mouseInput);
}

int  kcmFaceIsoCurve::OnMouseMove(kuiMouseInput& mouseInput)
{
	if(!aSelFace_.IsNull()){
		QxFacePointPicker picker(aSelFace_,mouseInput.aView_);
		double pnt[3],uv[2];
		if(picker.PickPoint(mouseInput.m_x,mouseInput.m_y,pnt,uv)){
			TRACE("\n pick point (%f,%f,%f),uv (%f,%f).\n",pnt[0],pnt[1],pnt[2],uv[0],uv[1]);
			Handle(Geom_Curve) aCurve;
			if(nIsoDir_ == 0){//U向,OCC计算的是等V参数线，即V固定，因此是U向
				aCurve = DoGetIsoCurve(uv[1]);
			}else{
				aCurve = DoGetIsoCurve(uv[0]);
			}
			if(!aCurve.IsNull()){
				if(pPrvwCurve_ == NULL){
					pPrvwCurve_ = new kcPreviewCurve(GetAISContext(),GetDocContext());
				}
				pPrvwCurve_->SetColor(0.0,0.0,1.0);
				pPrvwCurve_->Update(aCurve);

				pPrvwCurve_->Display(TRUE);
			}
		}
	}
	return kiCommand::OnMouseMove(mouseInput);
}

bool  kcmFaceIsoCurve::DoGetSelFace()
{
	int nSel = pPickEntityTool_->GetSelCount();
	if(nSel == 1){
		kiSelEntity se = pPickEntityTool_->GetFirstSelect();
		//
		TopoDS_Shape aS = se.SelectShape();
		if(aS.IsNull() || aS.ShapeType() != TopAbs_FACE){
			return false;
		}
		//
		aSelFace_ = TopoDS::Face(aS);
		return true;
	}
	return false;
}

bool  kcmFaceIsoCurve::DoBuildIsoCurve()
{
	int nSel = pPickEntityTool_->GetSelCount();
	if(nSel == 1){
		kiSelEntity se = pPickEntityTool_->GetFirstSelect();
		//
		TopoDS_Shape aS = se.EntityShape();
		//
		const TopoDS_Face &aFace = TopoDS::Face(aS);
		//
		const Handle(Geom_Surface) &aSurf = BRep_Tool::Surface(aFace);
		if(!aSurf.IsNull()){
			Handle(Geom_Curve) aCurve;
			if(nIsoDir_ == 0){//U向,OCC计算的是等V参数线，即V固定，因此是U向
				aCurve = aSurf->VIso(dParam_);
			}else{
				aCurve = aSurf->UIso(dParam_);
			}

			if(!aCurve.IsNull()){
				BRepBuilderAPI_MakeEdge me(aCurve);
				if(me.IsDone()){
					TopoDS_Edge aE = me.Edge();

					kcEntityFactory fac;
					kcEntity *pEnt = fac.Create(aE);
					if(pEnt != NULL){

						kcModel *pModel = GetModel();
						pModel->BeginUndo(GetName());
						pModel->AddEntity(pEnt);
						pModel->EndUndo();

						return true;
					}
				}
			}
		}
	}
	return false;
}

Handle(Geom_Curve)  kcmFaceIsoCurve::DoGetIsoCurve(double uv)
{
	Handle(Geom_Curve) aCurve;
	if(!aSelFace_.IsNull()){
		const Handle(Geom_Surface) &aSurf = BRep_Tool::Surface(aSelFace_);
		if(!aSurf.IsNull()){
			if(nIsoDir_ == 0){//U向,OCC计算的是等V参数线，即V固定，因此是U向
				aCurve = aSurf->VIso(uv);
			}else{
				aCurve = aSurf->UIso(uv);
			}
		}
	}

	return aCurve;
}

int  kcmFaceIsoCurve::OnExecute()
{
	return kiCommand::OnExecute();
}

int  kcmFaceIsoCurve::OnEnd(int nCode)
{
	if(pPrvwCurve_ != NULL){
		pPrvwCurve_->Display(FALSE);
		delete pPrvwCurve_;
	}

	return kiCommand::OnEnd(nCode);
}

// 创建必要的输入工具
BOOL  kcmFaceIsoCurve::CreateInputTools()
{
	kiOptionEnum *pEnumItem = new kiOptionEnum("流线方向",'O',nIsoDir_);
	pEnumItem->AddEnumItem("U向",'U');
	pEnumItem->AddEnumItem("V向",'V');
	//pEnumItem->AddEnumItem("双向",'B');
	aOptionSet_.AddOption(pEnumItem);

	//kiOptionDouble *pDblItem = new kiOptionDouble("参数值",'P',dParam_);
	//pDblItem->EnablePopup(true);
	//aOptionSet_.AddOption(pDblItem);

	pPickEntityTool_ = new kiInputEntityTool(this,"拾取一个Face",&aOptionSet_);
	pPickEntityTool_->SetOption(KCT_ENT_FACE,true);
	pPickEntityTool_->SetNaturalMode(false);
	
	return TRUE;
}

BOOL  kcmFaceIsoCurve::DestroyInputTools()
{
	KC_SAFE_DELETE(pPickEntityTool_);

	return TRUE;
}
BOOL  kcmFaceIsoCurve::InitInputToolQueue()
{
	AddInputTool(pPickEntityTool_);
	return TRUE;
}

