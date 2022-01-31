// OcctTryView.cpp : COcctLabView 类的实现
//

#include "stdafx.h"

#include <WNT_Window.hxx>
#include <StdSelect_ShapeTypeFilter.hxx>
#include <Graphic3d_MapOfStructure.hxx>
#include <Graphic3d_MapIteratorOfMapOfStructure.hxx>
#include "kiCommand.h"
#include "kiCommandMgr.h"
#include "kcBasePlane.h"
#include "XInputEdit.h"
#include "kiSelSet.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "kuiInterface.h"
#include "kcSnapMngr.h"
#include "kvGrid.h"
#include "kcgTextLib.h"
#include "kcDocContext.h"
#include "QxWndTimerMgr.h"
#include "QxSysLib.h"
#include "OcctLab.h"
#include "mainFrm.h"
#include "OcctLabDoc.h"
#include "OcctLabView.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

// COcctLabView

IMPLEMENT_DYNCREATE(COcctLabView, CView)

BEGIN_MESSAGE_MAP(COcctLabView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_COMMAND(ID_MENU_VIEW_FRONT, OnMenuViewFront)
	ON_COMMAND(ID_MENU_VIEW_BACK, OnMenuViewBack)
	ON_COMMAND(ID_MENU_VIEW_LEFT, OnMenuViewLeft)
	ON_COMMAND(ID_MENU_VIEW_RIGHT, OnMenuViewRight)
	ON_COMMAND(ID_MENU_VIEW_TOP, OnMenuViewTop)
	ON_COMMAND(ID_MENU_VIEW_BOTTOM, OnMenuViewBottom)
	ON_COMMAND(ID_MENU_VIEW_RESTORE, OnMenuViewRestore)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_DESTROY()
	ON_COMMAND(ID_MENU_DUMP_VIEW, OnMenuDumpView)
	ON_COMMAND(IDM_MENU_SHAPE_STRUCT, OnMenuShapeStruct)
	ON_COMMAND(ID_MENU_FITALL, &COcctLabView::OnMenuFitall)
	ON_COMMAND(ID_MENU_DISP_SENEITIVE, &COcctLabView::OnMenuDispSeneitive)
END_MESSAGE_MAP()

// COcctLabView 构造/析构

COcctLabView::COcctLabView()
{
	// TODO: 在此处添加构造代码
	this->m_pCommandMgr = NULL;
	this->m_pIModelTree = NULL;

	m_bPanning = FALSE;

	aDrawRect_ = new AIS_RubberBand(Quantity_NOC_WHITE, Aspect_TOL_SOLID, 1.0);

	dViewSize_ = 40.0;
	aBackGroundColor_.SetValues(0.1,0.0,0.1,Quantity_TOC_RGB);
}

COcctLabView::~COcctLabView()
{
}

BOOL COcctLabView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或
	// 样式

	return CView::PreCreateWindow(cs);
}

void COcctLabView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	if(hOccView_.IsNull()){
		COcctLabDoc *pDoc = GetDocument();
		//初始化命令管理器
		pDoc->InitCommandMgr();
		//获取并记录
		m_pCommandMgr = pDoc->GetCommandMgr();
		ASSERT(m_pCommandMgr);
		
		m_aisContext = GetDocument()->GetAISContext();
		
		InitV3dView();
	}

	//进行依次必要的初始化。
	InitInputEdit();

	InitModelTree();

	//初始化默认对象，只对新建文档创建
	if(GetDocument()->IsNewDocument()){
		kcModel *pModel = GetDocument()->GetModel();
		pModel->CreateDefaultLayer();
		pModel->CreateDefaultBasePlane(hOccView_);
	}

	//初始化文字
	CClientDC dc(this);
	HDC hDC = dc.GetSafeHdc();
	if(hDC){
		glb_InitArialCharText(hDC);
		glb_InitCourierCharText(hDC);
	}

	ShowStatusPrompt("");
}

void COcctLabView::OnDestroy()
{
	CView::OnDestroy();

	if(!hOccView_.IsNull())
		hOccView_->Remove();

	kcModel *pModel = GetDocument()->GetModel();
	ASSERT(pModel);
	pModel->SetModelTreeInterface(NULL);
}

