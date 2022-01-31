// OcctTryView.h : COcctLabView 类的接口
//


#pragma once

#include <V3d_View.hxx>
#include <AIS_RubberBand.hxx>

class kcBasePlane;
class kvGrid;
class kiCommandMgr;
class COcctLabDoc;
class kuiModelTreeInterface;

class COcctLabView : public CView
{
protected: // 仅从序列化创建
	COcctLabView();
	DECLARE_DYNCREATE(COcctLabView)

// 属性
public:
	COcctLabDoc*		GetDocument() const;

	Handle(V3d_View)	GetV3dView() const { return hOccView_; }

// 操作
public:
	// 更新背景色
	void				UpdateBackGroundColor();

	// 显示坐标轴
	void				UpdateTriedronDisplay();

protected:
	//
	void				UpdateSnapScreenPoint();
	void				ShowStatusPrompt(LPCTSTR lpszPrompt);

// 重写
	public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~COcctLabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	Handle(V3d_View)				hOccView_; //
	Handle(AIS_InteractiveContext)	m_aisContext;//仅仅记录引用
	kiCommandMgr					*m_pCommandMgr;//仅仅记录指针。
	kuiModelTreeInterface			*m_pIModelTree;//
	int							m_nPreX,m_nPreY;//前一个鼠标位置。
	int							m_nStartX,m_nStartY;//初始的X和Y。
	int							m_nRPreX,m_nRPreY;//前一个右键鼠标的位置
	BOOL							m_bPanning;//是否在平移
	Handle(AIS_RubberBand)		aDrawRect_;//绘制的选择矩形

	double						dViewSize_; //v3d_view的size
	Quantity_Color				aBackGroundColor_;

	//初始化view
	BOOL							InitV3dView();
	bool							CreateV3dView();
	//
	BOOL							InitInputEdit();
	//
	BOOL							InitModelTree();

	kcBasePlane*					GetCurrentBasePlane() const;

protected:
	enum eLineStyle { LS_Solid, LS_Dot, LS_ShortDash, LS_LongDash, LS_Default };
	CPen*  m_Pen;

	virtual void			DrawRectangle (const Standard_Integer  MinX  ,
		const Standard_Integer  MinY  ,
		const Standard_Integer  MaxX  ,
		const Standard_Integer  MaxY  ,
		BOOL  bDraw  ,
		const eLineStyle aLineStyle = LS_Default);

	virtual void			DrawRectangleEx (const Standard_Integer  MinX  ,
		const Standard_Integer  MinY  ,
		const Standard_Integer  MaxX  ,
		const Standard_Integer  MaxY  ,
		BOOL  bDraw  ,
		const eLineStyle aLineStyle = LS_Default);


// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMenuViewFront();
	afx_msg void OnMenuViewBack();
	afx_msg void OnMenuViewLeft();
	afx_msg void OnMenuViewRight();
	afx_msg void OnMenuViewTop();
	afx_msg void OnMenuViewBottom();
	afx_msg void OnMenuViewRestore();

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg void OnMenuDumpView();
	afx_msg void OnMenuShapeStruct();
	afx_msg void OnMenuFitall();
	afx_msg void OnMenuDispSeneitive();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
};

#ifndef _DEBUG  // OcctLabView.cpp 的调试版本
inline COcctLabDoc* COcctLabView::GetDocument() const
   { return reinterpret_cast<COcctLabDoc*>(m_pDocument); }
#endif

