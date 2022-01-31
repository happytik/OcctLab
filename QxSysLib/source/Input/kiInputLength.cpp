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

	_dDefaultLength = 0.0;
	_dLength = 0.0;
	_pPreviewLine = NULL;
	_pPreviewText = NULL;
}

// 用于InputTool中的构造函数。
kiInputLength::kiInputLength(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	Reset();

	_dDefaultLength = 0.0;
	_dLength = 0.0;
	_pPreviewLine = NULL;
	_pPreviewText = NULL;
}

kiInputLength::~kiInputLength(void)
{
	KC_SAFE_DELETE(_pPreviewLine);
	KC_SAFE_DELETE(_pPreviewText);
}

void	kiInputLength::Reset()
{
	_endPoint.set(0.0,0.0,0.0);
	_aBasePoint[0] = _aBasePoint[1] = _aBasePoint[2] = 0.0;
	_aBaseDir[0] = _aBaseDir[1] = 0.0;_aBaseDir[2] = 1.0;
	_bBaseDir = false;
	_bRealtimeUpdate = false;
}

void	kiInputLength::Set(double basePnt[3],bool bRealtimeUpdate)
{
	_aBasePoint[0] = basePnt[0];_aBasePoint[1] = basePnt[1];_aBasePoint[2] = basePnt[2];
	_bBaseDir = false;
	_bRealtimeUpdate = bRealtimeUpdate;
}

void	kiInputLength::Set(const kPoint3& basePnt,bool bRealtimeUpdate)
{
	_aBasePoint[0] = basePnt[0];_aBasePoint[1] = basePnt[1];_aBasePoint[2] = basePnt[2];
	_bBaseDir = false;
	_bRealtimeUpdate = bRealtimeUpdate;
}


void	kiInputLength::Set(double basePnt[3],double baseDir[3],bool bRealtimeUpdate)
{
	_aBasePoint[0] = basePnt[0];_aBasePoint[1] = basePnt[1];_aBasePoint[2] = basePnt[2];
	_aBaseDir[0] = baseDir[0];_aBaseDir[1] = baseDir[1];_aBaseDir[2] = baseDir[2];
	_bBaseDir = true;
	_bRealtimeUpdate = bRealtimeUpdate;
}

void	kiInputLength::Set(const kPoint3& aBasePnt,const kVector3& aBaseDir,bool bRealtimeUpdate)
{
	_aBasePoint[0] = aBasePnt[0];_aBasePoint[1] = aBasePnt[1];_aBasePoint[2] = aBasePnt[2];
	_aBaseDir[0] = aBaseDir[0];_aBaseDir[1] = aBaseDir[1];_aBaseDir[2] = aBaseDir[2];

	_bBaseDir = true;
	_bRealtimeUpdate = bRealtimeUpdate;
}

// 对baseDir进行反向
void kiInputLength::ReverseBaseDir()
{
	_aBaseDir[0] = -_aBaseDir[0];
	_aBaseDir[1] = -_aBaseDir[1];
	_aBaseDir[2] = -_aBaseDir[2];
}

//
int kiInputLength::OnBegin()
{
	Handle(AIS_InteractiveContext) aCtx = GetAISContext();
	kcDocContext *pDocCtx = GetDocContext();

	_pPreviewLine = new kcPreviewLine(aCtx,pDocCtx);
	_pPreviewLine->SetColor(1.0,0.0,0.0);

	_pPreviewText = new kcPreviewText(aCtx,pDocCtx);

	return KSTA_CONTINUE;
}

int		kiInputLength::OnEnd()
{
	KC_SAFE_DELETE(_pPreviewLine);
	KC_SAFE_DELETE(_pPreviewText);
	Reset();

	return KSTA_DONE;
}

// 获取提示字符串
void kiInputLength::DoGetPromptText(std::string& str)
{
	char sbuf[32];
	sprintf_s(sbuf,32,"%s<%f>:",this->m_szPromptText.c_str(),_dLength);
	str = sbuf;
}

// 出入文本完成，通常命令可以结束了
int kiInputLength::DoTextInputFinished(const char *pInputText)
{
	if(!pInputText || pInputText[0] == '\0'){//为空，使用默认值
		_dLength = _dDefaultLength;
		SetInputState(KINPUT_DONE);
		End();
		DoNotify();

		return KSTA_DONE;
	}

	kiParseDouble pd(pInputText);
	if(!pd.Parse(_dLength))
		return KSTA_CONTINUE;

	SetInputState(KINPUT_DONE);
	_dDefaultLength = _dLength;//记录上次输入值.

	End();
	DoNotify();

	return KSTA_DONE;
}

//鼠标消息
int kiInputLength::OnLButtonUp(kuiMouseInput& mouseInput)
{
	kPoint3 org(_aBasePoint);
	CalcCurrPoint(mouseInput,_endPoint);
	_dLength = _endPoint.distance(org);
	kVector3 bdir(_bBaseDir),v = _endPoint - org;
	if(v * bdir < 0.0)
		_dLength = -_dLength;

	SetInputState(KINPUT_DONE);
	_dDefaultLength = _dLength;

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
		if(_bRealtimeUpdate)
		{
			kPoint3 org(_aBasePoint);
			_endPoint = kpnt;
			_dLength = kpnt.distance(org);
			kVector3 bdir(_bBaseDir),v = _endPoint - org;
			if(v * bdir < 0.0)
				_dLength = -_dLength;

		}
		//
		if(_pPreviewLine)
		{
			_pPreviewLine->Update(gp_Pnt(_aBasePoint[0],_aBasePoint[1],_aBasePoint[2]),
				gp_Pnt(kpnt[0],kpnt[1],kpnt[2]));
			_pPreviewLine->Display(TRUE);
		}
		if(_pPreviewText)
		{
			char buf[32];
			kPoint3 org(_aBasePoint);
			double dist = kpnt.distance(org);
			sprintf_s(buf,32,"%f",dist);
			_pPreviewText->Text(buf,mouseInput.m_x,mouseInput.m_y,0);
		}
	}
	
	return KSTA_CONTINUE; 
}

// 右键认为是取消获取输入.
//
int kiInputLength::OnRButtonUp(kuiMouseInput& mouseInput)
{
	KC_SAFE_DELETE(_pPreviewLine);
	KC_SAFE_DELETE(_pPreviewText);
	SetInputState(KINPUT_CANCEL);
	return KSTA_CANCEL;
}

void kiInputLength::CalcCurrPoint(kuiMouseInput& mouseInput,kPoint3& kpnt)
{
	double point[3];
	if(_bBaseDir)
	{
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);
		kPoint3 vp,rp1,rp2;
		kVector3 vdir;
		kPoint3 org(_aBasePoint);
		kVector3 dir(_aBaseDir);
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