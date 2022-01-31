//
#include "stdafx.h"
#include "QxCommandItem.h"
#include "kiCommand.h"
#include "QxCommandItem.h"
#include "kcmPoint.h"
#include "kcmLine.h"
#include "kcmCircle.h"
#include "kcmArc.h"
#include "kcmSpline.h"
#include "kcmCurveOffset.h"
#include "kcmRect.h"
#include "kcmCurvePolyline.h"
#include "kcmInterCurve.h"
#include "kcmCurveBreak.h"
#include "kcmCurveTrim.h"
#include "kcmFaceCurve.h"
#include "kcmCurveFillet.h"
#include "kcmNormProjCurve.h"
#include "kcmCurveBlend.h"
#include "kcmSewCurve.h"
#include "kcmWireText.h"
#include "QxCurvePlugin.h"

//派生一个自己的类
class QxCurvePluginImpl : public QxCmdPluginImpl{
public:
	QxCurvePluginImpl();
	~QxCurvePluginImpl();

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

//

QxCmdPluginImpl* QxCmdPluginMain(QxCmdPluginCfg *pCfg)
{
	QxCurvePluginImpl *pImpl = new QxCurvePluginImpl;
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
QxCurvePluginImpl::QxCurvePluginImpl()
{
	eTopMenuID_ = QX_TOP_MENU_CURVE;
}

QxCurvePluginImpl::~QxCurvePluginImpl()
{
}

// 获取插件中的命令个数
int QxCurvePluginImpl::GetCommandNum() const
{
	return 30;
}

// 获取插件的名称
const char* QxCurvePluginImpl::GetPluginName() const
{
	return "Occt曲线造型插件";
}

enum eCmdPluginCmdType{
	CMD_CRV_POINT = 0,
	CMD_CRV_INTER_POINT,
	CMD_CRV_LINE,
	CMD_CRV_CIRCLE,
	CMD_CRV_ARC,
	CMD_CRV_SPLINE,
	CMD_CRV_UP_SPLINE,
	CMD_CRV_RECT,
	CMD_CRV_POLYLINE,
	CMD_CRV_INTER_CRV,
	CMD_CRV_FACE_BND_EDGE,
	CMD_CRV_FACE_BND_WIRE,
	CMD_CRV_FACE_RAW_WIRE,
	CMD_CRV_FACE_BND_EDGEDIR,
	CMD_CRV_FACE_ISO_CURVE,
	CMD_CRV_NORM_PRJ_CRV,
	CMD_CRV_BREAK_BY_POINT,
	CMD_CRV_BREAK_BY_CURVE,
	CMD_CRV_TRIM,
	CMD_CRV_TRIM_QUICK,
	CMD_CRV_SPLIT_ALL_EDGES,
	CMD_CRV_FILLET,
	CMD_CRV_FILLET_TWO_EDGE,
	CMD_CRV_OFFSET,
	CMD_CRV_BLEND,
	CMD_ID_SEWING_CURVE,
	CMD_CRV_TEXT
};

// 插件初始化和释放
bool QxCurvePluginImpl::Initialize()
{

	DoAddCommandItem((int)CMD_CRV_POINT,"点(&P)","位置点(&P",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_CRV_INTER_POINT,"点(&P)","曲线交点(&I");
	DoAddCommandItem((int)CMD_CRV_LINE,"直线(&L)");
	DoAddCommandItem((int)CMD_CRV_CIRCLE,"圆(&C)");
	DoAddCommandItem((int)CMD_CRV_ARC,"圆弧(&A)");
	DoAddCommandItem((int)CMD_CRV_SPLINE,"样条曲线","创建(&B)");
	DoAddCommandItem((int)CMD_CRV_UP_SPLINE,"样条曲线","非周期化(&U)");
	DoAddCommandItem((int)CMD_CRV_RECT,"矩形(&R)");
	DoAddCommandItem((int)CMD_CRV_POLYLINE,"多段线(&P)");
	DoAddCommandItem((int)CMD_CRV_INTER_CRV,"交线(&I)",KUI_MENU_BEFORE_SEP);
	
	DoAddCommandItem((int)CMD_CRV_FACE_BND_EDGE,"曲面上线","边界曲线(&E)");
	DoAddCommandItem((int)CMD_CRV_FACE_BND_WIRE,"曲面上线","边界环(&E)");
	DoAddCommandItem((int)CMD_CRV_FACE_RAW_WIRE,"曲面上线","原始顺序边界环(&E)");
	DoAddCommandItem((int)CMD_CRV_FACE_BND_EDGEDIR,"曲面上线","曲面边界方向(&D)");
	DoAddCommandItem((int)CMD_CRV_FACE_ISO_CURVE,"曲面上线","曲面流线(&I)");

	DoAddCommandItem((int)CMD_CRV_NORM_PRJ_CRV,"投影","投影到面(&P)");
	DoAddCommandItem((int)CMD_CRV_BREAK_BY_POINT,"打断","使用点(&P)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_CRV_BREAK_BY_CURVE,"打断","使用曲线(&C)");
	DoAddCommandItem((int)CMD_CRV_TRIM,"裁剪","曲线裁剪(&C)");
	DoAddCommandItem((int)CMD_CRV_TRIM_QUICK,"裁剪","快速裁剪(&C)");
	DoAddCommandItem((int)CMD_CRV_SPLIT_ALL_EDGES,"裁剪","全部分割(&S)");

	DoAddCommandItem((int)CMD_CRV_FILLET,"倒角","倒圆角(&F)");
	DoAddCommandItem((int)CMD_CRV_FILLET_TWO_EDGE,"倒角","两边倒圆角(&T)");
	DoAddCommandItem((int)CMD_CRV_OFFSET,"曲线偏移(&O)");
	DoAddCommandItem((int)CMD_CRV_BLEND,"曲线混接(&B)");

	DoAddCommandItem((int)CMD_ID_SEWING_CURVE,"组合曲线(&W)");
	
	DoAddCommandItem((int)CMD_CRV_TEXT,"文本(&T)",KUI_MENU_BEFORE_SEP);


	return true;
}

void QxCurvePluginImpl::Release()
{
}

// 根据命令在插件中的ID，创建命令对象
kiCommand* QxCurvePluginImpl::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_CRV_POINT:		pCommand = new kcmPoint; break;
	case CMD_CRV_INTER_POINT:	pCommand = new kcmCurveInterPoint;	break;
	case CMD_CRV_LINE:		pCommand = new kcmLine;	break;
	case CMD_CRV_CIRCLE:		pCommand = new kcmCircle;	break;
	case CMD_CRV_ARC:			pCommand = new kcmArc; break;
	case CMD_CRV_SPLINE:		pCommand = new kcmSpline; break;
	case CMD_CRV_UP_SPLINE:	pCommand = new kcmSplineUnperiodize; break;
	case CMD_CRV_RECT:		pCommand = new kcmRect; break;
	case CMD_CRV_POLYLINE:		pCommand = new kcmCurvePolyline; break;
	case CMD_CRV_INTER_CRV:	pCommand = new kcmInterCurve; break;
	case CMD_CRV_FACE_BND_EDGE:	pCommand = new kcmFaceBoundEdge; break;
	case CMD_CRV_FACE_BND_WIRE:	pCommand = new kcmFaceBoundWire; break;
	case CMD_CRV_FACE_RAW_WIRE:	pCommand = new kcmFaceRawWire; break;
	case CMD_CRV_FACE_BND_EDGEDIR:	pCommand = new kcmFaceBoundEdgeDire; break;
	case CMD_CRV_FACE_ISO_CURVE:	pCommand = new kcmFaceIsoCurve; break;
	case CMD_CRV_NORM_PRJ_CRV:		pCommand = new kcmNormProjCurve; break;
	case CMD_CRV_BREAK_BY_POINT:	pCommand = new kcmCurveBreakByPoint; break;
	case CMD_CRV_BREAK_BY_CURVE:	pCommand = new kcmCurveBreakByCurve; break;
	case CMD_CRV_TRIM:	pCommand = new kcmCurveTrim; break;
	case CMD_CRV_TRIM_QUICK:	pCommand = new kcmCurveQuickTrim; break;
	case CMD_CRV_SPLIT_ALL_EDGES:	pCommand = new kcmSplitAllEdges; break;
	case CMD_CRV_FILLET:	pCommand = new kcmCurveFillet; break;
	case CMD_CRV_FILLET_TWO_EDGE:	pCommand = new kcmTwoEdgeFillet; break;
	case CMD_CRV_OFFSET:	pCommand = new kcmCurveOffset; break;
	case CMD_CRV_BLEND:	pCommand = new kcmCurveBlend; break;
	case CMD_ID_SEWING_CURVE: pCommand = new kcmSewCurve; break;
	case CMD_CRV_TEXT:	pCommand = new kcmWireText; break;
	default:
		break;
	}
	return pCommand;
}

// 释放创建的命令对象
bool QxCurvePluginImpl::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}