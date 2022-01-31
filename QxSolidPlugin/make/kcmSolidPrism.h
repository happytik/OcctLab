//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_PRISM_H_
#define _KCM_SOLID_PRISM_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputLength;
class kiInputDoubleTool;
class kiOptionSet;

class kcmSolidPrism : public kiCommand
{
public:
	kcmSolidPrism(void);
	virtual ~kcmSolidPrism(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);


protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

public://IOptionCallBack
	//选项改变了
	virtual void				OnOptionChanged(kiOptionItem *pOptItem);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					MakePrismShape();

protected:
	kiOptionSet			*pFaceOptionSet_;
	kiInputEntityTool		*pFacePickTool_;
	kiOptionSet			*pMainOptionSet_;
	kiInputLength			*pInputLenTool_;

	double				dExtrudeLen_;
	int					nDirFlag_; 
};

#endif