#include "StdAfx.h"
#include "kcBasePlane.h"
#include "kiCommand.h"
#include "kcSnapMngr.h"
#include "QxCurveLib.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kiParseString.h"
#include "kiInputLength.h"

// 用于命令中的构造函数。
kiInputLength::kiInputLength(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	Reset();

	dDefaultLength_ = 0.0;
	dLength_ = 0.0;
	pPreviewLine_ = NULL;
	pPreviewText_ = NULL;
}

// 用于InputTool中的构造函数。
kiInputLength::kiInputLength(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	Reset();

	dDefaultLength_ = 0.0;
	dLength_ = 0.0;
	pPreviewLine_ = NULL;
	pPreviewText_ = NULL;
}

kiInputLength::~kiInputLength(void)
{
	KC_SAFE_DELETE(pPreviewLine_);
	KC_SAFE_DELETE(pPreviewText_);
}

void	kiInputLength::Reset()
{
	_endPoint.set(0.0,0.0,0.0);
	aBasePoint_[0] = aBasePoint_[1] = aBasePoint_[2] = 0.0;
	aBaseDir_[0] = aBaseDir_[1] = 0.0;aBaseDir_[2] = 1.0;
	bHasBaseDir_ = false;
	bRealtimeUpdate_ = false;
}

void	kiInputLength::Set(double basePnt[3],bool bRealtimeUpdate)
{
	aBasePoint_[0] = basePnt[0];aBasePoint_[1] = basePnt[1];aBasePoint_[2] = basePnt[2];
	bHasBaseDir_ = false;
	bRealtimeUpdate_ = bRealtimeUpdate;
}

void	kiInputLength::Set(const kPoint3& basePnt,bool bRealtimeUpdate)
{
	aBasePoint_[0] = basePnt[0];aBasePoint_[1] = basePnt[1];aBasePoint_[2] = basePnt[2];
	bHasBaseDir_ = false;
	bRealtimeUpdate_ = bRealtimeUpdate;
}


void	kiInputLength::Set(double basePnt[3],double baseDir[3],bool bRealtimeUpdate)
{
	aBasePoint_[0] = basePnt[0];aBasePoint_[1] = basePnt[1];aBasePoint_[2] = basePnt[2];
	aBaseDir_[0] = baseDir[0];aBaseDir_[1] = baseDir[1];aBaseDir_[2] = baseDir[2];
	bHasBaseDir_ = true;
	bRealtimeUpdate_ = bRealtimeUpdate;
}

void	kiInputLength::Set(const kPoint3& aBasePnt,const kVector3& aBaseDir,bool bRealtimeUpdate)
{
	aBasePoint_[0] = aBasePnt[0];aBasePoint_[1] = aBasePnt[1];aBasePoint_[2] = aBasePnt[2];
	aBaseDir_[0] = aBaseDir[0];aBaseDir_[1] = aBaseDir[1];aBaseDir_[2] = aBaseDir[2];

	bHasBaseDir_ = true;
	bRealtimeUpdate_ = bRealtimeUpdate;
}

// 对baseDir进行反向
void kiInputLength::ReverseBaseDir()
{
	aBaseDir_[0] = -aBaseDir_[0];
	aBaseDir_[1] = -aBaseDir_[1];
	aBaseDir_[2] = -aBaseDir_[2];
}

//
int kiInputLength::OnBegin()
{
	Handle(AIS_InteractiveContext) aCtx = GetAISContext();
	kcDocContext *pDocCtx = GetDocContext();

	pPreviewLine_ = new kcPreviewLine(aCtx,pDocCtx);
	pPreviewLine_->SetColor(1.0,0.0,0.0);

	pPreviewText_ = new kcPreviewText(aCtx,pDocCtx);

	return KSTA_CONTINUE;
}

int		kiInputLength::OnEnd()
{
	KC_SAFE_DELETE(pPreviewLine_);
	KC_SAFE_DELETE(pPreviewText_);
	Reset();

	return KSTA_DONE;
}