//初始化view
BOOL	COcctLabView::InitV3dView()
{
	kcModel *pModel = GetDocument()->GetModel();

	if(!CreateV3dView())
		return FALSE;
	
	pModel->SetActiveView(hOccView_);//设定关联
	//建立关联
	m_pCommandMgr->SetV3dView(hOccView_);

	kcSnapMngr *pSnapMgr = GetDocument()->GetSnapMgr();
	ASSERT(pSnapMgr != NULL);
	pSnapMgr->SetView(hOccView_);

	kcDocContext *pDocCtx = GetDocument()->GetDocContext();
	ASSERT(pDocCtx);
	pDocCtx->SetActiveView(this);
	pDocCtx->SetOcctView(hOccView_);

	// 设定主窗口
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if(pFrame){
		pDocCtx->SetTimerManager(pFrame->GetTimerMgr());
	}

	//更新显示.
	UpdateTriedronDisplay();
	return TRUE;
}

bool COcctLabView::CreateV3dView()
{
	ASSERT(!m_aisContext.IsNull());
	Handle(V3d_Viewer) aViewer = GetDocument()->GetViewer();
	if(aViewer.IsNull())
		return false;

	hOccView_ = aViewer->CreateView();
	if(hOccView_.IsNull())
		return false;

	// create window
	Handle(Graphic3d_GraphicDriver) theGraphicDriver = ((COcctLabApp *)AfxGetApp())->GetGraphicDriver();
	Handle(WNT_Window) aWNTWindow = new WNT_Window(GetSafeHwnd());
	if(aWNTWindow.IsNull())
		return FALSE;
	hOccView_->SetWindow(aWNTWindow);
	if (!aWNTWindow->IsMapped()) aWNTWindow->Map();

	// set attrib
	hOccView_->SetSize(dViewSize_);
	hOccView_->SetBackgroundColor(aBackGroundColor_);
	hOccView_->SetLightOff();

	return true;
}


// 更新背景色
void COcctLabView::UpdateBackGroundColor()
{
	if(hOccView_.IsNull())
		return;

	Quantity_Color col1(1.0,1.0,1.0,Quantity_TOC_RGB);
	Quantity_Color col2(0.0,0.0,0.0,Quantity_TOC_RGB);
	//Quantity_Color col2(1.0,0.5,0.0,Quantity_TOC_RGB);
	//hOccView_->SetBgGradientColors(col1,col2,Aspect_GFM_HOR,Standard_True);
	hOccView_->SetBackgroundColor(col2);
}

// 显示坐标轴
void	COcctLabView::UpdateTriedronDisplay()
{
	hOccView_->ZBufferTriedronSetup(Quantity_NOC_RED,Quantity_NOC_GREEN,
		Quantity_NOC_BLUE1,0.8,0.02,20);
	hOccView_->TriedronDisplay(Aspect_TOTP_LEFT_LOWER,Quantity_NOC_WHITE,0.2,V3d_ZBUFFER);
}

//
BOOL	COcctLabView::InitInputEdit()
{
	CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
	ASSERT(pMainFrm);
	CXInputEdit *pInputEdit = pMainFrm->GetInputEdit();
	ASSERT(pInputEdit);
	pInputEdit->Initialize();

	pInputEdit->PromptText("指令");

	return TRUE;
}

//
BOOL	COcctLabView::InitModelTree()
{
	CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
	ASSERT(pMainFrm);
	kuiModelTreeInterface *pModelTree = pMainFrm->GetModelTreeInterface();
	kcModel *pModel = GetDocument()->GetModel();
	if(pModel->GetModelTreeInterface() == NULL){
		pModel->SetModelTreeInterface(pModelTree);
	}
	m_pIModelTree = pModelTree;
	//
	kcDocContext *pDocCtx = GetDocument()->GetDocContext();
	ASSERT(pDocCtx);
	pDocCtx->SetModelTreeItf(pModelTree);

	return TRUE;
}

kcBasePlane* COcctLabView::GetCurrentBasePlane() const
{
	kcDocContext *pDocCtx = GetDocument()->GetDocContext();
	ASSERT(pDocCtx);
	return pDocCtx->GetCurrentBasePlane();
}

