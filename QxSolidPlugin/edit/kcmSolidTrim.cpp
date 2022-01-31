
#include "stdafx.h"
#include <vector>
#include <BRepTools.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include "kcEntity.h"
#include "QxBRepLib.h"
#include "kcEntityFactory.h"
#include "kcModel.h"
#include "kiInputEntityTool.h"
#include "kcDispArrow.h"
#include "kcmSolidTrim.h"

kcmSolidTrimByShell::kcmSolidTrimByShell(void)
{
	m_strName = "TrimByShell";
	m_strAlias = "TrimBS";
	m_strDesc = "曲面裁剪实体";

	m_pInputSolidTool = NULL;
	m_pInputShellTool = NULL;
	_bRevDir = false;
	_bParallel = false;
	_pDirArrow = NULL;
}

kcmSolidTrimByShell::~kcmSolidTrimByShell(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmSolidTrimByShell::CanFinish()//命令是否可以完成
{
	return FALSE;
}

int  kcmSolidTrimByShell::OnExecute()
{
	return KSTA_CONTINUE;
}

int  kcmSolidTrimByShell::OnEnd(int nCode)
{
	KC_SAFE_DELETE(_pDirArrow);
	return KSTA_CONTINUE;
}


// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmSolidTrimByShell::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputSolidTool){
		if(m_pInputSolidTool->IsInputDone()){
			NavToNextTool();
		}
	}else if(pTool == m_pInputShellTool){
		if(m_pInputShellTool->IsInputDone()){

			if(DoTrim()){
				Done();
				return KSTA_DONE;
			}else{
				Done();
				return KSTA_FAILED;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

static BOOL uxGetFaceRefPnt(const TopoDS_Face& aFace,gp_Pnt& pnt,gp_Pnt& spos,gp_Dir& norm)
{
	double U1,U2,V1,V2,u,v;
	BRepTools::UVBounds(aFace,U1,U2,V1,V2);
	u = (U1 + U2) / 2;
	v = (V1 + V2) / 2;

	BRepAdaptor_Surface BAS(aFace);
	BRepLProp_SLProps slProp(BAS,u,v,1,1e-10);
	if(slProp.IsNormalDefined()){
		spos = slProp.Value();
		norm = slProp.Normal();
		gp_XYZ v = spos.XYZ() + norm.XYZ() * 1.0 ;
		pnt.SetXYZ(v);

		return TRUE;
	}

	return FALSE;
}

int  kcmSolidTrimByShell::OnInputChanged(kiInputTool *pTool)
{
	if(pTool == m_pInputShellTool){
		ShowDirArrow();
	}
	return kiCommand::OnInputChanged(pTool);
}

void  kcmSolidTrimByShell::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if(pOptionItem && pOptionItem->GetID() == 1){
		ShowDirArrow();
	}
}

void  kcmSolidTrimByShell::ShowDirArrow()
{
	if(m_pInputShellTool->GetSelCount() > 0){
		kiSelEntity se = m_pInputShellTool->GetFirstSelect();
		if(se._pEntity){
			gp_Pnt refpnt,spos;
			gp_Dir norm;
			BOOL brc;
			TopoDS_Shape aShape = se._pEntity->GetShape();
			if(aShape.ShapeType() == TopAbs_FACE){
				brc = uxGetFaceRefPnt(TopoDS::Face(aShape),refpnt,spos,norm);
			}else{
				TopExp_Explorer exp;
				exp.Init(aShape,TopAbs_FACE);
				brc = uxGetFaceRefPnt(TopoDS::Face(exp.Current()),refpnt,spos,norm);
			}

			if(brc){
				if(_bRevDir) norm.Reverse();

				if(!_pDirArrow){
					_pDirArrow = new kcDispSolidArrow(GetModel()->GetAISContext(),GetDocContext());
				}
				kPoint3 org;
				kVector3 axdir;
				org.set(spos.X(),spos.Y(),spos.Z());
				axdir.set(norm.X(),norm.Y(),norm.Z());

				_pDirArrow->Init(org,axdir,50);
				_pDirArrow->SetColor(kColor(1.0,0,0));
				_pDirArrow->Display(TRUE);
			}
		}
	}
}

BOOL  kcmSolidTrimByShell::DoTrim()
{
	kcEntity *pSolidEnt = NULL,*pShellEnt = NULL;
	kiSelEntity se = m_pInputSolidTool->GetFirstSelect();
	pSolidEnt = se._pEntity;

	se = m_pInputShellTool->GetFirstSelect();
	pShellEnt = se._pEntity;

	gp_Pnt refPnt,spos;
	gp_Dir norm;
	TopoDS_Shape aHalfSpaceSolid;
	if(pShellEnt->GetType() == KCT_ENT_FACE){
		TopoDS_Face aFace = TopoDS::Face(pShellEnt->GetShape());
		if(uxGetFaceRefPnt(aFace,refPnt,spos,norm)){
			try{
				BRepPrimAPI_MakeHalfSpace MHS(aFace,refPnt);
				aHalfSpaceSolid = MHS.Solid();
			}catch(Standard_Failure){
			}
		}
	}else{
		TopoDS_Shell aShell = TopoDS::Shell(pShellEnt->GetShape());
		TopExp_Explorer exp;
		for(exp.Init(aShell,TopAbs_FACE);exp.More();exp.Next()){
			TopoDS_Face aFace = TopoDS::Face(exp.Current());
			if(uxGetFaceRefPnt(aFace,refPnt,spos,norm)){
				BRepPrimAPI_MakeHalfSpace MHS(aShell,refPnt);
				aHalfSpaceSolid = MHS.Solid();
				if(!aHalfSpaceSolid.IsNull())
					break;
			}
		}
	}

	if(aHalfSpaceSolid.IsNull()){
		AfxMessageBox("Make Half Space Solid Failed.");
		return FALSE;
	}

	CWaitCursor wc;
	TopoDS_Shape aNS;
	DWORD dwBegin = ::GetTickCount();
	TopoDS_Shape aSolid = pSolidEnt->GetShape();
	try{
		TopTools_ListOfShape aArgLS,aTools;
		aArgLS.Append(aSolid);
		aTools.Append(aHalfSpaceSolid);
		if(!_bRevDir){
			BRepAlgoAPI_Common comm;
			comm.SetArguments(aArgLS);
			comm.SetTools(aTools);
			comm.SetRunParallel(_bParallel);
			comm.Build();
			if(comm.IsDone()){
				aNS = comm.Shape();
			}
		}else{
			BRepAlgoAPI_Cut bcut;
			bcut.SetArguments(aArgLS);
			bcut.SetTools(aTools);
			bcut.SetRunParallel(_bParallel);
			bcut.Build();
			if(bcut.IsDone()){
				aNS = bcut.Shape();
			}
		}
	}catch(Standard_Failure){
	}
	TRACE("\n booloption use %d msec.",::GetTickCount() - dwBegin);

	if(aNS.IsNull()){
		AfxMessageBox("布尔运算失败.");
		return FALSE;
	}

	kcEntityFactory enfc;
	std::vector<kcEntity *> entList;
	if(enfc.Create(aNS,entList,true)){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		//删除旧的
		pModel->DelEntity(pSolidEnt);
		//pModel->DelEntity(pShellEnt);
		std::vector<kcEntity *>::size_type ix,isize = entList.size();
		for(ix = 0;ix < isize;ix ++){
			pModel->AddEntity(entList[ix]);
		}
		pModel->EndUndo();

		return TRUE;
	}
	
	return FALSE;
}

BOOL  kcmSolidTrimByShell::CreateInputTools()
{
	m_pInputSolidTool = new kiInputEntityTool(this,"选择被裁剪对象(Face/Shell/Solid)",NULL);
	m_pInputSolidTool->SetOption(KCT_ENT_SOLID | KCT_ENT_SHELL | KCT_ENT_FACE,1);
	m_pInputSolidTool->SetNaturalMode(true);

	kiOptionBool *pOptItem = new kiOptionBool("反向",'R',1,_bRevDir);
	pOptItem->SetOptionCallBack(this);
	_optionSet.AddOption(pOptItem);

	pOptItem = new kiOptionBool("并行",'P',2,_bParallel);
	pOptItem->SetOptionCallBack(this);
	_optionSet.AddOption(pOptItem);

	m_pInputShellTool = new kiInputEntityTool(this,"选择用于裁剪的曲面(Face/Shell)",&_optionSet);
	m_pInputShellTool->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL,true);
	m_pInputShellTool->SetNaturalMode(true);

	return TRUE;
}

BOOL  kcmSolidTrimByShell::DestroyInputTools()
{
	_optionSet.Clear();

	KC_SAFE_DELETE(m_pInputSolidTool);
	KC_SAFE_DELETE(m_pInputShellTool);

	return TRUE;
}

BOOL  kcmSolidTrimByShell::InitInputToolQueue()
{
	AddInputTool(m_pInputSolidTool);
	AddInputTool(m_pInputShellTool);

	return TRUE;
}

//////////////////////////////////////////////////////////////////
//
kcmSolidTrimSplit::kcmSolidTrimSplit(void)
{
	m_strName = "TrimSplit";
	m_strAlias = "TrimSp";
	m_strDesc = "分割裁剪";

	m_pInputObjTool1 = NULL;
	m_pInputObjTool2 = NULL;
	_bParallel = false;
}

kcmSolidTrimSplit::~kcmSolidTrimSplit(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmSolidTrimSplit::CanFinish()//命令是否可以完成
{
	if(m_pInputObjTool1 && m_pInputObjTool1->IsInputDone() &&
		m_pInputObjTool2 && m_pInputObjTool2->IsInputDone())
		return TRUE;

	return FALSE;
}

int  kcmSolidTrimSplit::OnExecute()
{
	return KSTA_CONTINUE;
}

int  kcmSolidTrimSplit::OnEnd(int nCode)
{
	return KSTA_CONTINUE;
}


// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmSolidTrimSplit::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputObjTool1){
		if(m_pInputObjTool1->IsInputDone()){
			NavToNextTool();
		}
	}else if(pTool == m_pInputObjTool2){
		if(m_pInputObjTool2->IsInputDone()){

			if(DoSplit()){
				Done();
				return KSTA_DONE;
			}else{
				Done();
				return KSTA_FAILED;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

BOOL  kcmSolidTrimSplit::GetInputEntList(kiInputEntityTool *pTool,kcEntityList &entList,
						  		    TopTools_ListOfShape& aShapeList,TopTools_ListOfShape& aCpyShapeList)
{
	kiSelSet *pSelSet = pTool->GetSelSet();
	for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected()){
		kiSelEntity se = pSelSet->CurSelected();
		//
		entList.push_back(se._pEntity);
		//拷贝shape
		TopoDS_Shape aShape = se._pEntity->GetShape();
		aShapeList.Append(aShape);
		//拷贝一份
		TopoDS_Shape aCpy = QxBRepLib::CopyShape(aShape);
		if(aCpy.IsNull()){
			ASSERT(FALSE);
			return FALSE;
		}
		aCpyShapeList.Append(aCpy);
	}
	return TRUE;
}

//进行分割运算
static TopoDS_Shape uxDoSplit(const TopTools_ListOfShape &aArgLS,
							  const TopTools_ListOfShape &aToolLS,
							  bool bParallel)
{
	TopoDS_Shape aResult;
	try{
		BRepAlgoAPI_Splitter BSplit;

		BSplit.SetArguments(aArgLS);
		BSplit.SetTools(aToolLS);
		BSplit.SetRunParallel(bParallel);

		BSplit.Build();
		if(!BSplit.IsDone() || BSplit.HasErrors()){
			AfxMessageBox("Boolean option Split failed.");
			return aResult;
		}

		aResult = BSplit.Shape();
	}catch(Standard_Failure){
	}

	return aResult;
}

//从shape，创建entlist
static void uxBuildEntList(const TopoDS_Shape &aS,kcEntityList &entList)
{
	kcEntityFactory entfac;
	kcEntity *pEnt = NULL;

	if(aS.ShapeType() == TopAbs_FACE || aS.ShapeType() == TopAbs_SHELL ||
		aS.ShapeType() == TopAbs_SOLID)
	{
		pEnt = entfac.Create(aS);
		if(pEnt){
			entList.push_back(pEnt);
		}
	}else if(aS.ShapeType() == TopAbs_COMPOUND){
		TopoDS_Compound aComp = TopoDS::Compound(aS);
		TopoDS_Iterator ite(aComp);
		for(;ite.More();ite.Next()){
			pEnt = entfac.Create(ite.Value());
			if(pEnt){
				entList.push_back(pEnt);
			}
		}
	}
}

BOOL  kcmSolidTrimSplit::DoSplit()
{
	kcEntityList entList1,entList2,entList;
	TopTools_ListOfShape shpList1,shpList2,cpShpList1,cpShpList2;

	if(!GetInputEntList(m_pInputObjTool1,entList1,shpList1,cpShpList1) ||
		!GetInputEntList(m_pInputObjTool2,entList2,shpList2,cpShpList2))
	{
		return FALSE;
	}

	CWaitCursor wc;

	//进行相互分割
	TopoDS_Shape aS1,aS2;
	aS1 = uxDoSplit(cpShpList1,shpList2,_bParallel);
	aS2 = uxDoSplit(cpShpList2,shpList1,_bParallel);

	if(aS1.IsNull() && aS2.IsNull()){
		return FALSE;
	}

	uxBuildEntList(aS1,entList);
	uxBuildEntList(aS2,entList);
	
	if(!entList.empty()){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		//删除旧的
		kcEntityList::iterator ite;
		for(ite = entList1.begin();ite != entList1.end();ite ++){
			pModel->DelEntity((*ite),FALSE);
		}
		for(ite = entList2.begin();ite != entList2.end();ite ++){
			pModel->DelEntity((*ite),FALSE);
		}
		
		//添加新的
		for(ite = entList.begin();ite != entList.end();ite ++){
			pModel->AddEntity(*ite);
		}
		pModel->EndUndo();

		return TRUE;
	}
	
	return FALSE;
}

BOOL  kcmSolidTrimSplit::CreateInputTools()
{
	m_pInputObjTool1 = new kiInputEntityTool(this,"选择第一组曲面或体",NULL);
	m_pInputObjTool1->SetOption(KCT_ENT_SOLID | KCT_ENT_SHELL | KCT_ENT_FACE,false);

	kiOptionBool *pOptItem = new kiOptionBool("并行",'P',1,_bParallel);
	pOptItem->SetOptionCallBack(this);
	_optionSet.AddOption(pOptItem);

	m_pInputObjTool2 = new kiInputEntityTool(this,"选择第二则曲面或体",&_optionSet);
	m_pInputObjTool2->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL | KCT_ENT_SOLID,true);

	return TRUE;
}

BOOL  kcmSolidTrimSplit::DestroyInputTools()
{
	_optionSet.Clear();

	KC_SAFE_DELETE(m_pInputObjTool1);
	KC_SAFE_DELETE(m_pInputObjTool2);

	return TRUE;
}

BOOL  kcmSolidTrimSplit::InitInputToolQueue()
{
	AddInputTool(m_pInputObjTool1);
	AddInputTool(m_pInputObjTool2);

	return TRUE;
}