#include "StdAfx.h"
#include "kcBasePlane.h"
#include "kiCommand.h"
#include "kcSnapMngr.h"
#include "QxSurfLib.h"
#include "kcPreviewText.h"
#include "kvCoordSystem.h"
#include "kiParseString.h"
#include "kiInputPoint.h"

kiInputPoint::kiInputPoint(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	_pPoint = NULL;
	_bSnapPoint = false;
	_bSpacePoint = false;//默认工作平面上点。
	_bRealtimeUpdate = false;
	_bShowText = true;
	bEnableSubShape_ = false;
	_pPreviewText = NULL;
}

kiInputPoint::kiInputPoint(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet)
:kiInputTool(pParent,pszPrompt,pOptionSet)
{
	_pPoint = NULL;
	_bSnapPoint = false;
	_bSpacePoint = false;//默认工作平面上点。
	_bRealtimeUpdate = false;
	_bShowText = true;
	_pPreviewText = NULL;
}

kiInputPoint::~kiInputPoint(void)
{
}

//初始化
BOOL	kiInputPoint::Init(kPoint3 *pPoint,bool bSpacePoint)
{
	_pPoint = pPoint;
	_bSpacePoint = bSpacePoint;
	return TRUE;
}

// 是否点的值实时更新
void	kiInputPoint::SetRealtimeUpdate(bool bEnable)
{
	_bRealtimeUpdate = bEnable;
}

void	kiInputPoint::SetShowText(bool bEnable)
{
	_bShowText = bEnable;
}

// 是否允许拾取对象中的vertex
void kiInputPoint::EnableSubShape(bool bEnable)
{
	bEnableSubShape_ = bEnable;
}

int kiInputPoint::OnBegin()
{
	ASSERT(_pPoint);
	_bSnapPoint = false;
	//
	Handle(AIS_InteractiveContext) aAISCtx = GetAISContext();
	
	if(_bShowText)
		_pPreviewText = new kcPreviewText(aAISCtx,GetDocContext());
	else
		_pPreviewText = NULL;

	if(bEnableSubShape_ && !aAISCtx.IsNull()){//启动点拾取
		aAISCtx->Deactivate();
		aAISCtx->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
	}

	return KSTA_CONTINUE;
}

int kiInputPoint::OnEnd()
{
	KC_SAFE_DELETE(_pPreviewText);

	Handle(AIS_InteractiveContext) aAISCtx = GetAISContext();
	if(bEnableSubShape_ && !aAISCtx.IsNull()){
		aAISCtx->Deactivate();
		aAISCtx->Activate(0);
	}

	return KSTA_DONE;
}

//鼠标消息
int		kiInputPoint::OnLButtonUp(kuiMouseInput& mouseInput)
{
	CalcPoint(mouseInput);

	KC_SAFE_DELETE(_pPreviewText);

	SetInputState(KINPUT_DONE);
	DoNotify();

	return KINPUT_DONE;
}

int		kiInputPoint::OnMouseMove(kuiMouseInput& mouseInput)
{
	Handle(AIS_InteractiveContext) aAISCtx = GetAISContext();
	if(IsInputRunning() && !aAISCtx.IsNull()){
		Handle(V3d_View) aView = GetCurrV3dView();
		if(!aView.IsNull()){
			aAISCtx->MoveTo(mouseInput.m_x,mouseInput.m_y,aView,Standard_True);
		}

		if(_bRealtimeUpdate){
			//计算点
			CalcPoint(mouseInput);

			if(_pPreviewText && _bShowText)
			{
				char sbuf[128];
				sprintf_s(sbuf,128,"%.3f,%.3f,%.3f",_pPoint->x(),_pPoint->y(),_pPoint->z());
				_pPreviewText->Text(sbuf,mouseInput.m_x,mouseInput.m_y,0.5);
			}
		}
	}
	return kiInputTool::OnMouseMove(mouseInput);
}

// 右键认为是取消获取输入.
//
int  kiInputPoint::OnRButtonUp(kuiMouseInput& mouseInput)
{
	SetInputState(KINPUT_CANCEL);
	return KSTA_CANCEL;
}

// 获取提示字符串
void  kiInputPoint::DoGetPromptText(std::string& str)
{
	char sbuf[128];
	if(GetCurOptionSet() == NULL)
		sprintf_s(sbuf,128,"%s<%f,%f,%f>:",m_szPromptText.c_str(),_pPoint->x(),_pPoint->y(),_pPoint->z());
	else
		sprintf_s(sbuf,128,"%s<%f,%f,%f> ",m_szPromptText.c_str(),_pPoint->x(),_pPoint->y(),_pPoint->z());
	str = sbuf;
}

// 出入文本完成，通常命令可以结束了
int		kiInputPoint::DoTextInputFinished(const char *pInputText)
{
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

// 更加点的值
void	kiInputPoint::CalcPoint(kuiMouseInput& mouseInput)
{
	bool bDone = false;
	Handle(AIS_InteractiveContext) aAISCtx = GetAISContext();
	if(bEnableSubShape_ && !aAISCtx.IsNull()){
		//aAISCtx->Select(Standard_False);
		aAISCtx->SelectDetected();

		aAISCtx->InitSelected();
		if(aAISCtx->MoreSelected()){
			TopoDS_Shape aS = aAISCtx->SelectedShape();
			if(!aS.IsNull()){
				gp_Pnt aPnt = BRep_Tool::Pnt(TopoDS::Vertex(aS));
				_point.set(aPnt.X(),aPnt.Y(),aPnt.Z());
				bDone = true;
			}
		}
	}

	if(!bDone){
		kcBasePlane *pWorkPlane = GetCurrentBasePlane();
		ASSERT(pWorkPlane);

		_bSnapPoint = false;
		//是否有自动捕获点
		if(m_pSnapMgr && m_pSnapMgr->HasSnapPoint()){
			kiSnapPoint sp = m_pSnapMgr->GetSnapPoint();
			_point = sp._point;
			_bSnapPoint = true;
		}else{
			//否则工作平面上的点
			pWorkPlane->PixelToWorkPlane(mouseInput.m_x,mouseInput.m_y,_point);
		}
		if(!_bSpacePoint){
			kvCoordSystem cs;
			pWorkPlane->GetCoordSystem(cs);
			//如果不在平面上,投影到平面上
			if(!QxSurfLib::IsPointOnPlane(_point,cs.Org(),cs.Z())){
				kPoint3 pp;
				QxSurfLib::ProjectPointToPlane(_point,cs.Org(),cs.Z(),pp);
				_point = pp;

				if(_bSnapPoint)
					_bSnapPoint = false;
			}
		}
	}
	*_pPoint = _point;

}

