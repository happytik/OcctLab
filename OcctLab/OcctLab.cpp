// OcctTry.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "QxPluginMgr.h"
#include "kiCommandMgr.h"
#include "kuiSyscmdPlugin.h"
#include "kuiTrsfPlugin.h"
#include "kcHandleMgr.h"
#include "ScriptLib.h"
#include "MainFrm.h"
#include "OcctLabDoc.h"
#include "OcctLabView.h"
#include <OpenGl_GraphicDriver.hxx>
#include "OcctLab.h"


// COcctLabApp

BEGIN_MESSAGE_MAP(COcctLabApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// COcctLabApp 构造

COcctLabApp::COcctLabApp()
{
#ifdef _DEBUG
	// Disable tracking of memory for the scope of the InitInstance()
	AfxEnableMemoryTracking(FALSE);
#endif  // _DEBUG

	try
	{
		Handle(Aspect_DisplayConnection) aDisplayConnection;
		m_graphicDriver = new OpenGl_GraphicDriver (aDisplayConnection);
	}
	catch(Standard_Failure)
	{
		AfxMessageBox("Fatal error during graphic initialization",MB_ICONSTOP);
		ExitProcess(1);
	}

	m_pPluginMgr = NULL;
}


// 唯一的一个 COcctLabApp 对象

COcctLabApp theApp;

// COcctLabApp 初始化

BOOL COcctLabApp::InitInstance()
{
#ifdef _DEBUG
	// Disable tracking of memory for the scope of the InitInstance()
	AfxEnableMemoryTracking(FALSE);
#endif  // _DEBUG

	//如果初始化失败,则推出.
	if(m_graphicDriver.IsNull())
		return FALSE;

	TRACE("\n ------ CP_ACP is %d,OEM_ACP is %d.------\n",GetACP(),GetOEMCP());

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();

	CWinApp::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)

	if(!LoadPlugin()){
		return FALSE;
	}

	//脚本初始化
	///NOTE:这个地方初始化未必合适，目前先放在这里。
	//
	LuaScriptInit();

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COcctLabDoc),
		RUNTIME_CLASS(CMainFrame),       // 主 SDI 框架窗口
		RUNTIME_CLASS(COcctLabView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// 仅当存在后缀时才调用 DragAcceptFiles，
	//  在 SDI 应用程序中，这应在 ProcessShellCommand  之后发生

	return TRUE;
}

// 加载插件,获取插件的命令信息
//
BOOL	COcctLabApp::LoadPlugin()
{
	m_pPluginMgr = new QxPluginMgr;
	if(!m_pPluginMgr)
		return FALSE;

	QxCmdPlugin *pPlugin = NULL;
		
	pPlugin = new kuiTrsfPlugin;
	m_pPluginMgr->LoadPlugin(pPlugin);

	pPlugin = new kuiSyscmdPlugin;
	m_pPluginMgr->LoadPlugin(pPlugin);

	SearchAndLoadDynPlugin();

	return TRUE;
}

//动态加载插件
BOOL COcctLabApp::SearchAndLoadDynPlugin()
{
	//获取插件目录，在执行目录的plugin子目录下
	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(m_hInstance, szBuff, _MAX_PATH);
	size_t n = strnlen_s(szBuff,_MAX_PATH);
	while(n > 0 && szBuff[n-1] != '\\'){
		n --;
	}
	szBuff[n] = '\0';
	CString szPath = szBuff;
	szPath += "Plugin"; //

	//遍历dll文件
	CFileFind fileFind;
	CString szFindFlt = szPath + "\\*.dll";
	BOOL bFound = fileFind.FindFile(szFindFlt);
	while(bFound){
		bFound = fileFind.FindNextFile();
		//
		CString szDllPath = fileFind.GetFilePath();
		//
		if(m_pPluginMgr->LoadDynPlugin(szDllPath)){
			TRACE("\n###---load plugin %s.------\n",szDllPath);
		}
	}

	return TRUE;
}

int COcctLabApp::ExitInstance()
{
	if(m_pPluginMgr){
		m_pPluginMgr->Destroy();
		delete m_pPluginMgr;
	}

	return CWinApp::ExitInstance();
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void COcctLabApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// COcctLabApp 消息处理程序


