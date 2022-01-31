#include "StdAfx.h"
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <ShapeFix_Solid.hxx>
#include <BRepCheck_Shell.hxx>
#include "kiInputEntityTool.h"
#include "kiInputVector.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxBRepLib.h"
#include "kvCoordSystem.h"
#include "kcEntityFactory.h"
#include "kcmSolidSewing.h"
#include <istream>

kcmSolidSewing::kcmSolidSewing(void)
{
	m_strName = "曲面缝合";
	m_strAlias = "FaceSew";
	m_strDesc = "曲面缝合";

	m_pInputEntity = NULL;
	m_dSewingTol = 0.001;
}

kcmSolidSewing::~kcmSolidSewing(void)
{
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL	kcmSolidSewing::CanFinish()//命令是否可以完成
{
	return FALSE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int		kcmSolidSewing::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputEntity)
	{
		if(m_pInputEntity->IsInputDone())
		{
			if(DoSewing())
			{
				m_pInputEntity->ClearSel();
				Done();
				Redraw();

				return KSTA_DONE;
			}
			else
			{
				m_pInputEntity->ClearSel();
				EndCommand(KSTA_ERROR);
				
				return KSTA_ERROR;
			}
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

BOOL	kcmSolidSewing::DoSewing()
{
	kiSelSet *pSelSet = m_pInputEntity->GetSelSet();
	if(!pSelSet || pSelSet->GetSelCount() <= 1)
		return FALSE;

	CWaitCursor wc;
	TopoDS_Shape aNS;
	kcEntityList aSelEntList;
	try{
		BRepBuilderAPI_Sewing sew(m_dSewingTol);
		TopoDS_Shape aShape,aCS;
		kiSelEntity se;

		pSelSet->InitSelected();
		while(pSelSet->MoreSelected()){
			se = pSelSet->CurSelected();
			aSelEntList.push_back(se._pEntity);

			aShape = se._pEntity->GetShape();
			aCS = QxBRepLib::CopyShape(aShape);
			if(!aCS.IsNull()){
				sew.Add(aCS);
			}

			pSelSet->NextSelected();
		}

		sew.Perform();
		aNS = sew.SewedShape();
		if(aNS.IsNull())
			return FALSE;
	}catch(Standard_Failure){
		return FALSE;
	}

	if(aNS.ShapeType() == TopAbs_SHELL){
		try{
			BRepCheck_Shell chkShell(TopoDS::Shell(aNS));
			if(BRepCheck_NoError == chkShell.Closed()){//shell is closed
			
				TopoDS_Shape aSS;
				BRepBuilderAPI_MakeSolid ms(TopoDS::Shell(aNS));
				if(ms.IsDone()){
					aSS = ms.Shape();

					ShapeFix_Solid fs(TopoDS::Solid(aSS));
					fs.Perform();
					TopoDS_Shape aSolid = fs.Solid();
					if(!aSolid.IsNull())
						aNS = aSolid;
				}
			}
		}catch (Standard_Failure){
			
		}
	}

	kcEntityFactory fac;
	kcEntity *pEntity = fac.Create(aNS);
	if(pEntity){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());

		pModel->AddEntity(pEntity);
		//删除旧的
		kcEntityList::iterator ite = aSelEntList.begin();
		for(;ite != aSelEntList.end();ite ++){
			pModel->DelEntity(*ite);
		}

		pModel->EndUndo();

		Redraw();

		return TRUE;
	}
	
	return FALSE;
}


int		kcmSolidSewing::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kcmSolidSewing::OnEnd(int nCode)
{
	m_pInputEntity->ClearSel();
	return nCode;
}

// 创建必要的输入工具
BOOL	kcmSolidSewing::CreateInputTools()
{
	m_optionSet.AddDoubleOption("缝合精度",'T',m_dSewingTol);
	m_pInputEntity = new kiInputEntityTool(this,"拾取要缝合曲面",&m_optionSet);
	m_pInputEntity->SetOption(KCT_ENT_FACE | KCT_ENT_SHELL);
	m_pInputEntity->SetNaturalMode(true);
	//添加初始选择对象
	m_pInputEntity->AddInitSelected(false);

	return TRUE;
}

BOOL	kcmSolidSewing::DestroyInputTools()
{
	KC_SAFE_DELETE(m_pInputEntity);
	return TRUE;
}

BOOL	kcmSolidSewing::InitInputToolQueue()
{
	AddInputTool(m_pInputEntity);
	return TRUE;
}


