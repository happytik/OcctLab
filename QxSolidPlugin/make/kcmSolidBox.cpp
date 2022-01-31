#include "StdAfx.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include "kiInputPoint.h"
#include "kiInputValueTool.h"
#include "kcPreviewObj.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcModel.h"
#include "QxCurveLib.h"
#include "kcmSolidBox.h"

#define		STA_BOX_INPUT_POINT1	1
#define		STA_BOX_INPUT_POINT2	2
#define		STA_BOX_INPUT_HEIGHT	3

kcmSolidBox::kcmSolidBox(void)
{
	m_strName = "SolidBox";
	m_strAlias = "SoBox";
	m_strDesc = "立方体";

	m_pInputPoint1 = NULL;
	m_pInputPoint2 = NULL;
	m_dHeight = 20;

	m_pPreviewRect = NULL;
	m_pPreviewBox = NULL;
}

kcmSolidBox::~kcmSolidBox(void)
{
}

// 执行一次初始化和释放的地方。
BOOL	kcmSolidBox::CanFinish()//命令是否可以完成
{
	return FALSE;
}

//输入工具结束时
int	kcmSolidBox::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputPoint1)
	{
		m_nState = STA_BOX_INPUT_POINT2;
		NavToNextTool();
	}
	else if(pTool == m_pInputPoint2)
	{
		m_nState = STA_BOX_INPUT_HEIGHT;
		NavToNextTool();
	}
	else if(pTool == m_pInputHeight)
	{
		//创建box.
		if(BuildBox())
		{
			Done();

			return KSTA_DONE;
		}
	}
	return kiCommand::OnInputFinished(pTool);
}

//具体的初始化和释放代码,每个子命令可以做各自的初始化和销毁操作。
int		kcmSolidBox::OnInitialize()
{
	return KSTA_CONTINUE;
}

int		kcmSolidBox::OnDestroy()
{
	KC_SAFE_DELETE(m_pInputPoint1);
	KC_SAFE_DELETE(m_pInputPoint2);
	KC_SAFE_DELETE(m_pInputHeight);

	KC_SAFE_DELETE(m_pPreviewRect);
	KC_SAFE_DELETE(m_pPreviewBox);

	return KSTA_CONTINUE;
}

// 执行函数
int		kcmSolidBox::OnExecute()
{
	m_nState = STA_BOX_INPUT_POINT1;

	return KSTA_CONTINUE;
}

int		kcmSolidBox::OnEnd(int nCode)
{
	KC_SAFE_DELETE(m_pPreviewRect);
	KC_SAFE_DELETE(m_pPreviewBox);

	return KSTA_CONTINUE;
}

int		kcmSolidBox::OnApply()
{
	return KSTA_CONTINUE;
}

// 创建必要的输入工具
BOOL	kcmSolidBox::CreateInputTools()
{
	m_pInputPoint1 = new kiInputPoint(this,"输入第一个点:");
	m_pInputPoint2 = new kiInputPoint(this,"输入第二个点:");
	m_pInputHeight = new kiInputDoubleTool(this,"输入高度:");

	m_pInputPoint1->Init(&m_aPnt1,false);
	m_pInputPoint2->Init(&m_aPnt2,false);
	m_pInputPoint2->SetRealtimeUpdate(true);
	m_pInputHeight->Init(&m_dHeight,-10000,10000);

	return TRUE;
}

BOOL	kcmSolidBox::InitInputToolQueue()
{
	AddInputTool(m_pInputPoint1);
	AddInputTool(m_pInputPoint2);
	AddInputTool(m_pInputHeight);

	return TRUE;
}

