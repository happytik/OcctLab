//2021.12.20
#include "stdafx.h"
#include "QxCommandItem.h"
#include "kiCommand.h"
#include "kcmPlane.h"
#include "kcmRevolSurface.h"
#include "kcmPrismSurface.h"
#include "kcmLoopToFace.h"
#include "kcmLoftSurface.h"
#include "kcmSurfPolygon.h"
#include "kcmRegionBoolean.h"
#include "kcmRuledSurface.h"
#include "kcmSurfaceFilling.h"
#include "kcmSurfacePipe.h"
#include "kcmSurfaceSplit.h"
#include "kcmSurfaceTool.h"
#include "QxSurfPlugin.h"

//派生一个自己的类
class QxSurfPluginImpl : public QxCmdPluginImpl{
public:
	QxSurfPluginImpl();
	~QxSurfPluginImpl();

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
	QxSurfPluginImpl *pImpl = new QxSurfPluginImpl;
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
QxSurfPluginImpl::QxSurfPluginImpl()
{
	eTopMenuID_ = QX_TOP_MENU_SURFACE;
}

QxSurfPluginImpl::~QxSurfPluginImpl()
{
}

// 获取插件中的命令个数
int QxSurfPluginImpl::GetCommandNum() const
{
	return 30;
}

// 获取插件的名称
const char* QxSurfPluginImpl::GetPluginName() const
{
	return "Occt曲面造型插件";
}

enum eCmdPluginCmdType{
	CMD_SURF_PLANE = 0,
	CMD_SURF_PLN_POLYGON,
	CMD_SURF_PLN_LOOP_TO_FACE,
	CMD_SURF_REV_SURF,
	CMD_SURF_PRISM,
	CMD_SURF_LOFT,
	CMD_SURF_RULED,
	CMD_SURF_PIPE,
	CMD_SURF_SWEEP1,
	CMD_SURF_SWEEPN,
	CMD_SURF_SPLIT,
	CMD_SURF_FILLING,
	CMD_SURF_PRIMARY,
	CMD_REGION_BOOL_FUSE,
	CMD_REGION_BOOL_COMMON,
	CMD_REGION_BOOL_CUT,
	CMD_HELICOID_ARCHIMEDES,
	CMD_HELICOID_SCREWINVSURF,
	CMD_HELICOID_ARCHIMEDES_ISOCRV,
};

// 插件初始化和释放
bool QxSurfPluginImpl::Initialize()
{
	//const char *pszTopMenu = "曲面(&S)";

	DoAddCommandItem((int)CMD_SURF_PLANE,"平面(&P)","两点平面(&T)");
	DoAddCommandItem((int)CMD_SURF_PLN_POLYGON,"平面(&P)","多边形(&P)");
	DoAddCommandItem((int)CMD_SURF_PLN_LOOP_TO_FACE,"平面(&P)","闭合环生成平面(&P)");
	DoAddCommandItem((int)CMD_SURF_REV_SURF,"旋转面(&R)");
	DoAddCommandItem((int)CMD_SURF_PRISM,"拉伸面(&R)");

	DoAddCommandItem((int)CMD_SURF_LOFT,"蒙面(&R)");
	DoAddCommandItem((int)CMD_SURF_RULED,"直纹面(&L)");
	DoAddCommandItem((int)CMD_SURF_PIPE,"管道面(&P)");
	DoAddCommandItem((int)CMD_SURF_SWEEP1,"扫掠面","单轨(&S)");
	DoAddCommandItem((int)CMD_SURF_SWEEPN,"扫掠面","多截面(&M)");
	DoAddCommandItem((int)CMD_SURF_SPLIT,"分割曲面(&S)");

	DoAddCommandItem((int)CMD_SURF_FILLING,"补面(&F)");

	DoAddCommandItem((int)CMD_SURF_PRIMARY,"原始面(&P)",KUI_MENU_BEFORE_SEP);

	DoAddCommandItem((int)CMD_REGION_BOOL_FUSE,"区域","区域并集(&U)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_REGION_BOOL_COMMON,"区域","区域交集(&C)");
	DoAddCommandItem((int)CMD_REGION_BOOL_CUT,"区域","区域差集(&D)");
		
	return true;
}

void QxSurfPluginImpl::Release()
{
}

// 根据命令在插件中的ID，创建命令对象
kiCommand* QxSurfPluginImpl::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_SURF_PLANE:			pCommand = new kcmPlane; break;
	case CMD_SURF_PLN_POLYGON:		pCommand = new kcmSurfPolygon; break;
	case CMD_SURF_PLN_LOOP_TO_FACE:		pCommand = new kcmLoopToFace; break;
	case CMD_SURF_REV_SURF:		pCommand = new kcmRevolSurface; break;
	case CMD_SURF_PRISM:			pCommand = new kcmPrismSurface; break;
	case CMD_SURF_LOFT:			pCommand = new kcmLoftSurface; break;
	case CMD_SURF_RULED:			pCommand = new kcmRuledSurface; break;
	case CMD_SURF_PIPE:			pCommand = new kcmSurfacePipe; break;
	case CMD_SURF_SWEEP1:			pCommand = new kcmSurfaceSweep1; break;
	case CMD_SURF_SWEEPN:			pCommand = new kcmSurfaceSweepN; break;
	case CMD_SURF_SPLIT:			pCommand = new kcmSurfaceSplit; break;
	case CMD_SURF_FILLING:			pCommand = new kcmSurfaceFilling; break;
	case CMD_SURF_PRIMARY:			pCommand = new kcmPrimarySurface; break;
	case CMD_REGION_BOOL_FUSE:		pCommand = new kcmRegionBoolFuse; break;
	case CMD_REGION_BOOL_COMMON:	pCommand = new kcmRegionBoolCommon; break;
	case CMD_REGION_BOOL_CUT:		pCommand = new kcmRegionBoolCut; break;
	default:
		break;
	}
	return pCommand;
}

// 释放创建的命令对象
bool QxSurfPluginImpl::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}