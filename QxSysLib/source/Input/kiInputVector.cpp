#include "StdAfx.h"
#include "kiInputPoint.h"
#include "kcPreviewObj.h"
#include "kiCommand.h"
#include "kcBasePlane.h"
#include "kvCoordSystem.h"
#include "QxCurveLib.h"
#include "kcDocContext.h"
#include "kiInputVector.h"

#define KI_STA_VEC_ORG		1
#define KI_STA_VEC_DIR		2

kiInputVector::kiInputVector(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	Reset();
}

kiInputVector::kiInputVector(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	Reset();
}

kiInputVector::~kiInputVector(void)
{
	Destroy();
}

void	kiInputVector::Reset()
{
	m_eType = eInputZVector;
	m_nVecType = m_eType;
	m_bFixedDir = false;
	m_bSetOrgPoint = false;
	m_strPrompt1 = "输入第一个点";
	m_strPrompt2 = "输入第二个点";

	m_pInputOrgPoint = NULL;
	m_pInputVecPoint = NULL;
	m_pPreviewLine = NULL;

	m_pOptionType = NULL;
	m_pOptionLength = NULL;
	m_dLength = 0;//

	SetInputState(KINPUT_RUNNING);

	m_nState = KI_STA_VEC_ORG;//获取起点
}

void	kiInputVector::Destroy()
{
	m_dirOptionSet.Clear();
	KC_SAFE_DELETE(m_pInputOrgPoint);
	KC_SAFE_DELETE(m_pInputVecPoint);
	KC_SAFE_DELETE(m_pPreviewLine);

	Reset();
}

//设置类型
void	kiInputVector::SetType(eInputVectorType eType)
{
	m_eType = eType;
	m_nVecType = eType;
}

void	kiInputVector::FixVectorDir(eInputVectorType eType)//固定在一个方向上
{
	m_bFixedDir = true;
	m_eType = eType;
}

void	kiInputVector::SetOrgPoint(const kPoint3& org)//设定了起始点
{
	m_bSetOrgPoint = true;
	m_point = org;
}

void	kiInputVector::SetPrompt(const char *pszPrompt1,const char *pszPrompt2)//设置两个提示字符串
{
	ASSERT(pszPrompt1 && pszPrompt2);
	m_strPrompt1 = pszPrompt1;
	m_strPrompt2 = pszPrompt2;
}

// 开始命令的执行
//
int		kiInputVector::Begin()
{
	if(!CreateInputTool())
		return KSTA_ERROR;

	if(m_bSetOrgPoint)
	{
		m_nState = KI_STA_VEC_DIR;
		m_pInputVecPoint->Begin();
	}
	else
	{
		m_nState = KI_STA_VEC_ORG;//获取起点
		m_pInputOrgPoint->Begin();
	}

	return KSTA_CONTINUE;
}

// 创建输入工具
//
bool	kiInputVector::CreateInputTool()
{
	if(m_pInputOrgPoint)//已经创建了
		return true;

	m_pInputOrgPoint = new kiInputPoint(this,m_strPrompt1.c_str(),GetCurOptionSet());
	m_pInputOrgPoint->Init(&m_aOrg);

	// 第二个选项
	if(!m_bFixedDir)
	{
		m_nVecType = m_eType;
		m_pOptionType = new kiOptionEnum("类型",'T',m_nVecType);
		m_pOptionType->AddEnumItem("平面",'W');
		m_pOptionType->AddEnumItem("X向",'x');
		m_pOptionType->AddEnumItem("Y向",'y');
		m_pOptionType->AddEnumItem("Z向",'z');
		m_pOptionType->AddEnumItem("空间",'s');
		m_dirOptionSet.AddOption(m_pOptionType);
	}
	
	m_pOptionLength = new kiOptionDouble("长度",'L',m_dLength);
	m_dirOptionSet.AddOption(m_pOptionLength);
	m_pInputVecPoint = new kiInputPoint(this,m_strPrompt2.c_str(),&m_dirOptionSet);
	m_dirOptionSet.SetOptionCallBack(this);
	
	m_pInputVecPoint->Init(&m_aDirPnt);
	m_pInputVecPoint->SetRealtimeUpdate(true);
	m_pInputVecPoint->SetShowText(false);

	return true;
}

// 结束命令的执行,nCode是输入的结果.
//
int kiInputVector::End()
{
	m_dirOptionSet.Clear();

	return KSTA_DONE;
}

