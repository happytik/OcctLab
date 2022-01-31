//////////////////////////////////////////////////////////////////////////
//
#ifndef _KUI_INPUT_POINT_H_
#define _KUI_INPUT_POINT_H_

#include "kPoint.h"
#include "kVector.h"
#include "kiOptionSet.h"
#include "kiInputTool.h"


// 使用比较多的一个类
class kcPreviewText;

class QXLIB_API kiInputPoint : public kiInputTool
{
public:
	kiInputPoint(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	kiInputPoint(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputPoint(void);

	//初始化
	BOOL						Init(kPoint3 *pPoint,bool bSpacePoint = true);
	// 是否点的值实时更新
	void						SetRealtimeUpdate(bool bEnable);
	void						SetShowText(bool bEnable);
	// 是否允许拾取对象中的vertex
	void						EnableSubShape(bool bEnable);

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
	// 出入文本完成，通常命令可以结束了
	virtual int				DoTextInputFinished(const char *pInputText);
	
protected:
	// 更加点的值
	void						CalcPoint(kuiMouseInput& mouseInput);

protected:
	kPoint3					*_pPoint;//关联变量
	kPoint3					_point;//输入点
	kVector3					_vdir;//当空间输入点时,给出视向.
	bool						_bSnapPoint;//是否自动捕捉点

protected://选项
	bool						_bSpacePoint;//是否空间点
	bool						_bRealtimeUpdate;//是否实时更新关联变量的值。
	bool						_bShowText;
	bool						bEnableSubShape_;//是否能够拾取对象中的Vertex

	kcPreviewText				*_pPreviewText;
};

#endif
