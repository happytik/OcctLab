#include "StdAfx.h"
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include "kiInputEntityTool.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSolidBoolean.h"

kcmSolidBoolean::kcmSolidBoolean(void)
{
	_pInputEntity1 = NULL;
	_pInputEntity2 = NULL;
	_bParallel = false;
	_bAsHalfSpace = false;
	bKeepTools_ = false;
}

kcmSolidBoolean::~kcmSolidBoolean(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidBoolean::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSolidBoolean::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == _pInputEntity1){
		if(_pInputEntity1->IsInputDone()){
			NavToNextTool();
		}
	}else if(pTool == _pInputEntity2){
		if(_pInputEntity2->IsInputDone()){
			if(DoBoolean()){
				_pInputEntity1->ClearSel();
				_pInputEntity2->ClearSel();

				Done();
				Redraw();
			
				return KSTA_DONE;
			}
		}
	}
	return KSTA_CONTINUE;
}

// 创建必要的输入工具
BOOL	kcmSolidBoolean::CreateInputTools()
{
	//目标对象，目前只支持一个
	_pInputEntity1 = new kiInputEntityTool(this,"拾取实体对象");
	_pInputEntity1->SetOption(KCT_ENT_SOLID | KCT_ENT_SHELL | KCT_ENT_FACE,1);
	_pInputEntity1->SetNaturalMode(true);
	_pInputEntity1->AddInitSelected(true);

	kiOptionBool *pOptItem = new kiOptionBool("并行",'P',2,_bParallel);
	pOptItem->SetOptionCallBack(this);
	_optionSet.AddOption(pOptItem);

	pOptItem = new kiOptionBool("作为半空间",'H',3,_bAsHalfSpace);
	pOptItem->SetOptionCallBack(this);
	_optionSet.AddOption(pOptItem);

	//工具对象，目前支持多个
	_pInputEntity2 = new kiInputEntityTool(this,"拾取工具实体",&_optionSet);
	_pInputEntity2->SetOption(KCT_ENT_SOLID | KCT_ENT_SHELL | KCT_ENT_FACE,false);
	_pInputEntity2->SetNaturalMode(true);

	return TRUE;
}

BOOL	kcmSolidBoolean::DestroyInputTools()
{
	_optionSet.Clear();

	KC_SAFE_DELETE(_pInputEntity1);
	KC_SAFE_DELETE(_pInputEntity2);

	return TRUE;
}

BOOL	kcmSolidBoolean::InitInputToolQueue()
{
	AddInputTool(_pInputEntity1);
	AddInputTool(_pInputEntity2);

	return TRUE;
}

BOOL kcmSolidBoolean::DoBoolean()
{
	kcEntity *pEntity1 = NULL;
	std::vector<kcEntity *> entList1,entList2;
	std::vector<kcEntity *>::size_type ix,isize;
	TopoDS_Shape aShape1;
	TopTools_ListOfShape aLS1,aLS2;
	if(!GetShapesAndEntities(_pInputEntity1,entList1,aLS1) ||
		!GetShapesAndEntities(_pInputEntity2,entList2,aLS2))
		return FALSE;

	TopoDS_Shape aS;
	aShape1 = aLS1.First();
	pEntity1 = entList1[0];

	aS = DoBoolean(aShape1,aLS2);
	if(aS.IsNull())
		return FALSE;

	kcEntityFactory entfac;
	kcEntity *pEnt = NULL;
	std::vector<kcEntity *> aEntList;
	if(aS.ShapeType() == TopAbs_SHELL){
		pEnt = entfac.Create(aS);
		if(pEnt)
			aEntList.push_back(pEnt);
	}if(aS.ShapeType() == TopAbs_SOLID){
		pEnt = entfac.Create(aS);
		if(pEnt)
			aEntList.push_back(pEnt);
	}else if(aS.ShapeType() == TopAbs_COMPOUND){
		TopoDS_Compound aComp = TopoDS::Compound(aS);
		TopoDS_Iterator ite(aComp);
		for(;ite.More();ite.Next()){
			pEnt = entfac.Create(ite.Value());
			if(pEnt)
				aEntList.push_back(pEnt);
		}
	}

	kcModel *pModel = GetModel();
	if(!aEntList.empty()){
		pModel->BeginUndo(GetName());
		//删除旧的
		pModel->DelEntity(pEntity1);
		//工具对象需要保留
		if(!bKeepTools_){
			isize = entList2.size();
			for(ix = 0;ix < isize;ix ++){
				pModel->DelEntity(entList2[ix]);
			}
		}
		//添加新生成的对象
		isize = aEntList.size();
		for(ix = 0;ix < isize;ix ++){
			pModel->AddEntity(aEntList[ix]);
		}
		pModel->EndUndo();
	}

	return TRUE;
}

