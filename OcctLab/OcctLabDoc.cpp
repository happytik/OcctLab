// OcctTryDoc.cpp :  COcctLabDoc 类的实现
//

#include "stdafx.h"
#include <OpenGl_GraphicDriver.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Graphic3d_AspectFillArea3d.hxx>
#include "QxCommandItem.h"
#include "QxPluginMgr.h"
#include "kiCommandMgr.h"
#include "kcModel.h"
#include "kcSnapMngr.h"
#include "kiCommand.h"
#include "kiUndoMgr.h"
#include "OcctLab.h"
#include "kcIGESReader.h"
#include "kcDXFReader.h"
#include "kcBRepReader.h"
#include "kcBRepExport.h"
#include "kcSTEPReader.h"
#include "kcSTEPExport.h"
#include "kcIGESExport.h"
#include "QxStgDocWriter.h"
#include "QxStgDocReader.h"
#include "kuiInterface.h"
#include "kiSelSet.h"
#include "kcDocContext.h"
#include "QxLuaScript.h"
#include "kcmDelete.h"
#include "mainfrm.h"
#include "OcctLabDoc.h"

// COcctLabDoc

IMPLEMENT_DYNCREATE(COcctLabDoc, CDocument)

BEGIN_MESSAGE_MAP(COcctLabDoc, CDocument)
	ON_COMMAND_RANGE(KI_CMD_ID_BEGIN,KI_CMD_ID_END,OnMenuItemDispatch)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_MENU_IMPORT_IGES, OnMenuImportIges)
	ON_COMMAND(ID_MENU_IMPORT_DXF, OnMenuImportDxf)
	ON_COMMAND(IDM_IMPORT_BREP_FILE, OnImportBrepFile)
	ON_COMMAND(ID_IMPORT_STEP_FILE, &COcctLabDoc::OnImportStepFile)
	ON_COMMAND(ID_MENU_EXPORT_STEP, &COcctLabDoc::OnMenuExportStep)
	ON_COMMAND(ID_MENU_EXPORT_BREP, &COcctLabDoc::OnMenuExportBrep)
	ON_COMMAND(ID_MENU_EXPORT_SELSTEP, &COcctLabDoc::OnMenuExportSelstep)
	ON_COMMAND(ID_MENU_EXPORT_SELBREP, &COcctLabDoc::OnMenuExportSelbrep)
	ON_COMMAND(ID_MENU_EXPORT_IGES, &COcctLabDoc::OnMenuExportIges)
	ON_COMMAND(IDM_EXPORT_SEL_IGES, &COcctLabDoc::OnExportSelIges)
END_MESSAGE_MAP()


// COcctLabDoc 构造/析构

COcctLabDoc::COcctLabDoc()
{
	// TODO: 在此添加一次性构造代码
	m_pModel = NULL;
	m_pCommandMgr = NULL;
	m_pSnapMgr = NULL;
	m_bNewDocument = TRUE;
	m_bSerializeOK = FALSE;

	OcctInit();
}

COcctLabDoc::~COcctLabDoc()
{
	if(m_pModel){
		m_pModel->Destroy();
		delete m_pModel;
	}

	if(m_pCommandMgr){
		m_pCommandMgr->Destroy();
		delete m_pCommandMgr;
	}

	if(m_pSnapMgr){
		delete m_pSnapMgr;
	}

	if(!m_AISContext.IsNull()){
		m_AISContext->RemoveAll(Standard_True);

		Handle(Graphic3d_GraphicDriver) hGLDrver = ((COcctLabApp *)AfxGetApp())->GetGraphicDriver();
		if(!hGLDrver.IsNull()){
			int num = hGLDrver->InquireViewLimit();
		}

		//m_AISContext.Nullify();

	}

	if(!m_Viewer.IsNull()){
		m_Viewer.Nullify();
		//
		//Handle(V3d_Viewer) aViewer;
		//glb_GetContext().SetViewer(aViewer);
	}
}

