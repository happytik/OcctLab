//对象统计
#ifndef _KCM_SHAPE_STATISTIC_H_
#define _KCM_SHAPE_STATISTIC_H_

#include "kiCommand.h"

class kiOptionItem;
class kiOptionSet;

class kcmShapeStatistic : public kiCommand{
public:
	kcmShapeStatistic();
	virtual ~kcmShapeStatistic();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

public:
	//选项被选中了
	virtual void			OnOptionSelected(kiOptionItem *pOptItem);

protected:
	virtual int			OnExecute();

protected:
	void					DoCollectSurfTypeInfo();
	void					DoCollectCurveTypeInfo();
	void					DoCountSurfInfo();
	//
	void					DoGetOrientStat();

protected:
	// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
	virtual	BOOL			CreateInputTools();
	// 销毁创建的输入工具.每个命令仅调用一次.
	virtual BOOL			DestroyInputTools();
	// 在begin中调用，主要是初始化工具队列。
	virtual BOOL			InitInputToolQueue();

protected:
	kiOptionSet			*_pOptionSet;
};

#endif