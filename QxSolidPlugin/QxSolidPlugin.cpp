//
#include "stdafx.h"
#include "QxCommandItem.h"
#include "kiCommand.h"
#include "kcmSolidSphere.h"
#include "kcmSolidBox.h"
#include "kcmSolidCylinder.h"
#include "kcmSolidCone.h"
#include "kcmSolidDeCompound.h"
#include "kcmSolidBoolean.h"
#include "kcmSolidPrism.h"
#include "kcmSolidRevol.h"
#include "kcmSolidPipe.h"
#include "kcmSolidPipeShell.h"
#include "kcmSolidFillet.h"
#include "kcmSolidSewing.h"
#include "kcmSolidOffset.h"
#include "kcmThickSolid.h"
#include "kcmSolidLoft.h"
#include "kcmSolidDraft.h"
#include "kcmSolidTrim.h"
#include "kcmOneFaceBase.h"
#include "QxSolidPlugin.h"

//派生一个自己的类
class QxSolidPluginImpl : public QxCmdPluginImpl{
public:
	QxSolidPluginImpl();
	~QxSolidPluginImpl();

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
	QxSolidPluginImpl *pImpl = new QxSolidPluginImpl;
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
QxSolidPluginImpl::QxSolidPluginImpl()
{
	eTopMenuID_ = QX_TOP_MENU_SOLID;
}

QxSolidPluginImpl::~QxSolidPluginImpl()
{
}

// 获取插件中的命令个数
int QxSolidPluginImpl::GetCommandNum() const
{
	return 40;
}

// 获取插件的名称
const char* QxSolidPluginImpl::GetPluginName() const
{
	return "Occt实体造型插件";
}

enum eCmdPluginCmdType{
	CMD_SOLID_BOX = 0,
	CMD_SOLID_SPHERE,
	CMD_SOLID_CYLINDER,
	CMD_SOLID_CONE,
	CMD_ID_BOOL_FUSE,
	CMD_ID_BOOL_COMMON,
	CMD_ID_BOOL_CUT,
	CMD_ID_BOOL_SPLIT,
	CMD_SOLID_DECOMP,
	CMD_SOLID_PRISM,
	CMD_SOLID_DRAFT,
	CMD_SOLID_REVOL,
	CMD_SOLID_PIPE,
	CMD_SOLID_PIPE_SHELL,
	CMD_SOLID_SWEEP,
	CMD_SOLID_SWEEP_SHELL,
	CMD_SOLID_LOFT,
	CMD_SOLID_BOOL_SPLITTER,
	CMD_SOLID_TRIM_BYSHELL,
	CMD_SOLID_TRIM_SPLIT,
	CMD_SOLID_SEWING,
	CMD_SOLID_OFFSET,
	CMD_SOLID_SHEET,
	CMD_SOLID_THICK,
	CMD_SOLID_FILLET,
	CMD_SOLID_FILLET_ALL,
	CMD_SOLID_FACE_COPY,
	CMD_SOLID_FACE_LOOP,
	CMD_SOLID_FACE_INTERP,
	CMD_SOLID_FACE_EXPSTEP,
	CMD_SOLID_EXTRACT_NFACE,
	CMD_SOLID_FACE_PSTONP,
	CMD_SOLID_FACE_CHGUORG,
	CMD_SOLID_FACE_CHGVORG,
};

// 插件初始化和释放
bool QxSolidPluginImpl::Initialize()
{
	//const char *pszTopMenu = "实体(&D)";

	DoAddCommandItem((int)CMD_SOLID_BOX,"立方体(&B)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_SPHERE,"球(&S)");
	DoAddCommandItem((int)CMD_SOLID_CYLINDER,"圆柱体(&C)");
	DoAddCommandItem((int)CMD_SOLID_CONE,"锥体(&C)");

	DoAddCommandItem((int)CMD_ID_BOOL_FUSE,"组合","并集(&F)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_ID_BOOL_COMMON,"组合","交集(&J)");
	DoAddCommandItem((int)CMD_ID_BOOL_CUT,"组合","差集(&C)");
	DoAddCommandItem((int)CMD_ID_BOOL_SPLIT,"组合","分割实体(&S)");

	DoAddCommandItem((int)CMD_SOLID_PRISM,"拉伸(&P)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_DRAFT,"拔模拉伸(&E)");
	DoAddCommandItem((int)CMD_SOLID_REVOL,"旋转(&R)");
	DoAddCommandItem((int)CMD_SOLID_PIPE,"管道","连续(&C)");
	DoAddCommandItem((int)CMD_SOLID_PIPE_SHELL,"管道","不连续(&D)");
	DoAddCommandItem((int)CMD_SOLID_SWEEP,"单轨扫略","连续(&C)");
	DoAddCommandItem((int)CMD_SOLID_SWEEP_SHELL,"单轨扫略","不连续(&D)");
	DoAddCommandItem((int)CMD_SOLID_LOFT,"放样(&L)");

	DoAddCommandItem((int)CMD_SOLID_DECOMP,"分解(&D)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_TRIM_BYSHELL,"裁剪","使用面裁剪(&S)");
	DoAddCommandItem((int)CMD_SOLID_TRIM_SPLIT,"裁剪","分割裁剪(&P)");

	DoAddCommandItem((int)CMD_SOLID_SEWING,"缝合曲面(&S)");
	DoAddCommandItem((int)CMD_SOLID_OFFSET,"偏移(&O)");
	DoAddCommandItem((int)CMD_SOLID_SHEET,"抽壳(&S)");
	DoAddCommandItem((int)CMD_SOLID_THICK,"等厚(&T)");

	DoAddCommandItem((int)CMD_SOLID_FILLET,"倒角(&F)","等半径倒圆角(&F)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_FILLET_ALL,"倒角(&F)","全部边倒圆角(&F)");

	DoAddCommandItem((int)CMD_SOLID_FACE_COPY,"基于面","面拷贝(&C)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_FACE_LOOP,"基于面","面参数域环(&C)");
	DoAddCommandItem((int)CMD_SOLID_FACE_INTERP,"基于面","插值面(&I)");
	DoAddCommandItem((int)CMD_SOLID_FACE_EXPSTEP,"基于面","导出面到STEP");
	DoAddCommandItem((int)CMD_SOLID_EXTRACT_NFACE,"基于面","提前第n个面(&N)");

	DoAddCommandItem((int)CMD_SOLID_FACE_PSTONP,"周期样条曲面","转非周期(&N)");
	DoAddCommandItem((int)CMD_SOLID_FACE_CHGUORG,"周期样条曲面","改变U向起点(&U)");
	DoAddCommandItem((int)CMD_SOLID_FACE_CHGVORG,"周期样条曲面","改变V向起点(&V)");

	return true;
}

void QxSolidPluginImpl::Release()
{
}

// 根据命令在插件中的ID，创建命令对象
kiCommand* QxSolidPluginImpl::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_SOLID_BOX:		pCommand = new kcmSolidBox;  break;
	case CMD_SOLID_SPHERE:		pCommand = new kcmSolidSphere; break;
	case CMD_SOLID_CYLINDER:	pCommand = new kcmSolidCylinder; break;
	case CMD_SOLID_CONE:		pCommand = new kcmSolidCone; break;
	case CMD_ID_BOOL_FUSE:	pCommand = new kcmSolidBoolFuse; break;
	case CMD_ID_BOOL_COMMON:	pCommand = new kcmSolidBoolCommon; break;
	case CMD_ID_BOOL_CUT:	pCommand = new kcmSolidBoolCut; break;
	case CMD_ID_BOOL_SPLIT:	pCommand = new kcmSolidSplit; break;
	case CMD_SOLID_DECOMP:		pCommand = new kcmSolidDeCompound; break;
	case CMD_SOLID_PRISM:		pCommand = new kcmSolidPrism; break;
	case CMD_SOLID_DRAFT:		pCommand = new kcmSolidDraft; break;
	case CMD_SOLID_REVOL:		pCommand = new kcmSolidRevol; break;
	case CMD_SOLID_PIPE:		pCommand = new kcmSolidPipe; break;
	case CMD_SOLID_PIPE_SHELL: pCommand = new kcmSolidPipeShell; break;
	case CMD_SOLID_SWEEP:		pCommand = new kcmSolidSweep; break;
	case CMD_SOLID_SWEEP_SHELL:	pCommand = new kcmSolidSweepShell; break;
	case CMD_SOLID_LOFT:		pCommand = new kcmSolidLoft; break;
	case CMD_SOLID_TRIM_BYSHELL:	pCommand = new kcmSolidTrimByShell; break;
	case CMD_SOLID_TRIM_SPLIT:		pCommand = new kcmSolidTrimSplit; break;
	case CMD_SOLID_SEWING:		pCommand = new kcmSolidSewing; break;
	case CMD_SOLID_OFFSET:		pCommand = new kcmSolidOffset; break;
	case CMD_SOLID_SHEET:		pCommand = new kcmSheetSolid; break;
	case CMD_SOLID_THICK:		pCommand = new kcmThickSolid; break;
	case CMD_SOLID_FILLET:		pCommand = new kcmSolidFillet; break;
	case CMD_SOLID_FILLET_ALL:		pCommand = new kcmSolidFilletAllEdge; break;
	case CMD_SOLID_FACE_COPY:	pCommand = new kcmOneFaceCopy; break;
	case CMD_SOLID_FACE_LOOP:	pCommand = new kcmOneFaceParLoop; break;
	case CMD_SOLID_FACE_INTERP: pCommand = new kcmInterpOneFace; break;
	case CMD_SOLID_FACE_EXPSTEP:	pCommand = new kcmExpOneFaceToStep; break;
	case CMD_SOLID_EXTRACT_NFACE:	pCommand = new kcmExtractNFace; break;
	case CMD_SOLID_FACE_PSTONP:	pCommand = new kcmPeriodicBSpSurfTry(1); break;
	case CMD_SOLID_FACE_CHGUORG:	pCommand = new kcmPeriodicBSpSurfTry(2); break;
	case CMD_SOLID_FACE_CHGVORG:	pCommand = new kcmPeriodicBSpSurfTry(3); break;
	default:
		break;
	}
	return pCommand;
}

// 释放创建的命令对象
bool QxSolidPluginImpl::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}