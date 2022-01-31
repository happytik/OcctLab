#include "StdAfx.h"
#include "QxCommandItem.h"
#include "kcmDispVisible.h"
#include "kcmUCSMgr.h"
#include "kcmShapeAnalysis.h"
#include "kcmDbgTool.h"
#include "kuiSyscmdPlugin.h"

enum eSysCmdPluginCmdType{
	CMD_DISP_HIDESEL = 0,
	CMD_DISP_SHOWALL,
	CMD_DISP_TYPE_FILLTER,
	CMD_DISP_HIDE_GRID,
	CMD_CHECK_SHAPE_VALID,
	CMD_CHECK_TOPOL_VALID,
	CMD_CHECK_SHAPE_CLOSED,
	CMD_ANALYSIS_FACE_NORMAL,
	CMD_ANALYSIS_SHAPE_NORMAL,
	CMD_ANALYSIS_FACE_ORIENT,
	CMD_ANALYSIS_FACE_UV_ORIENT,
	CMD_ANALYSIS_FACE_INFO,
	CMD_ANALYSIS_FACE_WIREORI,
	CMD_ANALYSIS_BAT_STEPFILE,
	CMD_ANALYSIS_NBS_POLES,
	CMD_ANALYSIS_CURVE_CURVATURE,
	CMD_ANALYSIS_MESH_DISPLAY,
	CMD_FIX_SHAPE_FIX,
	CMD_DBGTOOL_INPUT_FNL,
	CMD_TEST_FOR_DEBUG,
};

kuiSyscmdPlugin::kuiSyscmdPlugin(void)
{
	_strName = "系统命令插件";
	_nReserveNum = 50;
}

kuiSyscmdPlugin::~kuiSyscmdPlugin(void)
{
}

//初始化
BOOL  kuiSyscmdPlugin::Initialize()
{
	const char *pszDispMenu = "显示(&D)";
	const char *pszViewMenu = "视图(&V)";
	const char *pszToolMenu = "工具(&T)";
	const char *pszAnalysisMenu = "分析(&S)";

	AddCommandItem((int)CMD_DISP_HIDESEL,pszDispMenu,"隐藏选择对象(&H)");
	AddCommandItem((int)CMD_DISP_SHOWALL,pszDispMenu,"显示隐藏对象(&H)");
	AddCommandItem((int)CMD_DISP_TYPE_FILLTER,pszDispMenu,"过滤显示对象(&F)");
	AddCommandItem((int)CMD_DISP_HIDE_GRID,pszDispMenu,"隐藏网格显示");
		

	AddCommandItem((int)CMD_ANALYSIS_FACE_ORIENT,pszToolMenu,"分析","曲面方向(&O)",KUI_MENU_BEFORE_SEP);
	AddCommandItem((int)CMD_ANALYSIS_FACE_INFO,pszToolMenu,"分析","FACE结构数据(&F)");
	AddCommandItem((int)CMD_ANALYSIS_FACE_WIREORI,pszToolMenu,"分析","FACE环方向(&F)");
	AddCommandItem((int)CMD_ANALYSIS_BAT_STEPFILE,pszToolMenu,"分析","批量分析STEP文件(&B)");

	//分析
	AddCommandItem((int)CMD_ANALYSIS_NBS_POLES,pszAnalysisMenu,"显示控制点(&P)");
	AddCommandItem((int)CMD_ANALYSIS_CURVE_CURVATURE,pszAnalysisMenu,"曲线曲率(&C)");
	AddCommandItem((int)CMD_ANALYSIS_MESH_DISPLAY,pszAnalysisMenu,"曲面网格显示(&M)");
	AddCommandItem((int)CMD_ANALYSIS_FACE_UV_ORIENT,pszAnalysisMenu,"曲面UV方向(&O)");
	AddCommandItem((int)CMD_ANALYSIS_FACE_NORMAL,pszAnalysisMenu,"Face法线(&N)");
	AddCommandItem((int)CMD_ANALYSIS_SHAPE_NORMAL,pszAnalysisMenu,"Shape法线(&N)");

	AddCommandItem((int)CMD_DBGTOOL_INPUT_FNL,pszToolMenu,"输入面编号");
	AddCommandItem((int)CMD_TEST_FOR_DEBUG,pszToolMenu,"调试命令");

	return TRUE;
}

kiCommand*  kuiSyscmdPlugin::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_DISP_HIDESEL:		pCommand = new kcmDispHideSelect; break;
	case CMD_DISP_SHOWALL:		pCommand = new kcmDispShowAll; break;
	case CMD_DISP_TYPE_FILLTER:	pCommand = new kcmDispTypeFilter; break;
	case CMD_DISP_HIDE_GRID:		pCommand = new kcmHideGridDisplay; break;
	case CMD_ANALYSIS_SHAPE_NORMAL:	pCommand = new kcmShapeAnalysisNormal; break;
	case CMD_ANALYSIS_FACE_NORMAL:	pCommand = new kcmFaceNormalAnalysis; break;
	case CMD_ANALYSIS_FACE_ORIENT:	pCommand = new kcmShapeAnalysisFaceOrient; break;
	case CMD_ANALYSIS_FACE_UV_ORIENT:	pCommand = new kcmFaceOrientAnalysis; break;
	case CMD_ANALYSIS_FACE_INFO:	pCommand = new kiFacePropInfo; break;
	case CMD_ANALYSIS_FACE_WIREORI:	pCommand = new kiFaceWireOri; break;
	case CMD_ANALYSIS_BAT_STEPFILE:	pCommand = new kiBatAnalyze; break;
	case CMD_ANALYSIS_NBS_POLES:	pCommand = new kiDisplayNbsPoles; break;
	case CMD_ANALYSIS_CURVE_CURVATURE:	pCommand = new kcmCurveCurvature; break;
	case CMD_ANALYSIS_MESH_DISPLAY:	pCommand = new kcmMeshDisplay; break;
	case CMD_DBGTOOL_INPUT_FNL:	pCommand = new kcmInputFaceNoLF; break;
	case CMD_TEST_FOR_DEBUG:	pCommand = new kcmTestForDebug; break;
	default:
		break;
	}
	return pCommand;
}

bool  kuiSyscmdPlugin::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}