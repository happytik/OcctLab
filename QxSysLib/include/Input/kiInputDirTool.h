///输入一个方向
#ifndef _KI_INPUT_DIR_TOOL_H_
#define _KI_INPUT_DIR_TOOL_H_

#include "kPoint.h"
#include "kVector.h"
#include "kiInputTool.h"
#include "kiOptionSet.h"

class kiInputPoint;

class QXLIB_API kiInputDirTool : public kiInputTool
{
public:
	// 用于命令中的构造函数。
	kiInputDirTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	// 用于InputTool中的构造函数。
	kiInputDirTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputDirTool(void);

	// 初始化.bUseWCS:是否使用全局坐标系，false为使用基准平面的局部坐标系
	// bOnlyZDir:是否只包含Z向
	int						Initialize(kVector3 *pDir,bool bUseWCS,bool bOnlyZDir,bool bUnited);

protected:
	// 当选项改变后的调用函数
	virtual void				OnToolOptionChanged(kiOptionItem *pOptionItem);
	virtual void				OnToolOptionClicked(kiOptionItem *pOptionItem);

public:
	virtual void				OnOptionChanged(kiOptionItem *pOptionItem);
	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

	// 获取提示字符串
	virtual void				DoGetPromptText(std::string& str);
	
protected:
	kVector3					*_pDir;
	bool						_bOnlyZDir;//是否只需要标准Z向
	bool						_bUseWCS;//是否使用全局坐标系，false:使用基准面坐标系
	bool						_bUnited;
	int						_nStdAxis;//标准方向
};

#endif