// 构造函数初始化调用。
//
BOOL	COcctLabDoc::OcctInit()
{
	Handle(Graphic3d_GraphicDriver) theGraphicDriver = ((COcctLabApp *)AfxGetApp())->GetGraphicDriver();
	if(theGraphicDriver.IsNull())
		return FALSE;

	//创建viewer对象，每个Doc一个
	m_Viewer = new V3d_Viewer(theGraphicDriver);
	m_Viewer->SetDefaultLights();
	m_Viewer->SetLightOn();

	//创建Context对象，每个doc一个
	m_AISContext = new AIS_InteractiveContext(m_Viewer);

	//渲染模式.
	m_AISContext->SetDisplayMode(AIS_Shaded,Standard_False);
	m_AISContext->SetIsoNumber(3);
	m_AISContext->IsoOnPlane(Standard_True);
	//m_AISContext->SelectionColor(Quantity_NOC_GREEN);
	//开始带变线显示模式
	Handle(Prs3d_Drawer) aDrawer = m_AISContext->DefaultDrawer();
	aDrawer->SetFaceBoundaryDraw(true);

	//剖分精度方式
	TRACE("\n DeviationCoefficient is %f.",m_AISContext->DeviationCoefficient());
	TRACE("\n MaximalDeviationCoefficient is %f.",aDrawer->MaximalChordialDeviation());
	TRACE("\n DeviationAlgle is %f.",m_AISContext->DeviationAngle());

	Aspect_TypeOfDeflection defType = aDrawer->TypeOfDeflection();
	if(defType == Aspect_TOD_RELATIVE){
		TRACE("\n type of deflection is Aspect_TOD_RELATIVE.");
	}else{
		TRACE("\n type of deflection is Aspect_TOD_ABSOLUTE.");
	}
	// 设定为绝对值模式，显示效果更好。
	aDrawer->SetTypeOfDeflection(Aspect_TOD_ABSOLUTE);//取绝对值
	aDrawer->SetMaximalChordialDeviation(0.01);
	//m_AISContext->SetDeviationCoefficient(0.1);

	// 创建模型对象，每个Doc一个
	m_pModel = new kcModel(m_AISContext);
	//记录必要的对象，以便命令等对象可以方便的访问
	m_pModel->Initialize();

	m_pSnapMgr = new kcSnapMngr(m_pModel);

	pDocContext_ = new kcDocContext(this,m_AISContext,m_pModel);
	pDocContext_->SetSnapManager(m_pSnapMgr);

	//脚本初始化
	LuaScriptInit(pDocContext_);

	return TRUE;
}

// 初始化命令管理器
// 这里，由于需要使用到一些窗口对象，需要创建后才能使用，
// CDocument的构造函数，还没有创建窗口对象，因此不能调用该函数
BOOL	COcctLabDoc::InitCommandMgr()
{
	if(m_pCommandMgr)
		return TRUE;

	QxPluginMgr *pPluginMgr = ((COcctLabApp *)AfxGetApp())->GetPluginMgr();
	if(!pPluginMgr)
		return FALSE;

	QxCommandItem *pCmdItem = NULL;
	UINT iCount = pPluginMgr->GetItemCount();
	if(iCount == 0)
		return FALSE;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	kiInputEdit *pInputEdit = pFrame->GetInputEdit();
	pDocContext_->SetInputEdit(pInputEdit);

	//创建命令管理器对象，没有Doc一个对象
	m_pCommandMgr = new kiCommandMgr;
	m_pCommandMgr->Initialize(pDocContext_,pPluginMgr);
		
	pDocContext_->SetCmdManager(m_pCommandMgr);
	
	//注册系统命令，有对应的已有菜单项，
	RegisterSysCommand();

	return TRUE;
}

kiSelSet* COcctLabDoc::GetGlobalSelSet() const
{
	if(m_pModel){
		return m_pModel->GetGlobalSelSet();
	}
	return NULL;
}

BOOL COcctLabDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	m_bNewDocument = TRUE;
	m_pModel->Clear();
	m_pSnapMgr->Clear();
	//
	CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
	if(pMainFrm){
		kuiModelTreeInterface *pIF = pMainFrm->GetModelTreeInterface();
		if(pIF){
			pIF->SetModel(m_pModel);
			pIF->Initialize();
		}
	}
	
	//m_pModel->CreateDefaultLayer();

	return TRUE;
}

BOOL COcctLabDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	m_bNewDocument = FALSE;
	m_pModel->Clear();
	m_pSnapMgr->Clear();
	//
	CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
	if(pMainFrm)
	{
		kuiModelTreeInterface *pIF = pMainFrm->GetModelTreeInterface();
		if(pIF){
			pIF->SetModel(m_pModel);
			pIF->Initialize();
		}
	}

	m_bSerializeOK = FALSE;
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if(!m_bSerializeOK){
		return FALSE;
	}
	
	return TRUE;
}

BOOL COcctLabDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_bSerializeOK = FALSE;
	if(!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;
	return m_bSerializeOK;
}

void COcctLabDoc::DeleteContents()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDocument::DeleteContents();
}

void COcctLabDoc::OnCloseDocument()
{
	// TODO: 在此添加专用代码和/或调用基类
	// 文档关闭后，
	pDocContext_->Clear();

	CDocument::OnCloseDocument();
}

void COcctLabDoc::SetTitle(LPCTSTR lpszTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	CDocument::SetTitle(lpszTitle);
}

// COcctLabDoc 序列化

// 这里，主要保存了Model的数据，还应当可以保存系统的一些数据
//
void COcctLabDoc::Serialize(CArchive& ar)
{
	bool bRc = false;

	if (ar.IsStoring()){
		QxStgDocWriter aDocWiter(pDocContext_);
		bRc = aDocWiter.Save(ar);
	}
	else
	{
		QxStgDocReader aDocReader(pDocContext_);
		bRc = aDocReader.Load(ar);
	}
	m_bSerializeOK = bRc ? TRUE : FALSE;
}

