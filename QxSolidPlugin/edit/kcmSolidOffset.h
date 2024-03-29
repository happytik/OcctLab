//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_OFFSET_H_
#define _KCM_SOLID_OFFSET_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputOptionsTool;
class kcDispSolidArrow;
class kiOptionItem;

class kcmSolidOffset : public kiCommand
{
public:
	kcmSolidOffset(int subCmd);
	virtual ~kcmSolidOffset(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	// 当选项改变后的调用函数
	virtual void			OnOptionChanged(kiOptionItem *pOptionItem);
protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	// 具体的应用处理函数
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					BuildOffsetShape();

protected:
	int						nSubCmd_; //子命令 0:Face 1:SHELL 2:SOLID
	kiOptionSet				*pOptionSet_; 
	kiOptionItem			*pSideOptItem_;
	
	kiInputEntityTool		*pInputEntity_;
	kiInputOptionsTool		*pOptionsTool_;

	double					dOffset_;//偏移值
	int						nOffsetMethod_;//偏移方法
	bool					bOutSide_;//向外还是向内
	int						nJoinType_;//
};

#endif