void COcctLabView::DrawRectangle(const Standard_Integer  MinX    ,
								const Standard_Integer  MinY    ,
								const Standard_Integer  MaxX ,
								const Standard_Integer  MaxY ,
								BOOL  bDraw , 
								const eLineStyle aLineStyle)
{
	static int m_DrawMode = LS_Default;

	if  (!m_Pen && aLineStyle == LS_Solid )
	{
		m_Pen = new CPen(PS_SOLID, 1, RGB(0,0,0)); 
		m_DrawMode = R2_MERGEPENNOT;
	}
	else if (!m_Pen && aLineStyle == LS_Dot )
	{
		m_Pen = new CPen(PS_DOT, 1, RGB(0,0,0));   
		m_DrawMode = R2_XORPEN;
	}
	else if (!m_Pen && aLineStyle == LS_ShortDash)
	{
		m_Pen = new CPen(PS_DASH, 1, RGB(255,0,0));	
		m_DrawMode = R2_XORPEN;
	}
	else if (!m_Pen && aLineStyle == LS_LongDash)
	{
		m_Pen = new CPen(PS_DASH, 1, RGB(0,0,0));	
		m_DrawMode = R2_NOTXORPEN;
	}
	else if (aLineStyle == LS_Default) 
	{ 
		m_Pen = NULL;	
		m_DrawMode = R2_MERGEPENNOT;
	}

	CPen* pOldPen = NULL;
	CClientDC clientDC(this);
	if (m_Pen) 
		pOldPen = clientDC.SelectObject(m_Pen);
	clientDC.SetROP2(m_DrawMode);

	static	int nStoredMinX, nStoredMaxX, nStoredMinY, nStoredMaxY;
	static	BOOL m_bVisible = FALSE;

	if (m_bVisible && !bDraw) // move or up  : erase at the old position 
	{
		clientDC.MoveTo(nStoredMinX,nStoredMinY); 
		clientDC.LineTo(nStoredMinX,nStoredMaxY); 
		clientDC.LineTo(nStoredMaxX,nStoredMaxY); 
		clientDC.LineTo(nStoredMaxX,nStoredMinY); 
		clientDC.LineTo(nStoredMinX,nStoredMinY);
		m_bVisible = FALSE;
	}

	nStoredMinX = min ( MinX, MaxX );
	nStoredMinY = min ( MinY, MaxY );
	nStoredMaxX = max ( MinX, MaxX );
	nStoredMaxY = max ( MinY, MaxY);

	if (bDraw) // move : draw
	{
		clientDC.MoveTo(nStoredMinX,nStoredMinY); 
		clientDC.LineTo(nStoredMinX,nStoredMaxY); 
		clientDC.LineTo(nStoredMaxX,nStoredMaxY); 
		clientDC.LineTo(nStoredMaxX,nStoredMinY); 
		clientDC.LineTo(nStoredMinX,nStoredMinY);
		m_bVisible = TRUE;
	}

	if (m_Pen) clientDC.SelectObject(pOldPen);
}

void COcctLabView::DrawRectangleEx (const Standard_Integer  MinX  ,
		const Standard_Integer  MinY  ,
		const Standard_Integer  MaxX  ,
		const Standard_Integer  MaxY  ,
		BOOL  bDraw  ,
		const eLineStyle aLineStyle)
{
	Handle(AIS_InteractiveContext) aCtx = GetDocument()->GetAISContext();
	if(!bDraw){
		aCtx->Remove(aDrawRect_,false);
		aCtx->CurrentViewer()->RedrawImmediate();
		return;
	}

	CRect rect;
	GetWindowRect(rect);
	//设定属性
	switch(aLineStyle){
	case LS_Solid:
		aDrawRect_->SetLineColor(Quantity_NOC_WHITE);
		aDrawRect_->SetLineType(Aspect_TOL_SOLID);
		break;
	case LS_Dot:
		aDrawRect_->SetLineColor(Quantity_NOC_WHITE);
		aDrawRect_->SetLineType(Aspect_TOL_DOT);
		break;
	case LS_ShortDash:
		aDrawRect_->SetLineColor(Quantity_Color(1.0,0,0,Quantity_TOC_RGB));
		aDrawRect_->SetLineType(Aspect_TOL_DASH);
		break;
	case LS_LongDash:
		aDrawRect_->SetLineColor(Quantity_NOC_WHITE);
		aDrawRect_->SetLineType(Aspect_TOL_DOTDASH);
		break;
	case LS_Default:
	default:
		aDrawRect_->SetLineColor(Quantity_NOC_WHITE);
		aDrawRect_->SetLineType(Aspect_TOL_SOLID);
		break;
	}
	aDrawRect_->SetRectangle(MinX,rect.Height()-MinY,MaxX,rect.Height() - MaxY);
	if(!aCtx->IsDisplayed(aDrawRect_)){
		aCtx->Display(aDrawRect_,false);
	}else{
		aCtx->Redisplay(aDrawRect_,false);
	}
	aCtx->CurrentViewer()->RedrawImmediate();
}