// COcctLabDoc 诊断

#ifdef _DEBUG
void COcctLabDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COcctLabDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// 注册系统命令
// 用于一些不和菜单关联或比较特殊的命令.
//
BOOL  COcctLabDoc::RegisterSysCommand()
{
	ASSERT(m_pCommandMgr);
	kiCommand *pCommand = NULL;

	if(m_pCommandMgr){
		pCommand = new kcmDelete;
		pCommand->SetID(ID_EDIT_DELETE);
		m_pCommandMgr->RegisterCommand(pCommand);
	}

	return TRUE;
}

// COcctLabDoc 命令
void	COcctLabDoc::OnMenuItemDispatch(UINT nID)
{
	if(m_pCommandMgr){
		m_pCommandMgr->ExecCommand(nID);
	}
}

// 执行undo命令
void COcctLabDoc::OnEditUndo()
{
	if(!m_pModel) return;

	kiUndoMgr *pUndoMgr = m_pModel->GetUndoMgr();
	if(!pUndoMgr)
		return;

	if(pUndoMgr->Undo())
	{
		UpdateAllViews(NULL);
	}
}

void COcctLabDoc::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	if(!m_pModel) return;

	kiUndoMgr *pUndoMgr = m_pModel->GetUndoMgr();
	if(!pUndoMgr)
		return;

	if(!pUndoMgr || !pUndoMgr->CanUndo())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

// 执行重做命令
void COcctLabDoc::OnEditRedo()
{
	if(!m_pModel) return;

	kiUndoMgr *pUndoMgr = m_pModel->GetUndoMgr();
	if(!pUndoMgr)
		return;

	if(pUndoMgr->Redo())
	{
		UpdateAllViews(NULL);
	}
}

void COcctLabDoc::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	if(!m_pModel) return;

	kiUndoMgr *pUndoMgr = m_pModel->GetUndoMgr();
	if(!pUndoMgr)
		return;

	if(!pUndoMgr || !pUndoMgr->CanRedo())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

// 删除对象操作。
void COcctLabDoc::OnEditDelete()
{
	if(m_pCommandMgr)
		m_pCommandMgr->ExecCommand(ID_EDIT_DELETE);
}

void COcctLabDoc::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
	
}

void COcctLabDoc::OnMenuImportIges()
{
	kcIGESReader igesReader(m_pModel);
	if(igesReader.DoImport())
	{
		UpdateAllViews(NULL);
	}
}

void COcctLabDoc::OnMenuExportIges()
{
	kcIGESExport aExport(m_pModel);
	aExport.DoExport();
}

void COcctLabDoc::OnExportSelIges()
{
	kcIGESExport aExport(m_pModel);
	aExport.EnableSelectExport(true);
	aExport.SetAISContext(m_AISContext);

	aExport.DoExport();
}

void COcctLabDoc::OnMenuImportDxf()
{
	//CMainFrame *pMainFrm = (CMainFrame *)AfxGetMainWnd();
	//COcctLabDoc *pDoc = (COcctLabDoc *)pMainFrm->GetActiveDocument();
	kcDXFReader dxfReader(m_pModel);
	if(dxfReader.DoImport())
	{
		UpdateAllViews(NULL);
	}
}


void COcctLabDoc::OnImportBrepFile()
{
	kcBRepReader brepReader(m_pModel);
	if(brepReader.DoImport())
	{
		UpdateAllViews(NULL);
	}
}


void COcctLabDoc::OnImportStepFile()
{
	kcXCAFSTEPReader stepReader(m_pModel);
	//kcSTEPReader stepReader(m_pModel);
	if(stepReader.DoImport())
	{
		UpdateAllViews(NULL);
	}
}


void COcctLabDoc::OnMenuExportStep()
{
	kcSTEPExport stepExp(m_pModel);
	stepExp.DoExport();
}


void COcctLabDoc::OnMenuExportBrep()
{
	kcBRepExport brepExp(m_pModel);
	brepExp.DoExport();
}


void COcctLabDoc::OnMenuExportSelstep()
{
	kcSTEPExport stepExp(m_pModel);
	stepExp.EnableSelectExport(true);
	stepExp.SetAISContext(m_AISContext);

	int nRc = AfxMessageBox("作为组合导出?",MB_YESNO);
	if(nRc == IDYES){
		stepExp.ExportAsCompound(true);
	}

	stepExp.DoExport();
}


void COcctLabDoc::OnMenuExportSelbrep()
{
	kcBRepExport brepExp(m_pModel);
	brepExp.EnableSelectExport(true);
	brepExp.SetAISContext(m_AISContext);
	brepExp.DoExport();
}