// 子tool的回调函数,当子tool调用结束时,调用.
int		kiInputVector::OnSubInputFinished(kiInputTool *pTool)
{
	if(pTool == m_pInputOrgPoint)
	{
		//记录输入点.
		m_point = m_aOrg;
		m_pInputOrgPoint->End();
		
		//启动下一个工具
		m_nState = KI_STA_VEC_DIR;
		m_pInputVecPoint->Begin();
	}
	else if(pTool == m_pInputVecPoint)
	{
		m_pInputVecPoint->End();
		//输入成功。
		SetInputState(KINPUT_DONE);
		//
		if(m_pPreviewLine)
			m_pPreviewLine->Display(FALSE);

		//
		if(m_pCommand)
		{
			m_pCommand->OnInputFinished(this);
		}
	}
	return KSTA_CONTINUE;
}

// 当选项改变后的调用函数
void	kiInputVector::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if(pOptionItem == NULL)
		return;

	if(pOptionItem == m_pOptionType)
	{
		m_eType = (eInputVectorType)m_nVecType;
	}
	else if(pOptionItem == m_pOptionLength)
	{
		if(fabs(m_dLength) > KDBL_MIN){
			//获取拉伸方向
			kVector3 v;
			if(m_eType == eInputXVector){
				v.set(1.0,0.0,0.0);
			}else if(m_eType == eInputYVector){
				v.set(0.0,1.0,0.0);
			}else{
				v.set(0.0,0.0,1.0);
			}
			m_vec = v * m_dLength;

			m_pInputVecPoint->SetInputState(KINPUT_DONE);//结束
			SetInputState(KINPUT_DONE);

			if(m_pPreviewLine)
				m_pPreviewLine->Display(FALSE);

			if(m_pCommand){
				m_pCommand->OnInputFinished(this);
			}
		}
	}
}

//鼠标消息
int kiInputVector::OnLButtonUp(kuiMouseInput& mouseInput)
{
	if(!m_pInputOrgPoint || !m_pInputVecPoint)
		return KSTA_CONTINUE;

	if(m_nState == KI_STA_VEC_ORG)
	{
		m_pInputOrgPoint->OnLButtonUp(mouseInput);
	}
	else
	{
		m_pInputVecPoint->OnLButtonUp(mouseInput);
	}
		
	return KSTA_CONTINUE;
}

int kiInputVector::OnMouseMove(kuiMouseInput& mouseInput)
{
	if(m_nState == KI_STA_VEC_ORG)
	{
		m_pInputOrgPoint->OnMouseMove(mouseInput);
	}
	else if(m_nState == KI_STA_VEC_DIR)
	{
		m_pInputVecPoint->OnMouseMove(mouseInput);

		//计算向量
		if(!CalcVector(mouseInput))
			return KSTA_CONTINUE;

		//创建预览线
		if(m_pPreviewLine == NULL)
		{
			m_pPreviewLine = new kcPreviewLine(GetAISContext(),GetDocContext());
			m_pPreviewLine->SetColor(1.0,0.0,0.0);
		}
		if(m_pPreviewLine)
		{
			kPoint3 pt = m_point + m_vec;
			m_pPreviewLine->Update(gp_Pnt(m_point[0],m_point[1],m_point[2]),gp_Pnt(pt[0],pt[1],pt[2]));
			m_pPreviewLine->Display(TRUE);
		}
	}
	return kiInputTool::OnMouseMove(mouseInput);
}

//计算向量
bool kiInputVector::CalcVector(kuiMouseInput& mouseInput)
{
	kvCoordSystem cs;
	kcBasePlane *pBasePlane = GetCurrentBasePlane();
	ASSERT(pBasePlane);
	pBasePlane->GetCoordSystem(cs);

	// 计算目标点
	kPoint3 pt;
	if(m_eType == eInputZVector)
	{
		kPoint3 vp,rp1,rp2;
		kVector3 vdir;	
		double dist;
		pBasePlane->PixelToModel(mouseInput.m_x,mouseInput.m_y,vp,vdir);
		if(!QxCurveLib::CalcLineLineExtrema(m_point,cs.Z(),vp,vdir,rp1,rp2,dist))
			return false;

		pt = rp1;//要获取的点。
	}
	else if(m_eType == eInputXVector || m_eType == eInputYVector)
	{
		kPoint3 rp;
		bool bdone = false;
		if(m_eType == eInputXVector)
			bdone = QxCurveLib::ProjectPointToLine(m_aOrg,cs.X(),m_aDirPnt,rp);
		else
			bdone = QxCurveLib::ProjectPointToLine(m_aOrg,cs.Y(),m_aDirPnt,rp);
		if(!bdone)
			return false;

		pt = rp;
	}
	else
	{
		pt = m_aDirPnt;
	}
	m_vec = pt - m_point;

	return true;
}

bool kiInputVector::GetPoint(kPoint3& point) const
{
	if(m_nState == KI_STA_VEC_ORG)
		return false;
	point = m_point;
	return true;
}

bool kiInputVector::GetVector(kVector3& vec) const
{
	if(m_nState == KI_STA_VEC_ORG)
		return false;
	vec = m_vec;
	return true;
}