BOOL	kcmSolidBoolean::GetSolidEntity(kiInputEntityTool *pTool,kcSolidEntity *&pEntity)
{
	if(!pTool->IsInputDone())
		return FALSE;

	kiSelSet *pSelSet = pTool->GetSelSet();
	if(pSelSet->GetSelCount() != 1)
		return FALSE;

	pSelSet->InitSelected();
	kiSelEntity se = pSelSet->CurSelected();
	if(!IS_SOLID_ENTITY(se._pEntity))
		return FALSE;

	pEntity = (kcSolidEntity *)se._pEntity;
	return TRUE;
}

BOOL  kcmSolidBoolean::GetShapeAndEntity(kiInputEntityTool *pTool,kcEntity *&pEntity,TopoDS_Shape& aShape)
{
	if(!pTool->IsInputDone())
		return FALSE;

	kiSelSet *pSelSet = pTool->GetSelSet();
	if(pSelSet->GetSelCount() != 1)
		return FALSE;

	pSelSet->InitSelected();
	kiSelEntity se = pSelSet->CurSelected();
	if(IS_SHELL_ENTITY(se._pEntity)){
		kcShellEntity *pShellEntity = (kcShellEntity *)se._pEntity;
		pEntity = pShellEntity;
		aShape = pShellEntity->GetShell();

	}else if(IS_SOLID_ENTITY(se._pEntity)){
		kcSolidEntity *pSolidEntity = (kcSolidEntity *)se._pEntity;
		aShape = pSolidEntity->GetSolid();
		pEntity = pSolidEntity;

	}else if(IS_WIRE_ENTITY(se._pEntity)){
		kcWireEntity *pWireEntity = (kcWireEntity *)se._pEntity;
		aShape = pWireEntity->GetWire();
		pEntity = pWireEntity;
	}else if(IS_FACE_ENTITY(se._pEntity)){
		kcFaceEntity *pFaceEntity = (kcFaceEntity *)se._pEntity;
		aShape = pFaceEntity->GetFace();
		pEntity = pFaceEntity;
	}else{
		return FALSE;
	}

	return TRUE;
}

BOOL  kcmSolidBoolean::GetShapesAndEntities(kiInputEntityTool *pTool,
									   std::vector<kcEntity *>& entList,TopTools_ListOfShape& aLS)
{
	if(!pTool || !pTool->IsInputDone())
		return FALSE;

	kiSelSet *pSelSet = pTool->GetSelSet();
	for(pSelSet->InitSelected();pSelSet->MoreSelected();pSelSet->NextSelected()){
		kiSelEntity se = pSelSet->CurSelected();
		if(IS_FACE_ENTITY(se._pEntity) || IS_SHELL_ENTITY(se._pEntity) || IS_SOLID_ENTITY(se._pEntity)){
			entList.push_back(se._pEntity);
			TopoDS_Shape aS = se._pEntity->GetShape();
			//是否face生成半空间实体
			if(_bAsHalfSpace){
				if(aS.ShapeType() == TopAbs_FACE){
					aS = MakeHalfSolid(TopoDS::Face(aS),false);
				}
			}
			if(!aS.IsNull()){
				aLS.Append(aS);
			}
		}
	}

	return (entList.empty() ? FALSE : TRUE);
}

//使用一个face生成一个半空间实体
TopoDS_Shape  kcmSolidBoolean::MakeHalfSolid(const TopoDS_Face& aFace,bool bRev)
{
	TopoDS_Shape aHS = aFace;
	if(aFace.IsNull())
		return aHS;

	BRepAdaptor_Surface adpSurf(aFace);
	double mu = (adpSurf.FirstUParameter() + adpSurf.LastUParameter())/2;
	double mv = (adpSurf.FirstVParameter() + adpSurf.LastVParameter())/2;
	gp_Pnt pnt;
	gp_Vec D1U,D1V,aNorm;
	
	adpSurf.D1(mu,mv,pnt,D1U,D1V);
	aNorm = D1U.Crossed(D1V).Normalized();
	if(!bRev){
		aNorm.Reverse();
	}

	pnt.ChangeCoord() += aNorm.XYZ();

	try{
		BRepPrimAPI_MakeHalfSpace makeHS(aFace,pnt);
		if(makeHS.IsDone()){
			aHS = makeHS.Solid();
		}
	}catch(Standard_Failure){
	}

	return aHS;
}

