//////////////////////////////////////////////////////////////////////////
//
#ifndef _KUI_INPUT_POINT_TOOL_H_
#define _KUI_INPUT_POINT_TOOL_H_

#include "kPoint.h"
#include "kVector.h"
#include "kiOptionSet.h"
#include "kiInputTool.h"


class kcPreviewText;

///输入一个点的工具，可以点击输入，可以手动输入
//
class QXLIB_API kiInputPointTool : public kiInputTool
{
public:
	kiInputPointTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	kiInputPointTool(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputPointTool(void);

	//初始化
	BOOL						Initialize(kPoint3 *pPoint,bool bSpacePoint = true);
	// 是否点的值实时更新
	void						SetRealtimeUpdate(bool bEnable);

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();


public://获取结果
	bool						IsSnapPoint() const { return _bSnapPoint; }
	kPoint3					GetPoint() const { return _point; }
	kVector3					GetViewDir() const { return _vdir; }

public:
	//鼠标消息
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput); 
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	// 获取提示字符串
	virtual void				DoGetPromptText(std::string& str);

public:
	//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
	virtual int				OnEditInputFinished(const char *pInputText);

protected:
	// 计算点的值
	void						CalcPoint(kuiMouseInput& mouseInput);

protected:
	kPoint3					*_pPoint;//关联变量
	kPoint3					_point;//输入点
	kVector3					_vdir;//当空间输入点时,给出视向.
	bool						_bSnapPoint;//是否自动捕捉点

protected://选项
	bool						_bSpacePoint;//是否空间点
	bool						_bRealtimeUpdate;//是否实时更新关联变量的值。
};

#endif