// COcctLabView 绘制

void COcctLabView::OnDraw(CDC* /*pDC*/)
{
	COcctLabDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if(!hOccView_.IsNull())
	{
		hOccView_->Redraw();
	}
}


// COcctLabView 打印

BOOL COcctLabView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void COcctLabView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印前添加额外的初始化
}

void COcctLabView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 打印后添加清除过程
}


// COcctLabView 诊断

#ifdef _DEBUG
void COcctLabView::AssertValid() const
{
	CView::AssertValid();
}

void COcctLabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

COcctLabDoc* COcctLabView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COcctLabDoc)));
	return (COcctLabDoc*)m_pDocument;
}
#endif //_DEBUG


// COcctLabView 消息处理程序

void COcctLabView::OnLButtonDblClk(UINT nFlags, CPoint point)
{


	CView::OnLButtonDblClk(nFlags, point);
}

void COcctLabView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_nStartX = m_nPreX = point.x;
	m_nStartY = m_nPreY = point.y;

	if(m_pCommandMgr && m_pCommandMgr->HasCommandRunning())
	{
		kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
		m_pCommandMgr->OnLButtonDown(mouseInput);
	}

	CView::OnLButtonDown(nFlags, point);
}

void COcctLabView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(nFlags & MK_CONTROL)
	{//平移
		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
	else
	{
		// 如果命令在运行,有命令处理消息,否则默认处理
		if(m_pCommandMgr && m_pCommandMgr->HasCommandRunning())
		{
			kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
			m_pCommandMgr->OnLButtonUp(mouseInput);
		}
		else
		{
			if((point.x == this->m_nStartX) && 
				(point.y == this->m_nStartY))//和初始点进行比较，没有移动
			{
				if(nFlags & MK_SHIFT)
				{
					m_aisContext->SelectDetected(AIS_SelectionScheme_XOR);
				}
				else
				{
					m_aisContext->SelectDetected(AIS_SelectionScheme_Replace);
				}
			}
			else
			{//框选
				//隐藏选择框
				this->DrawRectangleEx(this->m_nStartX,this->m_nStartY,point.x,point.y,FALSE);

				if(nFlags & MK_SHIFT)
				{
					m_aisContext->SelectRectangle(Graphic3d_Vec2i(m_nStartX,m_nStartY),
						Graphic3d_Vec2i(point.x,point.y),hOccView_,
						AIS_SelectionScheme_XOR);
				}
				else
				{
					m_aisContext->SelectRectangle(Graphic3d_Vec2i(m_nStartX,m_nStartY),
						Graphic3d_Vec2i(point.x,point.y),hOccView_,
						AIS_SelectionScheme_Replace);
				}
			}

			kcModel *pModel = GetDocument()->GetModel();
			kiSelSet *pGlobalSelSet = GetDocument()->GetGlobalSelSet();
			pGlobalSelSet->GetSelected();
			pGlobalSelSet->Hilight(Quantity_NOC_BLUE1);
			//更新tree显示
			if(m_pIModelTree){
				m_pIModelTree->UnBoldAllItem(eEntityItem);
				for(pGlobalSelSet->InitSelected();pGlobalSelSet->MoreSelected();
					pGlobalSelSet->NextSelected())
				{
					kiSelEntity se = pGlobalSelSet->CurSelected();
					m_pIModelTree->BoldTreeItem(eEntityItem,se._pEntity->GetName());
				}
			}
			//
			m_aisContext->UpdateCurrentViewer();
		}
	}

	CView::OnLButtonUp(nFlags, point);
}

