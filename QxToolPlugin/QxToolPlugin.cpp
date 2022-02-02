//
#include "stdafx.h"
#include "kcmDispColor.h"
#include "kcmDispShadingModel.h"
#include "kcmShapeFix.h"
#include "kcmBasePlane.h"
#include "kcmShapeCheck.h"
#include "QxCommandItem.h"
#include "kcmShapeProperty.h"
#include "kcmShapeStatistic.h"
#include "kcmAnimationDemo.h"
#include "kcmLengthDimension.h"
#include "kcmDiameterDim.h"
#include "kcmRadiusDim.h"
#include "kcmAngleDim.h"
#include "QxToolPlugin.h"

//派生一个自己的类
class QxToolPluginImpl : public QxCmdPluginImpl{
public:
	QxToolPluginImpl();
	~QxToolPluginImpl();

	// 获取插件中的命令个数
	virtual int		GetCommandNum() const;
	// 获取插件的名称
	virtual const char*	GetPluginName() const;

	// 插件初始化和释放
	virtual bool		Initialize();
	virtual void		Release();

	// 根据命令在插件中的ID，创建命令对象
	virtual kiCommand*			DoCreateCommand(int nLocalID);
	// 释放创建的命令对象
	virtual bool					DoReleaseCommand(kiCommand *pCommand);
};

QxCmdPluginImpl* QxCmdPluginMain(QxCmdPluginCfg *pCfg)
{
	QxToolPluginImpl *pImpl = new QxToolPluginImpl;
	return pImpl;
}

