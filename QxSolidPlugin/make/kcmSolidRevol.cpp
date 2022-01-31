#include "StdAfx.h"
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "kvCoordSystem.h"
#include "kcmSolidRevol.h"

#define KC_STA_INPUT_ENTITY		1
#define KC_STA_INPUT_VEC		2

kcmSolidRevol::kcmSolidRevol(void)
{
	m_strName = "Revol Solid";
	m_strAlias = "RevolSolid";
	m_strDesc = "Revol face to solid";

	m_nState = KC_STA_INPUT_ENTITY;
	m_pInputEntity = NULL;
	m_pInputVec = NULL;
	m_dAngle = 180.0;
}

kcmSolidRevol::~kcmSolidRevol(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidRevol::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int	kcmSolidRevol::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->IsInputDone())
		{
			m_nState = KC_STA_INPUT_VEC;

			NavToNextTool();
		}
	}
	else if(pTool == m_pInputVec)
	{
		if(m_pInputVec->IsInputDone())
		{
			//生成旋转面
			if(MakeRevolShape())
			{
				m_pInputEntity->ClearSel();

				Done();
				Redraw();

				return KSTA_DONE;
			}
		}
	}
	return KSTA_CONTINUE;
}


int	kcmSolidRevol::OnExecute()
{
	return KSTA_CONTINUE;
}

int	kcmSolidRevol::OnEnd(int nCode)
{
	return nCode;
}

//
BOOL	kcmSolidRevol::MakeRevolShape()
{
	kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
	if(pSelSet->GetSelCount() != 1)
		return FALSE;

	pSelSet->InitSelected();
	kiSelEntity se = pSelSet->CurSelected();
	if(!IS_FACE_ENTITY(se._pEntity))
		return FALSE;

	kvCoordSystem cs;
	kcBasePlane *pWorkPlane = GetCurrentBasePlane();
	pWorkPlane->GetCoordSystem(cs);
	kPoint3 org; 
	kVector3 vec;
	m_pInputVec->GetPoint(org);
	m_pInputVec->GetVector(vec);
	vec.normalize();
	kVector3 z = cs.Z();
	kVector3 x = z ^ vec;
	x.normalize();

	gp_Ax1 ax1(gp_Pnt(org[0],org[1],org[2]),gp_Dir(vec[0],vec[1],vec[2]));
	double rad = m_dAngle * K_PI / 180.0;

	kcEntity *pNewEntity = NULL;
	kcFaceEntity *pSurEnt = (kcFaceEntity *)se._pEntity;
	TopoDS_Face aFace = pSurEnt->GetFace();



	TopoDS_Shape aShape;
	try{
		BRepPrimAPI_MakeRevol mr(aFace,ax1,rad,Standard_True);
		if(mr.IsDone())
		{
			aShape = mr.Shape();
			if(aShape.IsNull())
				return FALSE;
		}
	}catch(Standard_Failure){
		return FALSE;
	}

	if(aShape.ShapeType() == TopAbs_SOLID)
	{
		TopoDS_Solid aSolid = TopoDS::Solid(aShape);
		kcSolidEntity *pSolidEnt = new kcSolidEntity;
		pSolidEnt->SetSolid(aSolid);
		pNewEntity = pSolidEnt;
	}

	if(pNewEntity)
	{
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		pModel->AddEntity(pNewEntity);
		pModel->EndUndo();
	}

	return TRUE;
}

// 创建必要的输入工具
BOOL	kcmSolidRevol::CreateInputTools()
{
	kiOptionItem *pItem = new kiOptionDouble("旋转角度",'A',m_dAngle);
	m_optionSet.AddOption(pItem);

	m_pInputEntity = new kiInputEntityTool(this,"选择一个曲面");
	m_pInputEntity->SetOption(KCT_ENT_FACE,1);

	m_pInputVec = new kiInputVector(this,"选择旋转轴",&m_optionSet);
	m_pInputVec->SetType(eInputWorkPlaneVector);

	return TRUE;
}

BOOL	kcmSolidRevol::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);
	KC_SAFE_DELETE(m_pInputVec);

	return TRUE;
}

BOOL	kcmSolidRevol::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	AddInputTool(m_pInputVec);

	return TRUE;
}

