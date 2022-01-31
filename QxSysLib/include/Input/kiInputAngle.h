//////////////////////////////////////////////////////////////////////////
// 输入角度，以角度或弧度表示。
#ifndef _KI_INPUT_ANGLE_H_
#define _KI_INPUT_ANGLE_H_

#include "kiInputTool.h"

class kcPreviewLine;
class kcPreviewArc;
class kcPreviewText;
class kvCoordSystem;

class QXLIB_API kiInputAngle : public kiInputTool
{
public:
	// 用于命令中的构造函数。
	kiInputAngle(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	// 用于InputTool中的构造函数。
	kiInputAngle(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputAngle(void);

	bool					Init(kvCoordSystem *pCS,double *pAngle,bool bRadian);

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

	void					Reset();
	void					Clear();

	double					CalcAngle(kuiMouseInput& mouseInput);

protected:
	// 获取提示字符串
	virtual void			DoGetPromptText(std::string& str);
	// 出入文本完成，通常命令可以结束了
	virtual int				DoTextInputFinished(const char *pInputText);

public:
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput);
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	kvCoordSystem			*_pLocalCS;//使用原点和z向，x向为开始方向
	double					*_pAngle;
	bool					_bRadian;//是否弧度

protected:
	kcPreviewLine			*_pPreviewLine;
	kcPreviewLine			*_pPreviewLine2;
	kcPreviewArc			*_pPreviewArc;
	kcPreviewText			*_pPreviewText;
};

#endif