// 获取提示字符串
void kiInputLength::DoGetPromptText(std::string& str)
{
	char sbuf[32];
	sprintf_s(sbuf,32,"%s<%f>:",this->m_szPromptText.c_str(),dLength_);
	str = sbuf;
}

// 出入文本完成，通常命令可以结束了
int kiInputLength::DoTextInputFinished(const char *pInputText)
{
	if(!pInputText || pInputText[0] == '\0'){//为空，使用默认值
		dLength_ = dDefaultLength_;
		SetInputState(KINPUT_DONE);
		End();
		DoNotify();

		return KSTA_DONE;
	}

	kiParseDouble pd(pInputText);
	if(!pd.Parse(dLength_))
		return KSTA_CONTINUE;

	SetInputState(KINPUT_DONE);
	dDefaultLength_ = dLength_;//记录上次输入值.

	End();
	DoNotify();

	return KSTA_DONE;
}

//鼠标消息
int kiInputLength::OnLButtonUp(kuiMouseInput& mouseInput)
{
	kPoint3 org(aBasePoint_);
	CalcCurrPoint(mouseInput,_endPoint);
	dLength_ = _endPoint.distance(org);
	kVector3 bdir(aBaseDir_),v = _endPoint - org;
	if(v * bdir < 0.0)
		dLength_ = -dLength_;

	SetInputState(KINPUT_DONE);
	dDefaultLength_ = dLength_;

	DoNotify();

	return KSTA_DONE;
}

int kiInputLength::OnMouseMove(kuiMouseInput& mouseInput) 
{
	// 计算当前点
	if(IsInputRunning())
	{
		kPoint3 kpnt;
		CalcCurrPoint(mouseInput,kpnt);
		if(bRealtimeUpdate_)
		{
			kPoint3 org(aBasePoint_);
			_endPoint = kpnt;
			dLength_ = kpnt.distance(org);
			kVector3 bdir(aBaseDir_),v = _endPoint - org;
			if(v * bdir < 0.0)
				dLength_ = -dLength_;

		}
		//
		if(pPreviewLine_)
		{
			pPreviewLine_->Update(gp_Pnt(aBasePoint_[0],aBasePoint_[1],aBasePoint_[2]),
				gp_Pnt(kpnt[0],kpnt[1],kpnt[2]));
			pPreviewLine_->Display(TRUE);
		}
		if(pPreviewText_)
		{
			char buf[32];
			kPoint3 org(aBasePoint_);
			double dist = kpnt.distance(org);
			sprintf_s(buf,32,"%f",dist);
			pPreviewText_->Text(buf,mouseInput.m_x,mouseInput.m_y,0);
		}
	}
	
	return KSTA_CONTINUE; 
}

// 右键认为是取消获取输入.
//
int kiInputLength::OnRButtonUp(kuiMouseInput& mouseInput)
{
	KC_SAFE_DELETE(pPreviewLine_);
	KC_SAFE_DELETE(pPreviewText_);
	SetInputState(KINPUT_CANCEL);
	return KSTA_CANCEL;
}

void kiInputLength::CalcCurrPoint(kuiMouseInput& mouseInput,kPoint3& kpnt)
{
	double point[3];
	if(bHasBaseDir_)
	{
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);
		kPoint3 vp,rp1,rp2;
		kVector3 vdir;
		kPoint3 org(aBasePoint_);
		kVector3 dir(aBaseDir_);
		double dist;
		pWorkPlane->PixelToModel(mouseInput.m_x,mouseInput.m_y,vp,vdir);
		if(!QxCurveLib::CalcLineLineExtrema(vp,vdir,org,dir,rp1,rp2,dist))
			return;

		kpnt = rp2;
	}
	else
	{
		CalcInputPoint(mouseInput,point);
		kpnt.set(point);
	}
}