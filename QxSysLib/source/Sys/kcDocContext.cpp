//
#include "stdafx.h"
#include "kcModel.h"
#include "kiCommandMgr.h"
#include "kcSnapMngr.h"
#include "kuiInterface.h"
#include "kiInputEdit.h"
#include "kcDocContext.h"

kcDocContext::kcDocContext(CDocument *pDoc,const Handle(AIS_InteractiveContext) &aCtx,kcModel *pModel)
	:pDocument_(pDoc),aAISCtx_(aCtx),pModel_(pModel)
{
	hMainWnd_ = NULL;
	pCmdMgr_ = NULL;
	pSnapMgr_ = NULL;
	pInputEdit_ = NULL;
	pModelTreeItf_ = NULL;

	pActiveView_ = NULL;
	pCmdTimerMgr_ = NULL;
}

kcDocContext::~kcDocContext()
{
}

const Handle(AIS_InteractiveContext)& kcDocContext::GetAISContext() const
{
	return aAISCtx_;
}

kcModel* kcDocContext::GetModel() const
{
	return pModel_;
}

HWND  kcDocContext::GetMainWnd() const
{
	return hMainWnd_;
}

void  kcDocContext::SetMainWnd(HWND hWnd)
{
	hMainWnd_ = hWnd;
}

void  kcDocContext::SetCmdManager(kiCommandMgr *pCmdMgr)
{
	pCmdMgr_ = pCmdMgr;
}

kiCommandMgr*  kcDocContext::GetCmdManager() const
{
	return pCmdMgr_;
}

void  kcDocContext::SetSnapManager(kcSnapMngr *pSnapMgr)
{
	pSnapMgr_ = pSnapMgr;
}

kcSnapMngr*  kcDocContext::GetSnapManager() const
{
	return pSnapMgr_;
}

void  kcDocContext::SetInputEdit(kiInputEdit *pEdit)
{
	pInputEdit_ = pEdit;
}

kiInputEdit*  kcDocContext::GetInputEdit() const
{
	return pInputEdit_;
}

void  kcDocContext::SetModelTreeItf(kuiModelTreeInterface *pTreeItf)
{
	pModelTreeItf_ = pTreeItf;
}

kuiModelTreeInterface*  kcDocContext::GetModelTreeItf() const
{
	return pModelTreeItf_;
}

void  kcDocContext::SetActiveView(CView *pView)
{
	pActiveView_ = pView;
}

CView*  kcDocContext::GetActiveView() const
{
	return pActiveView_;
}

void  kcDocContext::SetOcctView(const Handle(V3d_View) &aView)
{
	aV3dView_ = aView;
}

const Handle(V3d_View)&  kcDocContext::GetOcctView() const
{
	return aV3dView_;
}

IQxTimerManager* kcDocContext::GetTimerManager() const
{
	return pCmdTimerMgr_;
}

void kcDocContext::SetTimerManager(IQxTimerManager *pMgr)
{
	pCmdTimerMgr_ = pMgr;
}

void  kcDocContext::Clear()
{
	pDocument_ = NULL;
	hMainWnd_ = NULL;
	pModel_ = NULL;
	pCmdMgr_ = NULL;
	pSnapMgr_ = NULL;
	pInputEdit_ = NULL;
	pModelTreeItf_ = NULL;
	pActiveView_ = NULL;
	pCmdTimerMgr_ = NULL;
}

double kcDocContext::GetViewToWindowScale()
{
	double scale = -1.0;
	if(!aV3dView_.IsNull()){
		double dWidth,dHeight;
		int nWidth,nHeight;

		aV3dView_->Size(dWidth,dHeight);
		Handle(Aspect_Window) aWindow = aV3dView_->Window();
		aWindow->Size(nWidth,nHeight);

		scale = dWidth / nWidth;
		TRACE("\n###--- scale is %d,V3d_View scale is %d.---\n",scale,
			aV3dView_->Scale());
	}
	return scale;
}

kcBasePlane*  kcDocContext::GetCurrentBasePlane()
{
	if(pModel_){
		return pModel_->GetCurrentBasePlane();
	}
	return NULL;
}

void  kcDocContext::RefreshCurView()
{
	if(pActiveView_){
		pActiveView_->Invalidate(FALSE);
	}
}

void  kcDocContext::RedrawCurView()
{
	if(pActiveView_){
		pActiveView_->Invalidate(FALSE);
		pActiveView_->UpdateWindow();//È·±£ÖØ»æ
	}
}

void  kcDocContext::RefreshAllView()
{
	if(pActiveView_){
		pActiveView_->Invalidate(FALSE);
	}

	if(pDocument_){
		pDocument_->UpdateAllViews(NULL);
	}
}