//
#ifndef _KC_DOC_CONTEXT_H_
#define _KC_DOC_CONTEXT_H_

#include "QxLibDef.h"
#include "QxSysInterface.h"

class kcModel;
class kiCommandMgr;
class kcSnapMngr;
class kiInputEdit;
class kuiModelTreeInterface;
class kcBasePlane;

// 说明：
//   这里定义了一个基类，记录了Doc文档中的主要数据和变量，方便各个地方传参并使用
//
class QXLIB_API kcDocContext{
public:
	kcDocContext(CDocument *pDoc,const Handle(AIS_InteractiveContext) &aCtx,kcModel *pModel);
	virtual ~kcDocContext();

	const Handle(AIS_InteractiveContext)&	GetAISContext() const;
	kcModel*			GetModel() const;

	HWND				GetMainWnd() const;
	void				SetMainWnd(HWND hWnd);

	void				SetCmdManager(kiCommandMgr *pCmdMgr);
	kiCommandMgr*		GetCmdManager() const;

	void				SetSnapManager(kcSnapMngr *pSnapMgr);
	kcSnapMngr*		GetSnapManager() const;

	void				SetInputEdit(kiInputEdit *pEdit);
	kiInputEdit*		GetInputEdit() const;

	void				SetModelTreeItf(kuiModelTreeInterface *pTreeItf);
	kuiModelTreeInterface*	GetModelTreeItf() const;

	void				SetActiveView(CView *pView);
	CView*			GetActiveView() const;

	void				SetOcctView(const Handle(V3d_View) &aView);
	const Handle(V3d_View)&	GetOcctView() const;

	IQxTimerManager*	GetTimerManager() const;
	void				SetTimerManager(IQxTimerManager *pMgr);

	void				Clear();

public:
	double			GetViewToWindowScale();

	kcBasePlane*		GetCurrentBasePlane();

public:
	virtual void		RefreshCurView();
	virtual void		RedrawCurView();

	virtual void		RefreshAllView();

protected:
	Handle(AIS_InteractiveContext)		aAISCtx_; //
	CDocument		*pDocument_;//所在的Doc对象
	HWND				hMainWnd_; //主窗口句柄
	kcModel			*pModel_;
	kiCommandMgr		*pCmdMgr_;
	kcSnapMngr		*pSnapMgr_;
	kiInputEdit		*pInputEdit_;
	kuiModelTreeInterface	*pModelTreeItf_;//模型树接口

	CView			*pActiveView_;
	Handle(V3d_View)	aV3dView_;

	IQxTimerManager	*pCmdTimerMgr_; //定时器管理接口
};

#endif