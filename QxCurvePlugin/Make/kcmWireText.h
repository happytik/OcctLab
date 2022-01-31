//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_WIRE_TEXT_H_
#define _KCM_WIRE_TEXT_H_

#include "kiCommand.h"

class kiInputPoint;

class kcmWireText : public kiCommand
{
public:
	kcmWireText(void);
	virtual ~kcmWireText(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();

	//
protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					MakeWireText(const char *lpszText,LOGFONT& lf,double dHeight,double dGap,BOOL bWire);

protected:
	kPoint3					m_aPoint;
	kiInputPoint			*m_pInputPoint;

	TopTools_SequenceOfShape m_aSeqOfShape;
};

#endif