//////////////////////////////////////////////////////////////////////////
// 并
kcmSolidBoolFuse::kcmSolidBoolFuse()
{
	m_strName = "并集";
	m_strAlias = "SolidFuse";
	m_strDesc = "实体并集";
}

TopoDS_Shape	kcmSolidBoolFuse::DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS)
{
	TopoDS_Shape aS;
	TopTools_ListOfShape aArgLS;
	aArgLS.Append(aS1);

	try{
		BRepAlgoAPI_Fuse fuse;

		fuse.SetArguments(aArgLS);
		fuse.SetTools(aLS);
		fuse.SetRunParallel(_bParallel);
		//fuse.SetGlue(BOPAlgo_GlueShift);

		fuse.Build();
		if(!fuse.IsDone() || fuse.HasErrors()){
			AfxMessageBox("Boolean option Fuse failed.");
			return aS;
		}

		//
		//fuse.RefineEdges();
		
		aS = fuse.Shape();

		ShapeUpgrade_UnifySameDomain usd(aS);
		usd.Build();
		TopoDS_Shape aUS = usd.Shape();
		if(!aUS.IsNull()){
			aS = aUS;
		}

	}catch(Standard_Failure){
	}

	return aS;
}


//////////////////////////////////////////////////////////////////////////
// 交
kcmSolidBoolCommon::kcmSolidBoolCommon()
{
	m_strName = "交集";
	m_strAlias = "SolidComm";
	m_strDesc = "实体交集";
}

TopoDS_Shape	kcmSolidBoolCommon::DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS)
{
	TopoDS_Shape aS;
	TopTools_ListOfShape aArgLS;
	aArgLS.Append(aS1);

	try{
		BRepAlgoAPI_Common comm;

		comm.SetArguments(aArgLS);
		comm.SetTools(aLS);
		comm.SetRunParallel(_bParallel);

		comm.Build();
		if(!comm.IsDone() || comm.HasErrors()){
			AfxMessageBox("Boolean option comm failed.");
			return aS;
		}

		aS = comm.Shape();
	}catch(Standard_Failure){
	}

	return aS;
}

//////////////////////////////////////////////////////////////////////////
// 差
kcmSolidBoolCut::kcmSolidBoolCut()
{
	m_strName = "差集";
	m_strAlias = "SolidCut";
	m_strDesc = "实体差集";
}

TopoDS_Shape	kcmSolidBoolCut::DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS)
{
	TopoDS_Shape aS;
	TopTools_ListOfShape aArgLS;
	aArgLS.Append(aS1);

	try{
		BRepAlgoAPI_Cut fcut;

		fcut.SetArguments(aArgLS);
		fcut.SetTools(aLS);
		fcut.SetRunParallel(_bParallel);

		fcut.Build();
		if(!fcut.IsDone() || fcut.HasErrors()){
			AfxMessageBox("Boolean option Fcut failed.");
			return aS;
		}

		aS = fcut.Shape();
	}catch(Standard_Failure){
	}

	return aS;
}

////////////////////////////////////////////////////////////////////////////////
//
//分割
kcmSolidSplit::kcmSolidSplit()
{
	m_strName = "实体分割";
	m_strAlias = "SolidSplit";
	m_strDesc = "分割曲面和实体对象";

	bKeepTools_ = true;
}

TopoDS_Shape  kcmSolidSplit::DoBoolean(const TopoDS_Shape& aS1,const TopTools_ListOfShape& aLS)
{
	TopoDS_Shape aS;
	TopTools_ListOfShape aArgLS;
	aArgLS.Append(aS1);

	try{
		BRepAlgoAPI_Splitter BSplit;

		BSplit.SetArguments(aArgLS);
		BSplit.SetTools(aLS);
		BSplit.SetRunParallel(_bParallel);

		BSplit.Build();
		if(!BSplit.IsDone() || BSplit.HasErrors()){
			AfxMessageBox("Boolean option Split failed.");
			return aS;
		}

		aS = BSplit.Shape();
	}catch(Standard_Failure){
	}

	return aS;
}