void COcctLabView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_aisContext.IsNull())
		return;

	//如果没有焦点，设置交点
	CWnd *pActiveWnd = this->GetActiveWindow();
	if(pActiveWnd)
	{
		if((pActiveWnd->GetSafeHwnd() != this->GetSafeHwnd()) &&
			(pActiveWnd->GetSafeHwnd() != NULL))
		{
			if((point.x != this->m_nPreX) ||
				(point.y != this->m_nPreY))//鼠标移动，不再原处，则设置焦点.
			{
				//DTRACE("\nOnMouseMove SetFocus...");
				SetFocus();
			}
		}
	}

	//先处理旋转
	if(nFlags & MK_MBUTTON)
	{
		this->hOccView_->Rotation(point.x,point.y);
		this->hOccView_->Redraw();

		//记录当前点
		this->m_nPreX = point.x;
		this->m_nPreY = point.y;
	}
	else if(nFlags & MK_LBUTTON)
	{
		if(nFlags & MK_CONTROL)
		{
			
		}
		else if(nFlags & MK_SHIFT)
		{

		}
		else
		{
			//有命令,则由命令处理
			if(m_pCommandMgr && m_pCommandMgr->HasCommandRunning())
			{
				kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
				m_pCommandMgr->OnMouseMove(mouseInput);
			}
			else
			{
				//框选操作

				//隐藏选择框
				this->DrawRectangleEx(this->m_nStartX,this->m_nStartY,point.x,point.y,FALSE);

				if(nFlags & MK_SHIFT)
				{
					m_aisContext->SelectRectangle(Graphic3d_Vec2i(m_nStartX,m_nStartY),
						Graphic3d_Vec2i(point.x,point.y),hOccView_,
						AIS_SelectionScheme_XOR);
				}
				else
				{
					m_aisContext->SelectRectangle(Graphic3d_Vec2i(m_nStartX,m_nStartY),
						Graphic3d_Vec2i(point.x,point.y),hOccView_,
						AIS_SelectionScheme_Replace);
				}

				//显示选择框选
				this->DrawRectangleEx(this->m_nStartX,this->m_nStartY,point.x,point.y,TRUE);
			}
		}
	}
	else if(nFlags & MK_RBUTTON)
	{
		//缩放
		if(nFlags & MK_CONTROL)
		{
			this->hOccView_->Zoom(m_nPreX,m_nPreY,point.x,point.y);
			//记录当前点
			this->m_nPreX = point.x;
			this->m_nPreY = point.y;
		}
		else// if(nFlags & MK_SHIFT)//平移
		{
			//这里默认鼠标右键是平移，避免太小改变导致的移动
			this->hOccView_->Pan(point.x - this->m_nRPreX,
				this->m_nRPreY - point.y);

			//记录当前点
			this->m_nRPreX = point.x;
			this->m_nRPreY = point.y;
			m_bPanning = TRUE;
		}
	}
	else
	{
		//有命令,则由命令处理
		if(m_pCommandMgr && m_pCommandMgr->HasCommandRunning())
		{
			kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
			m_pCommandMgr->OnMouseMove(mouseInput);
		}
		else
		{
			//鼠标移动的消息
			m_aisContext->MoveTo(point.x,point.y,hOccView_,Standard_True);

			//记录当前点
			this->m_nPreX = point.x;
			this->m_nPreY = point.y;

			kcBasePlane *pWorkPlane = GetCurrentBasePlane();
			if(pWorkPlane){
				kPoint3 pnt;
				pWorkPlane->PixelToWorkPlane(point.x,point.y,pnt);
				CString szMsg;
				szMsg.Format("%.4f,%.4f,%.4f",pnt.x(),pnt.y(),pnt.z());
				ShowStatusPrompt(szMsg);
			}
		}
	}

	CView::OnMouseMove(nFlags, point);
}

void COcctLabView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if(nFlags & MK_SHIFT)
	{

	}
	else
	{
		//hOccView_->SetDegenerateModeOn();
		this->hOccView_->StartRotation(point.x,point.y);
		this->m_nPreX = point.x;
		this->m_nPreY = point.y;
	}

	CView::OnMButtonDown(nFlags, point);
}

void COcctLabView::OnMButtonUp(UINT nFlags, CPoint point)
{
	//重新计算捕捉点的屏幕坐标
	UpdateSnapScreenPoint();

	CView::OnMButtonUp(nFlags, point);
}