//
void QxCmdPluginFree(QxCmdPluginImpl *pImpl)
{
	if(pImpl){
		delete pImpl;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
QxToolPluginImpl::QxToolPluginImpl()
{
}

QxToolPluginImpl::~QxToolPluginImpl()
{
}

// 获取插件中的命令个数
int QxToolPluginImpl::GetCommandNum() const
{
	return 30;
}

// 获取插件的名称
const char* QxToolPluginImpl::GetPluginName() const
{
	return "Occt工具插件";
}

enum eCmdPluginCmdType{
	CMD_DISP_COLOR = 0,
	CMD_DISP_WIREFRM,
	CMD_DISP_SHADING,
	CMD_BPLN_OFFSET,
	CMD_BPLN_THREEPNT,
	CMD_BPLN_FACE,
	CMD_ID_CHECK_SHAPE_VALID,
	CMD_ID_CHECK_TOPOL_VALID,
	CMD_ID_CHECK_SHAPE_CLOSED,
	CMD_ID_SHAPE_FIX,
	CMD_STAT_SHAPE,
	CMD_SHAPE_PROPERTY,
	CMD_EDGE_PROPERTY,
	CMD_FACE_PROPERTY,
	CMD_ANIMA_DEMO,
	CMD_LENGTH_DIM,
	CMD_DIAMETER_DIM,
	CMD_RADIUS_DIM,
	CMD_ANGGLE_DIM_TWO_LINE,
};

// 插件初始化和释放
bool QxToolPluginImpl::Initialize()
{
	const char *pszDispMenu = "显示(&D)";
	const char *pszToolMenu = "工具(&T)";
	const char *pszViewMenu = "视图(&V)";
	
	AddCommandItem((int)CMD_DISP_COLOR,pszDispMenu,"颜色(&C)",KUI_MENU_BEFORE_SEP);
	AddCommandItem((int)CMD_DISP_WIREFRM,pszDispMenu,"框架显示(&W)");
	AddCommandItem((int)CMD_DISP_SHADING,pszDispMenu,"渲染显示(&S)");

	//基准面
	AddCommandItem((int)CMD_BPLN_FACE,pszViewMenu,"基准面","基于Face(&F)");
	AddCommandItem((int)CMD_BPLN_OFFSET,pszViewMenu,"基准面","偏移(&O)",KUI_MENU_BEFORE_SEP);
	AddCommandItem((int)CMD_BPLN_THREEPNT,pszViewMenu,"基准面","三点(&T)");
	
	////检查
	AddCommandItem((int)CMD_ID_CHECK_SHAPE_VALID,pszToolMenu,"检查","对象有效性(&V)",KUI_MENU_BEFORE_SEP);
	AddCommandItem((int)CMD_ID_CHECK_TOPOL_VALID,pszToolMenu,"检查","拓扑有效性(&T)");
	AddCommandItem((int)CMD_ID_CHECK_SHAPE_CLOSED,pszToolMenu,"检查","闭合性(&C)");

	AddCommandItem((int)CMD_ID_SHAPE_FIX,pszToolMenu,"修正","整体修正(&O)");

	AddCommandItem((int)CMD_STAT_SHAPE,pszToolMenu,"统计(&C)");
	AddCommandItem((int)CMD_SHAPE_PROPERTY,pszToolMenu,"对象属性(&P)",KUI_MENU_BEFORE_SEP);
	AddCommandItem((int)CMD_EDGE_PROPERTY,pszToolMenu,"边属性(&E)");
	AddCommandItem((int)CMD_FACE_PROPERTY,pszToolMenu,"面属性(&F)");

	AddCommandItem((int)CMD_ANIMA_DEMO,pszToolMenu,"动画演示(&T)",KUI_MENU_BEFORE_SEP);

	AddCommandItem((int)CMD_LENGTH_DIM,pszToolMenu,"标注","线性标注(&L)");
	AddCommandItem((int)CMD_DIAMETER_DIM,pszToolMenu, "标注", "直径标注(&D)");
	AddCommandItem((int)CMD_RADIUS_DIM, pszToolMenu, "标注", "半径标注(&R)");
	AddCommandItem((int)CMD_ANGGLE_DIM_TWO_LINE, pszToolMenu, "标注", "两线角度标注(&R)");

	return true;
}

void QxToolPluginImpl::Release()
{
}

// 根据命令在插件中的ID，创建命令对象
kiCommand* QxToolPluginImpl::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_DISP_COLOR:		pCommand = new kcmDispColor; break;
	case CMD_DISP_WIREFRM:		pCommand = new kcmDispWireFrame; break;
	case CMD_DISP_SHADING:		pCommand = new kcmDispShading; break;
	case CMD_ID_CHECK_SHAPE_VALID:	pCommand = new kcmShapeCheckValid; break;
	case CMD_ID_CHECK_TOPOL_VALID:	pCommand = new kcmShapeCheckTopoValid; break;
	case CMD_ID_CHECK_SHAPE_CLOSED:	pCommand = new kcmShapeCheckClose; break;
	case CMD_BPLN_OFFSET:		pCommand = new kcmOffsetBasePlane; break;
	case CMD_BPLN_THREEPNT:	pCommand = new kcmThreePntBasePlane; break;
	case CMD_BPLN_FACE:	pCommand = new kcmBasePlaneByFace; break;
	case CMD_ID_SHAPE_FIX:	pCommand = new kcmShapeFix; break;
	case CMD_STAT_SHAPE:		pCommand = new kcmShapeStatistic; break;
	case CMD_SHAPE_PROPERTY:	pCommand = new kcmShapeProperty; break;
	case CMD_EDGE_PROPERTY:	pCommand = new kcmEdgeProperty; break;
	case CMD_FACE_PROPERTY:	pCommand = new kcmFaceProperty; break;
	case CMD_ANIMA_DEMO:	pCommand = new kcmAnimationDemo; break;
	case CMD_LENGTH_DIM:	pCommand = new kcmLengthDimension(0); break;
	case CMD_DIAMETER_DIM:	pCommand = new kcmDiameterDim; break;
	case CMD_RADIUS_DIM:	pCommand = new kcmRadiusDim; break;
	case CMD_ANGGLE_DIM_TWO_LINE:	pCommand = new kcmAngleDim(0); break;
	default:
		break;
	}
	return pCommand;
}

// 释放创建的命令对象
bool QxToolPluginImpl::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}