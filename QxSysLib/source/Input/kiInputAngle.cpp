#include "StdAfx.h"
#include <Aspect_TypeOfLine.hxx>
#include "kcBasePlane.h"
#include "kiCommand.h"
#include "kcSnapMngr.h"
#include "QxCurveLib.h"
#include "kvCoordSystem.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kcmUtils.h"
#include "kiParseString.h"
#include "kiInputAngle.h"

// 用于命令中的构造函数。
kiInputAngle::kiInputAngle(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	Reset();
}

// 用于InputTool中的构造函数。
kiInputAngle::kiInputAngle(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	Reset();
}

kiInputAngle::~kiInputAngle(void)
{
	Clear();
}

bool kiInputAngle::Init(kvCoordSystem *pCS,double *pAngle,bool bRadian)
{
	ASSERT(pCS && pAngle);
	_pLocalCS = pCS;
	_pAngle = pAngle;
	_bRadian = bRadian;
	return true;
}

void kiInputAngle::Reset()
{
	_pLocalCS = NULL;
	_pAngle = NULL;
	_bRadian = true;//默认弧度

	_pPreviewLine = NULL;
	_pPreviewLine2 = NULL;
	_pPreviewArc = NULL;
	_pPreviewText = NULL;
}

void kiInputAngle::Clear()
{
	KC_SAFE_DELETE(_pPreviewLine);
	KC_SAFE_DELETE(_pPreviewLine2);
	KC_SAFE_DELETE(_pPreviewArc);
	KC_SAFE_DELETE(_pPreviewText);
}

//
int kiInputAngle::OnBegin()
{
	ASSERT(_pLocalCS && _pAngle);
	Handle(AIS_InteractiveContext) aCtx = GetAISContext();
	kcDocContext *pDocCtx = GetDocContext();

	_pPreviewLine = new kcPreviewLine(aCtx,pDocCtx);
	_pPreviewLine->SetColor(1.0,0.0,0.0);

	_pPreviewLine2 = new kcPreviewLine(aCtx,pDocCtx);
	_pPreviewLine2->SetColor(0.0,1.0,0.0);
	_pPreviewLine2->SetType(Aspect_TOL_DOT);

	_pPreviewArc = new kcPreviewArc(aCtx,pDocCtx);
	_pPreviewArc->SetType(Aspect_TOL_DOT);
	_pPreviewArc->SetColor(0.0,1.0,0.0);

	_pPreviewText = new kcPreviewText(aCtx,pDocCtx);
	    
	return KSTA_CONTINUE;
}

int kiInputAngle::OnEnd()
{
	KC_SAFE_DELETE(_pPreviewLine);
	KC_SAFE_DELETE(_pPreviewLine2);
	KC_SAFE_DELETE(_pPreviewArc);
	KC_SAFE_DELETE(_pPreviewText);

	return KSTA_DONE;
}

// 获取提示字符串
void kiInputAngle::DoGetPromptText(std::string& str)
{
	char sbuf[128];
	if(_pAngle)
	{
		//只显示角度
		double ang = *_pAngle;
		if(_bRadian)
			ang = ang * 180 / K_PI;
		if(GetCurOptionSet())
			sprintf_s(sbuf,128,"%s<%f>",m_szPromptText.c_str(),ang);
		else
			sprintf_s(sbuf,128,"%s<%f>:",m_szPromptText.c_str(),ang);
	}
	else
	{
		if(GetCurOptionSet())
			sprintf_s(sbuf,128,"%s",m_szPromptText.c_str());
		else
			sprintf_s(sbuf,128,"%s:",m_szPromptText.c_str());
	}
	str = sbuf;
}

//// 当输入为空时的处理,可以采用默认值
//int		kiInputAngle::OnEditInputNull()
//{
//	SetInputState(KINPUT_DONE);
//	return KSTA_DONE;
//}

// 出入文本完成，通常命令可以结束了
int kiInputAngle::DoTextInputFinished(const char *pInputText)
{
	if(!pInputText || pInputText[0] == '\0'){//使用默认值
		SetInputState(KINPUT_DONE);
		return KSTA_DONE;
	}

	double angle;
	kiParseDouble pd(pInputText);
	if(!pd.Parse(angle))
		return KSTA_CONTINUE;

	*_pAngle = angle;
	if(_bRadian)
		*_pAngle = angle * K_PI / 180;//convert to radian
	SetInputState(KINPUT_DONE);

	End();
	DoNotify();

	return KSTA_DONE;
}

double kiInputAngle::CalcAngle(kuiMouseInput& mouseInput)
{
	double angle = 0.0;
	double curpnt[3];
	CalcInputPoint(mouseInput,curpnt);
	kVector3 basdir = _pLocalCS->X();
	kVector3 refdir = _pLocalCS->Z();
	kVector3 todir(curpnt[0] - _pLocalCS->Org().x(),
		curpnt[1] - _pLocalCS->Org().y(),
		curpnt[2] - _pLocalCS->Org().z());
	todir.normalize();

	angle = basdir.AngleWithRef(todir,refdir);

	return angle;
}

int kiInputAngle::OnLButtonUp(kuiMouseInput& mouseInput) 
{  
	*_pAngle = CalcAngle(mouseInput);
	if(!_bRadian)
		*_pAngle = *_pAngle * 180 / K_PI;// to angle

	Clear();
	SetInputState(KINPUT_DONE);
	DoNotify();

	return KSTA_DONE; 
}

int kiInputAngle::OnMouseMove(kuiMouseInput& mouseInput) 
{  
	kiInputTool::OnMouseMove(mouseInput);

	if(IsInputRunning()){
		double curpnt[3];

		CalcInputPoint(mouseInput,curpnt);
		kVector3 basdir = _pLocalCS->X();
		kVector3 refdir = _pLocalCS->Z();
		kVector3 todir(curpnt[0] - _pLocalCS->Org().x(),
			curpnt[1] - _pLocalCS->Org().y(),
			curpnt[2] - _pLocalCS->Org().z());
		double dist = todir.length();
		todir.normalize();

		double angle = basdir.AngleWithRef(todir,refdir);

		//record result
		*_pAngle = angle;
		if(!_bRadian)
			*_pAngle = angle * 180 / K_PI;

		gp_Pnt org(_pLocalCS->Org().x(),_pLocalCS->Org().y(),_pLocalCS->Org().z());
		if(_pPreviewLine){
			gp_Pnt pnt(org.X() + dist * basdir[0],org.Y() + dist * basdir[1],
				org.Z() + dist * basdir[2]);
			_pPreviewLine->Update(org,pnt);
			_pPreviewLine->Display(TRUE);
		}
		if(_pPreviewLine2){
			gp_Pnt pnt(curpnt[0],curpnt[1],curpnt[2]);
			_pPreviewLine2->Update(org,pnt);
			_pPreviewLine2->Display(TRUE);
		}

		if(_pPreviewArc){
			if(!_pPreviewArc->HasInited()){
				Handle(Geom_Circle) aCirc = ulb_MakeCircle(*_pLocalCS,dist);
				if(!aCirc.IsNull()){
					_pPreviewArc->Init(aCirc);
				}else{
					KC_SAFE_DELETE(_pPreviewArc);
				}
			}
			if(_pPreviewArc){
				_pPreviewArc->Update(0.0,angle);
				_pPreviewArc->Display(TRUE);
			}
		}
	}
	return KSTA_CONTINUE; 
}

int kiInputAngle::OnRButtonUp(kuiMouseInput& mouseInput) 
{  
	Clear();
	SetInputState(KINPUT_CANCEL);
	return KSTA_CANCEL; 
}