int  kcmSolidBox::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	if(m_nState == STA_BOX_INPUT_POINT2)
	{
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);

		if(m_pPreviewRect == NULL)
		{
			m_pPreviewRect = new kcPreviewRect(GetAISContext(),GetDocContext());
			m_pPreviewRect->SetColor(0.0,1.0,1.0);
			m_pPreviewRect->Init(gp_Pnt(m_aPnt1[0],m_aPnt1[1],m_aPnt1[2]),pWorkPlane);
		}
		if(m_pPreviewRect)
		{
			m_pPreviewRect->Update(gp_Pnt(m_aPnt2[0],m_aPnt2[1],m_aPnt2[2]));
			m_pPreviewRect->Display(TRUE);

			Redraw();
		}
	}
	else if(m_nState == STA_BOX_INPUT_HEIGHT)
	{
		KC_SAFE_DELETE(m_pPreviewRect);

		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);
		if(m_pPreviewBox == NULL)
		{
			m_pPreviewBox = new kcPreviewBox(GetAISContext(),GetDocContext());
			m_pPreviewBox->SetColor(0.0,0.5,1.0);
			m_pPreviewBox->Init(gp_Pnt(m_aPnt1[0],m_aPnt1[1],m_aPnt1[2]),
				gp_Pnt(m_aPnt2[0],m_aPnt2[1],m_aPnt2[2]),pWorkPlane);
		}
		if(m_pPreviewBox)
		{
			//计算p3.
			kPoint3 vp,rp1,rp2;
			kVector3 vdir;
			double dist;
			kvCoordSystem cs;
			pWorkPlane->GetCoordSystem(cs);
			pWorkPlane->PixelToModel(mouseInput.m_x,mouseInput.m_y,vp,vdir);
			if(QxCurveLib::CalcLineLineExtrema(kPoint3(m_aPnt2[0],m_aPnt2[1],m_aPnt2[2]),cs.Z(),
				                         vp,vdir,rp1,rp2,dist))
			{
				vp = rp1;
				cs.WCSToUCS(vp);
				m_dHeight = vp[2];//记录高度

				if(m_dHeight > 0.0){
					KTRACE("\n rp1:%f,%f,%f,rp2:%f,%f,%f.dist:%f.",rp1[0],rp1[1],rp1[2],rp2[0],rp2[1],rp2[2],dist);
					m_pPreviewBox->Update(gp_Pnt(rp1[0],rp1[1],rp1[2]));
					m_pPreviewBox->Display(TRUE);
				}
			}

			Redraw();
		}
	}

	return KSTA_CONTINUE;
}

int	 kcmSolidBox::OnLButtonUp(kuiMouseInput& mouseInput)
{
	if(m_nState == STA_BOX_INPUT_HEIGHT)
	{
		if(BuildBox())
		{
			Done();
			return KSTA_DONE;
		}
	}
	return kiCommand::OnLButtonUp(mouseInput);
}

//
bool	kcmSolidBox::BuildBox()
{
	if(fabs(m_dHeight) < KC_MIN_DIST)
		return false;

	kcBasePlane *pWorkPlane = GetCurrentBasePlane();
	ASSERT(pWorkPlane);

	kvCoordSystem cs;
	pWorkPlane->GetCoordSystem(cs);

	kPoint3 lp1(m_aPnt1),lp2(m_aPnt2),lpo;
	double dx,dy,dz;
	cs.WCSToUCS(lp1);
	cs.WCSToUCS(lp2);
	lpo[0] = (lp1[0] < lp2[0]) ? lp1[0] : lp2[0];
	lpo[1] = (lp1[1] < lp2[1]) ? lp1[1] : lp2[1];
	lpo[2] = lp1[2];
	dx = fabs(lp1[0] - lp2[0]);
	dy = fabs(lp1[1] - lp2[1]);
	dz = m_dHeight;
	if(dz < 0.0)
		lpo[2] += m_dHeight;
	cs.UCSToWCS(lpo);
	
	gp_Ax2 axis(gp_Pnt(lpo[0],lpo[1],lpo[2]),
		gp_Dir(gp_Vec(cs.Z().x(),cs.Z().y(),cs.Z().z())),
		gp_Dir(gp_Vec(cs.X().x(),cs.X().y(),cs.X().z())));

	TopoDS_Solid aSolid;
	try{
		BRepPrimAPI_MakeBox mbox(axis,dx,dy,dz);
		aSolid = mbox.Solid();
		if(!mbox.IsDone()|| aSolid.IsNull())
			return false;

#ifdef _V_DEBUG
		gp_Ax3 ax(gp_Pnt(10,10,0),gp_Dir(0,0,1),gp_Dir(1,1,0)),axto;
		gp_Trsf trsf;
		trsf.SetTransformation(ax,axto);
		aSolid.Location(TopLoc_Location(trsf));
#endif
	}
	catch(Standard_Failure){
		return false;
	}
	
	kcSolidEntity *pEntity = new kcSolidEntity;
	if(pEntity)
	{
		pEntity->SetSolid(aSolid);
		kcModel *pModel = GetModel();
		pModel->AddEntity(pEntity);

		Redraw();
	}

	return true;
}