// 目前先实现为这样，基本想法是：
// 由于OCC只能基于屏幕中心进行缩放（？？？需要进一步的确认），因此，当希望模型局部显示时
// 首先移动到屏幕中心，然后再进行缩放。
//
BOOL COcctLabView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	static CPoint s_prePnt;
	Standard_Real dViewWith,dViewHeight;
	this->hOccView_->Size(dViewWith,dViewHeight);

	double dCoef = 1.0;
	BOOL bZoom = TRUE;
	if(zDelta > 0.0){//放大
		if(dViewWith < 0.01 || dViewHeight < 0.01)
			bZoom = FALSE;
		else
			dCoef = 5.0/4.0;
	}else{//缩小
		if(dViewWith > 1000000.0 || dViewHeight > 1000000.0)
			bZoom = FALSE;
		else
			dCoef = 4.0/5.0;
	}

	//太小或太大禁止缩放
	if(bZoom){
#ifdef _R_DEBUG
		//使鼠标点所在的模型位置移动到屏幕中心进行缩放
		// 位置改变了，才重设中心
		if(abs(s_prePnt.x - pt.x) > 4 || abs(s_prePnt.y - pt.y) > 4){
			CRect rc;
			CPoint scp;
			int mx,my,icx,icy;

			scp = pt;
			this->ScreenToClient(&scp);//pt是基于屏幕的坐标!!!
			GetClientRect(&rc);
			mx = (rc.left + rc.right) / 2;
			my = (rc.top + rc.bottom) / 2;
			TRACE("\n x %d,y %d,mx %d,my %d",scp.x,scp.y,mx,my);
			icx = mx - scp.x;
			icy = scp.y - my;//注意平面坐标系和投影平面坐标系的Y向相反
			TRACE("\n icx %d,icy %d",icx,icy);
			double len = sqrt(icx * icx + icy * icy);
			double mrat = 100 / len;
			if(mrat > 1.0) mrat = 1.0;
			icx = (int)(icx * mrat);
			icy = (int)(icy * mrat);
								
			hOccView_->Pan(icx,icy);
		}
#endif
		this->hOccView_->SetZoom(dCoef,Standard_True);		
		this->hOccView_->Redraw();

		//视图变换，需要更新snap点
//		UpdateSnapPoints();
	}

	s_prePnt = pt;

	//重新计算捕捉点的屏幕坐标
	UpdateSnapScreenPoint();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void COcctLabView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// 处理平移或缩放
	if((nFlags & MK_CONTROL) || (nFlags & MK_SHIFT)){
		m_nPreX = point.x;
		m_nPreY = point.y;
	}

	//为了平移
	m_nRPreX = point.x;
	m_nRPreY = point.y;
	m_bPanning = FALSE;

	// 对应的命令进行处理。
	if(m_pCommandMgr){
		kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
		m_pCommandMgr->OnRButtonDown(mouseInput);
	}

	CView::OnRButtonDown(nFlags, point);
}

void COcctLabView::OnRButtonUp(UINT nFlags, CPoint point)
{
	//平移状态下，命令管理器不处理
	if(m_pCommandMgr && !m_bPanning){
		kuiMouseInput mouseInput(nFlags,point.x,point.y,hOccView_);
		m_pCommandMgr->OnRButtonUp(mouseInput);
	}

	m_bPanning = FALSE;
	m_nRPreX = m_nRPreY = 0;

	CView::OnRButtonUp(nFlags, point);
}


BOOL COcctLabView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return TRUE;
	//return CView::OnEraseBkgnd(pDC);
}

