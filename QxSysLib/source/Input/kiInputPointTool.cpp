#include "StdAfx.h"
#include "kcBasePlane.h"
#include "kiCommand.h"
#include "kcSnapMngr.h"
#include "QxSurfLib.h"
#include "kcPreviewText.h"
#include "kvCoordSystem.h"
#include "kcDocContext.h"
#include "kiParseString.h"
#include "kiInputPointTool.h"

kiInputPointTool::kiInputPointTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	_pPoint = NULL;
	_bSnapPoint = false;
	_bSpacePoint = false;//默认工作平面上点。
	_bRealtimeUpdate = false;
}

kiInputPointTool::kiInputPointTool(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	_pPoint = NULL;
	_bSnapPoint = false;
	_bSpacePoint = false;//默认工作平面上点。
	_bRealtimeUpdate = false;
}

kiInputPointTool::~kiInputPointTool(void)
{
}

//初始化
BOOL  kiInputPointTool::Initialize(kPoint3 *pPoint,bool bSpacePoint)
{
	_pPoint = pPoint;
	_bSpacePoint = bSpacePoint;
	return TRUE;
}

// 是否点的值实时更新
void  kiInputPointTool::SetRealtimeUpdate(bool bEnable)
{
	_bRealtimeUpdate = bEnable;
}

int  kiInputPointTool::OnBegin()
{
	ASSERT(_pPoint);
	_bSnapPoint = false;

	return KSTA_CONTINUE;
}

int  kiInputPointTool::OnEnd()
{
	return KSTA_DONE;
}

//鼠标消息
int  kiInputPointTool::OnLButtonUp(kuiMouseInput& mouseInput)
{
	CalcPoint(mouseInput);

	SetInputState(KINPUT_DONE);
	DoNotify();

	return KINPUT_DONE;
}

int  kiInputPointTool::OnMouseMove(kuiMouseInput& mouseInput)
{
	if(IsInputRunning() && _bRealtimeUpdate){
		//计算点
		CalcPoint(mouseInput);

		//实时更新点
		if(m_pCommand){
			m_pCommand->OnInputChanged(this);
		}
	}
	return kiInputTool::OnMouseMove(mouseInput);
}

// 右键认为是取消获取输入.
//
int  kiInputPointTool::OnRButtonUp(kuiMouseInput& mouseInput)
{
	SetInputState(KINPUT_CANCEL);
	return KSTA_CANCEL;
}

// 获取提示字符串
void  kiInputPointTool::DoGetPromptText(std::string& str)
{
	char sbuf[128];
	if(GetCurOptionSet() == NULL)
		sprintf_s(sbuf,127,"%s<%f,%f,%f>",m_szPromptText.c_str(),_pPoint->x(),_pPoint->y(),_pPoint->z());
	else
		sprintf_s(sbuf,127,"%s<%f,%f,%f> ",m_szPromptText.c_str(),_pPoint->x(),_pPoint->y(),_pPoint->z());
	str = sbuf;
}

// 当InputEdit输入完成后，调用该函数
//
int  kiInputPointTool::OnEditInputFinished(const char *pInputText)
{
	//输入不能为空
	if(!pInputText)
		return KSTA_CONTINUE;

	kPoint3 pnt;
	kiParseStringToPoint3 psp(pInputText);
	if(psp.Parse(pnt)){
		*_pPoint = pnt;

		SetInputState(KINPUT_DONE);
		End();

		DoNotify();

		return KSTA_DONE;
	}

	return KSTA_CONTINUE;
}

//更新点的值
void  kiInputPointTool::CalcPoint(kuiMouseInput& mouseInput)
{
	kcBasePlane *pBasePlane = GetCurrentBasePlane();
	ASSERT(pBasePlane);

	_bSnapPoint = false;
	//是否有自动捕获点
	if(m_pSnapMgr && m_pSnapMgr->HasSnapPoint()){
		kiSnapPoint sp = m_pSnapMgr->GetSnapPoint();
		_point = sp._point;
		_bSnapPoint = true;
	}else{
		//否则工作平面上的点
		pBasePlane->PixelToWorkPlane(mouseInput.m_x,mouseInput.m_y,_point);
	}

	if(!_bSpacePoint){
		kvCoordSystem cs;
		pBasePlane->GetCoordSystem(cs);
		//如果不在平面上,投影到平面上
		if(!QxSurfLib::IsPointOnPlane(_point,cs.Org(),cs.Z())){
			kPoint3 pp;
			QxSurfLib::ProjectPointToPlane(_point,cs.Org(),cs.Z(),pp);
			_point = pp;

			if(_bSnapPoint)
				_bSnapPoint = false;
		}
	}
	*_pPoint = _point;
}

