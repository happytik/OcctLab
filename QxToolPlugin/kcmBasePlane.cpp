#include "stdafx.h"
#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "kiInputDirTool.h"
#include "kiInputValueTool.h"
#include "kiInputPointTool.h"
#include "kiInputEntityTool.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "kcBasePlane.h"
#include "kcmBasePlane.h"


///////////////////////////////////////////////////////
//
kcmOffsetBasePlane::kcmOffsetBasePlane()
{
	m_strName = "OffsetBasePlane";
	m_strAlias = "Obpln";
	m_strDesc = "基准面偏移";

	_pInputDirTool = NULL;
	_pInputOffsetTool = NULL;
	_offDire.set(0,0,1);
	_dOffset = 20;
}

kcmOffsetBasePlane::~kcmOffsetBasePlane()
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmOffsetBasePlane::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmOffsetBasePlane::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == _pInputDirTool){
		if(_pInputDirTool->IsInputDone()){

			NavToNextTool();
		}
	}else if(pTool == _pInputOffsetTool){
		if(_pInputOffsetTool->IsInputDone()){

			if(CreateBasePlane()){
				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL  kcmOffsetBasePlane::CreateInputTools()
{
	_pInputDirTool = new kiInputDirTool(this,"偏移方向",NULL);
	_pInputDirTool->Initialize(&_offDire,false,false,true);

	_pInputOffsetTool = new kiInputDoubleTool(this,"偏移距离",NULL);
	_pInputOffsetTool->Init(&_dOffset);

	return TRUE;
}

// 销毁创建的输入工具.每个命令仅调用一次.
BOOL  kcmOffsetBasePlane::DestroyInputTools()
{
	KC_SAFE_DELETE(_pInputDirTool);
	KC_SAFE_DELETE(_pInputOffsetTool);

	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL  kcmOffsetBasePlane::InitInputToolQueue()
{
	AddInputTool(_pInputDirTool);
	AddInputTool(_pInputOffsetTool);

	return TRUE;
}

BOOL  kcmOffsetBasePlane::CreateBasePlane()
{
	kcModel *pModel = GetModel();
	Handle(V3d_View) hView = GetCurrV3dView();
	kcBasePlane *pCurBPln = GetCurrentBasePlane();
	kvCoordSystem cs;
	kPoint3 org,norg;

	pCurBPln->GetCoordSystem(cs);
	org = cs.Org();
	norg = org + _offDire * _dOffset;
	cs.Set(norg,cs.X(),cs.Y(),cs.Z());

	kcBasePlane *pNewBPln = new kcBasePlane(hView);
	pNewBPln->Create(cs,pModel);
	pModel->AddBasePlane(pNewBPln);

	return TRUE;
}

////////////////////////////////////////////////////////////
//
///三点生成基准面
kcmThreePntBasePlane::kcmThreePntBasePlane()
{
	m_strName = "ThreePntBasePlane";
	m_strAlias = "TPtPln";
	m_strDesc = "三点生成基准面";

	pInputPointTool1_ = NULL;
	pInputPointTool2_ = NULL;
	pInputPointTool3_ = NULL;
}

kcmThreePntBasePlane::~kcmThreePntBasePlane()
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmThreePntBasePlane::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmThreePntBasePlane::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == pInputPointTool1_){
		if(pTool->IsInputDone()){
			NavToNextTool();
		}else{
			return KSTA_CONTINUE;
		}
	}else if(pTool == pInputPointTool2_){
		if(pTool->IsInputDone()){
			NavToNextTool();
		}else{
			return KSTA_CONTINUE;
		}
	}else if(pTool == pInputPointTool3_){
		if(pTool->IsInputDone()){
			if(CreateBasePlane()){
				Done();
				return KSTA_DONE;
			}else{
				return KSTA_CONTINUE;
			}
		}else{
			return KSTA_CONTINUE;
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL  kcmThreePntBasePlane::CreateInputTools()
{
	pInputPointTool1_ = new kiInputPointTool(this,"第一点:");
	pInputPointTool1_->Initialize(&pnt1_);

	pInputPointTool2_ = new kiInputPointTool(this,"第二点:");
	pInputPointTool2_->Initialize(&pnt2_);

	pInputPointTool3_ = new kiInputPointTool(this,"第三点:");
	pInputPointTool3_->Initialize(&pnt3_);

	return TRUE;
}
	
// 销毁创建的输入工具.每个命令仅调用一次.
BOOL  kcmThreePntBasePlane::DestroyInputTools()
{
	KC_SAFE_DELETE(pInputPointTool1_);
	KC_SAFE_DELETE(pInputPointTool2_);
	KC_SAFE_DELETE(pInputPointTool3_);
	return TRUE;
}
	
// 在begin中调用，主要是初始化工具队列。
BOOL  kcmThreePntBasePlane::InitInputToolQueue()
{
	AddInputTool(pInputPointTool1_);
	AddInputTool(pInputPointTool2_);
	AddInputTool(pInputPointTool3_);
	return TRUE;
}

BOOL  kcmThreePntBasePlane::CreateBasePlane()
{
	kcModel *pModel = GetModel();
	Handle(V3d_View) hView = GetCurrV3dView();
	kvCoordSystem cs;
	
	if(!cs.Create(pnt1_,pnt2_,pnt3_))
		return FALSE;

	kcBasePlane *pNewBPln = new kcBasePlane(hView);
	pNewBPln->Create(cs,pModel);
	pModel->AddBasePlane(pNewBPln);

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
kcmBasePlaneByFace::kcmBasePlaneByFace()
{
	InitSet("PlaneFaceBasePlane","PFBasePlane","由Face及其一点生成基准面");

	pFacePickTool_ = NULL;
}

kcmBasePlaneByFace::~kcmBasePlaneByFace()
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL kcmBasePlaneByFace::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int kcmBasePlaneByFace::OnInputFinished(kiInputTool *pTool)
{
	if(pFacePickTool_->IsInputDone()){
		if(CreateBasePlane()){
			Done();
			return KSTA_DONE;
		}else{
			pFacePickTool_->End();

			NavToInputTool(pFacePickTool_);
		}
	}
	return KSTA_CONTINUE;
}

int kcmBasePlaneByFace::OnExecute()
{
	if(pFacePickTool_){
		pFacePickTool_->AddInitSelected(true);
		//
		if(pFacePickTool_->IsInputDone()){
			if(CreateBasePlane()){
				pFacePickTool_->ClearSel();

				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return kiCommand::OnExecute();
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL kcmBasePlaneByFace::CreateInputTools()
{
	pFacePickTool_ = new kiInputEntityTool(this,"拾取一个Face:");
	pFacePickTool_->SetOption(KCT_ENT_FACE, 1);
	pFacePickTool_->SetNaturalMode(false); //允许选取局部对象

	return TRUE;
}
	
// 销毁创建的输入工具.每个命令仅调用一次.
BOOL kcmBasePlaneByFace::DestroyInputTools()
{
	KC_SAFE_DELETE(pFacePickTool_);

	return TRUE;
}
	
// 在begin中调用，主要是初始化工具队列。
BOOL kcmBasePlaneByFace::InitInputToolQueue()
{
	AddInputTool(pFacePickTool_);

	return TRUE;
}

BOOL kcmBasePlaneByFace::CreateBasePlane()
{
	if(pFacePickTool_->GetSelCount() != 1)
		return FALSE;

	kiSelEntity se = pFacePickTool_->GetFirstSelect();
	TopoDS_Shape aS = se.SelectShape();
	TopoDS_Face aFace = TopoDS::Face(aS);
	//
	const Handle(Geom_Surface) &aSurf = BRep_Tool::Surface(aFace);
	if(aSurf.IsNull()){
		return FALSE;
	}

	Handle(Geom_Surface) aBasSurf = aSurf;
	if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		Handle(Geom_RectangularTrimmedSurface) aTrmSurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf);
		aBasSurf = aTrmSurf->BasisSurface();
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface))){
		Handle(Geom_OffsetSurface) aOffSurf = Handle(Geom_OffsetSurface)::DownCast(aSurf);
		aBasSurf = aOffSurf->BasisSurface();
	}
	kcModel *pModel = GetModel();
	Handle(V3d_View) hView = GetCurrV3dView();
	kvCoordSystem cs;
	kcBasePlane *pNewBasePlane = NULL;
	
	//
	if(aBasSurf->IsKind(STANDARD_TYPE(Geom_Plane))){//平面
		Handle(Geom_Plane) aPlane = Handle(Geom_Plane)::DownCast(aBasSurf);
		gp_Pln pln = aPlane->Pln();
		gp_Ax3 ax3 = pln.Position();

		if(aFace.Orientation() == TopAbs_FORWARD){
			cs.Set(pln.Position().Ax2());
		}else{
			gp_Dir zD = ax3.Direction();
			zD.Reverse(); //反向
			cs.Set(ax3.Location(),ax3.XDirection(),zD);
		}

		pNewBasePlane = new kcBasePlane(hView);
		pNewBasePlane->Create(cs,pModel);
		pModel->AddBasePlane(pNewBasePlane);
	}else{
	}
	
	return TRUE;
}