void COcctLabView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (!hOccView_.IsNull())
	{
		hOccView_->MustBeResized();

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

//
void	COcctLabView::UpdateSnapScreenPoint()
{
	kcSnapMngr *pSnapMgr = GetDocument()->GetSnapMgr();
	if(pSnapMgr)
	{
		pSnapMgr->CalcScreenPoint();
	}
}

void	COcctLabView::ShowStatusPrompt(LPCTSTR lpszPrompt)
{
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if(!pFrame){
		COcctLabApp *pApp = (COcctLabApp *)AfxGetApp();
		pFrame = (CMainFrame *)pApp->GetMainWnd();
	}
	if(pFrame){
		pFrame->ShowPrompt(lpszPrompt);
	}
}

//////////////////////////////////////////////////////////////////////////
// 视图相关.
//
void  COcctLabView::OnMenuViewFront()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Yneg);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnMenuViewBack()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Ypos);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnMenuViewLeft()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Xneg);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnMenuViewRight()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Xpos);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnMenuViewTop()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Zpos);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnMenuViewBottom()
{
	if(!hOccView_.IsNull()){
		hOccView_->SetProj(V3d_Zneg);

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

// 默认XY平面，也是基准面
void COcctLabView::OnMenuViewRestore()
{
	if(!hOccView_.IsNull()){
		kcModel *pModel = GetDocument()->GetModel();
		ASSERT(pModel);
		kcBasePlane *pBasePlane = pModel->GetCurrentBasePlane();
		ASSERT(pBasePlane);
		pBasePlane->SetDefaultView();

		//重新计算捕捉点的屏幕坐标
		UpdateSnapScreenPoint();
	}
}

void COcctLabView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//
	if(m_pCommandMgr != NULL){
		kuiKeyInput keyInput(nChar,nRepCnt,nFlags);
		m_pCommandMgr->OnKeyDown(keyInput);
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void COcctLabView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nChar == VK_ESCAPE)
	{
		if(m_pCommandMgr && m_pCommandMgr->HasCommandRunning())
		{
			m_pCommandMgr->EndCommand(KSTA_CANCEL);
		}
	}
	//
	if(m_pCommandMgr != NULL){
		//执行删除命令
		if(nChar == VK_DELETE && !m_pCommandMgr->HasCommandRunning()){
			m_pCommandMgr->ExecCommand(ID_EDIT_DELETE);
		}

		kuiKeyInput keyInput(nChar,nRepCnt,nFlags);
		m_pCommandMgr->OnKeyUp(keyInput);
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

// 输出当前view的处理信息。
void COcctLabView::OnMenuDumpView()
{
	if(hOccView_.IsNull())
		return;

	TCHAR szModulePath[512];
	DWORD dwSize,ix;
	dwSize = ::GetModuleFileName(NULL,szModulePath,512);
	ix = dwSize - 1;
	while(szModulePath[ix] != '\\')
		ix --;
	szModulePath[ix + 1] = '\0';
	CString szOutFile = szModulePath;
	szOutFile = szOutFile + "dumpview.txt";
	//FILE *fout = freopen(szOutFile,"w",stdout);
	//if(fout)
	//{
//		Handle(Visual3d_View) aView = hOccView_->View();
//
//		printf("\n ****************dump view********************\n");
////		aView->Exploration();
//
//		printf("\n ****************dump displayed structures********************\n");
//		//每个stru的dump
//		Graphic3d_MapOfStructure SG;
//		aView->DisplayedStructures(SG);
//
//		Handle(Graphic3d_Structure) G;
//		for(Graphic3d_MapIteratorOfMapOfStructure It(SG); It.More();It.Next())
//		{
//			G = It.Key();
//			printf("\n ****************dump structures********************\n");
////			G->Exploration();
//		}


		//fclose(fout);

		//::ShellExecute(NULL,"open",szOutFile,NULL,NULL,SW_SHOWNORMAL);
	//}
}

void COcctLabView::OnMenuShapeStruct()
{
	if(!m_aisContext.IsNull())
	{
		//m_aisContext->Select();
		m_aisContext->InitSelected();
		if(m_aisContext->MoreSelected())
		{
			Handle(AIS_InteractiveObject) aObj = m_aisContext->SelectedInteractive();
			Handle(AIS_Shape) aShapeObj = Handle(AIS_Shape)::DownCast(aObj);
			TopoDS_Shape aShape = aShapeObj->Shape();
			if(!aShape.IsNull())
			{
				ShowShapeStructDialog(aShape);
			}
		}
	}
}


void COcctLabView::OnMenuFitall()
{
	// TODO: 在此添加命令处理程序代码
	if(!hOccView_.IsNull()){
		hOccView_->FitAll();
		hOccView_->ZFitAll();
	}
}


void COcctLabView::OnMenuDispSeneitive()
{
	static bool s_bDispSensitive = false;
	if(m_aisContext.IsNull())
		return;

	if(!s_bDispSensitive){
		m_aisContext->DisplayActiveSensitive(hOccView_);
	}else{
		m_aisContext->ClearActiveSensitive(hOccView_);
	}
	s_bDispSensitive = !s_bDispSensitive;
}


void COcctLabView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: 在此添加专用代码和/或调用基类
	TRACE("\n###---------------OnActivateView-------.\n");

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
